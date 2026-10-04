// alles_esp32.c
// Alles multicast synthesizer
// Brian Whitman
// brian@variogr.am

#include "alles.h"
#define configUSE_TASK_NOTIFICATIONS 1
//#define configTASK_NOTIFICATION_ARRAY_ENTRIES 2
#define MAX_WIFI_WAIT_S 120

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_sleep.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "lwip/netdb.h"
#include "wifi_manager.h"
#include "http_app.h"
#include "power.h"

#include "driver/gpio.h"


// Button handlers
void wifi_reconfigure();
extern esp_err_t buttons_init();
void esp_show_debug();

// wifi and multicast
extern wifi_config_t* wifi_manager_config_sta ;
extern void mcast_listen_task(void *pvParameters);

uint8_t board_level;
uint8_t status;
uint8_t debug_on = 0;

// For CPU usage
unsigned long last_task_counters[MAX_TASKS];

void delay_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

#ifdef ATOM_VOICES3R
uint8_t board_level = ALLES_ATOM_VOICES3R;
#else
uint8_t board_level = ALLES_BOARD_V2;
#endif
uint8_t status = RUNNING;

char githash[8];

// Button event
extern QueueHandle_t gpio_evt_queue;

// Task handles for the renderers, multicast listener and main
TaskHandle_t mcastTask = NULL;
TaskHandle_t parseTask = NULL;
TaskHandle_t upgradeTask = NULL;

#define ALLES_PARSE_TASK_COREID (0)
#define ALLES_RECEIVE_TASK_COREID (1)
#define ALLES_PARSE_TASK_PRIORITY (ESP_TASK_PRIO_MIN +2)
#define ALLES_RECEIVE_TASK_PRIORITY (ESP_TASK_PRIO_MIN + 3)
#define ALLES_PARSE_TASK_NAME       "alles_par_task"
#define ALLES_RECEIVE_TASK_NAME     "alles_rec_task"
#define ALLES_PARSE_TASK_STACK_SIZE (8 * 1024)
#define ALLES_RECEIVE_TASK_STACK_SIZE (4 * 1024)
#define ALLES_MAX_OSCS 120


// Battery status for V2 board. If no v2 board, will stay at 0
uint8_t battery_mask = 0;

extern uint32_t udp_message_counter;


// Make AMY's parse task run forever, as a FreeRTOS task (with notifications)
void esp_parse_task() {
    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        alles_parse_message(message_start_pointer, message_length);
        xTaskNotifyGive(mcastTask);
    }
}

#ifdef ATOM_VOICES3R
// AMY leaves a lot of headroom (a single full velocity sine peaks around -23dBFS), which is too quiet
// on the Atom's small speaker. Boost each bus here, before AMY's volume and output soft clipping.
#define ATOM_OUTPUT_GAIN_SHIFT 2  // x4, +12dB

// The Atom VoiceS3R has one speaker and the ES8311 only plays the left channel, so mix each bus down to mono
void mono_bus_hook(uint16_t bus, SAMPLE *buf, uint16_t len) {
    SAMPLE *left = buf;
    SAMPLE *right = buf + len;
    for(uint16_t i=0;i<len;i++) {
        SAMPLE mono = SHIFTL(SHIFTR(left[i], 1) + SHIFTR(right[i], 1), ATOM_OUTPUT_GAIN_SHIFT);
        left[i] = mono;
        right[i] = mono;
    }
}
#endif

// init AMY from the esp. AMY runs its own render tasks on both cores and writes to i2s.
amy_err_t esp_amy_init() {
    amy_config_t amy_config = amy_default_config();
    amy_config.features.reverb = 0;
    amy_config.features.echo = 0;
    amy_config.features.chorus = 1;
    amy_config.max_oscs = ALLES_MAX_OSCS;
    amy_config.audio = AMY_AUDIO_IS_I2S;
#ifdef ATOM_VOICES3R
    amy_config.i2s_mclk = ATOM_I2S_MCLK;
    amy_config.i2s_bclk = ATOM_I2S_BCLK;
    amy_config.i2s_lrc = ATOM_I2S_LRCLK;
    amy_config.i2s_dout = ATOM_I2S_DOUT;
    // The mic input (ATOM_I2S_DIN) isn't used
    amy_config.amy_external_bus_postprocess_hook = mono_bus_hook;
    // Keep the render buffers in internal RAM, everything else can go to PSRAM
    amy_config.ram_caps_events = MALLOC_CAP_SPIRAM;
    amy_config.ram_caps_synth = MALLOC_CAP_SPIRAM;
    amy_config.ram_caps_delay = MALLOC_CAP_SPIRAM;
    amy_config.ram_caps_sample = MALLOC_CAP_SPIRAM;
    amy_config.ram_caps_sysex = MALLOC_CAP_SPIRAM;
    amy_config.ram_caps_block = MALLOC_CAP_INTERNAL;
    amy_config.ram_caps_fbl = MALLOC_CAP_INTERNAL;
#else
    amy_config.i2s_bclk = CONFIG_I2S_BCLK;
    amy_config.i2s_lrc = CONFIG_I2S_LRCLK;
    amy_config.i2s_dout = CONFIG_I2S_DIN;
#endif
    amy_start(amy_config);
    amy_global.latency_ms = ALLES_LATENCY_MS;
    return AMY_OK;
}


// Show a CPU usage counter. This shows the delta in use since the last time you called it
void esp_show_debug() { 
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x, i;
    const char* const tasks[] = { ALLES_PARSE_TASK_NAME, ALLES_RECEIVE_TASK_NAME, AMY_RENDER_TASK_NAME, AMY_FILL_BUFFER_TASK_NAME, "main", "wifi", "IDLE0", "IDLE1", 0 }; 
    const uint8_t cores[] = {ALLES_PARSE_TASK_COREID, ALLES_RECEIVE_TASK_COREID, AMY_RENDER_TASK_COREID, AMY_FILL_BUFFER_TASK_COREID, 0, 0, 0, 1, 0};

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );
    uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, NULL );
    unsigned long counter_since_last[MAX_TASKS];
    //unsigned long ulTotalRunTime = 0;
    unsigned long ulTotalRunTime_per_core[2];
    ulTotalRunTime_per_core[0] = 0;
    ulTotalRunTime_per_core[1] = 0;

    //TaskStatus_t xTaskDetails;

    // We have to check for the names we want to track
    for(i=0;i<MAX_TASKS;i++) { // for each name
        counter_since_last[i] = 0;
        for(x=0; x<uxArraySize; x++) { // for each task
            if(strcmp(pxTaskStatusArray[x].pcTaskName, tasks[i])==0) {
                counter_since_last[i] = pxTaskStatusArray[x].ulRunTimeCounter - last_task_counters[i];
                last_task_counters[i] = pxTaskStatusArray[x].ulRunTimeCounter;
                //ulTotalRunTime = ulTotalRunTime + counter_since_last[i];
                ulTotalRunTime_per_core[cores[i]] += counter_since_last[i];
            }
        }
    }
    printf("------ CPU usage since last call to debug()\n");
    for(i=0;i<MAX_TASKS;i++) {
        printf("%d %-15s\t%-15ld\t\t%2.2f%%\n", cores[i], tasks[i], counter_since_last[i], (float)counter_since_last[i]/ulTotalRunTime_per_core[cores[i]] * 100.0);
    }   
    printf("------\nDelta queue size %d. Received %" PRIu32 " messages\n", amy_global.delta_qsize, udp_message_counter);
    udp_message_counter = 0;
    vPortFree(pxTaskStatusArray);

}

   

// callback to let us know when we have wifi set up ok.
void wifi_connected(void *pvParameter){
    ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    ESP_LOGI("main", "I have a connection and my IP is %s!", str_ip);
    status |= WIFI_MANAGER_OK;
}


// Called when the WIFI button is hit. Deletes the saved SSID/pass and restarts into the captive portal
void wifi_reconfigure() {
     printf("reconfigure wifi\n");

    if(wifi_manager_config_sta){
        memset(wifi_manager_config_sta, 0x00, sizeof(wifi_config_t));
    }

    if(wifi_manager_lock_json_buffer( portMAX_DELAY )){
        wifi_manager_generate_ip_info_json( UPDATE_USER_DISCONNECT );
        wifi_manager_unlock_json_buffer();
    }

    wifi_manager_save_sta_config();
    delay_ms(100);
    esp_restart();
}

void firmware_upgrade( void * pvParameters) {
    esp_http_client_config_t config = {
        .url = "https://github.com/shorepine/alles/raw/main/ota/alles.bin",
        .cert_pem = NULL,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        printf("Problem with upgrade %i %s\n", ret, esp_err_to_name(ret));
    }
    esp_restart();
}

// Battery level bits for the sync response, from the battery voltage
uint8_t battery_level_bits(float voltage) {
    if(voltage > 3.95) return BATTERY_VOLTAGE_4;
    if(voltage > 3.80) return BATTERY_VOLTAGE_3;
    if(voltage > 3.60) return BATTERY_VOLTAGE_2;
    if(voltage > 3.30) return BATTERY_VOLTAGE_1;
    return 0;
}

#ifdef ATOM_VOICES3R
// Outside this range there's no battery: below it there's no base and G8 is floating, above it the base's
// power switch is off and G8 isn't seeing the battery
#define ATOM_BATTERY_MIN_MV 2500
#define ATOM_BATTERY_MAX_MV 4500

// The battery base has no charge status line, so only the level is reported
void atom_battery_monitor() {
    int mv = atom_battery_read_mv();
    battery_mask = (mv >= ATOM_BATTERY_MIN_MV && mv <= ATOM_BATTERY_MAX_MV) ? battery_level_bits(mv/1000.0) : 0;
}
#else
void power_monitor() {
    power_status_t power_status;

    const amy_err_t ret = power_read_status(&power_status);
    if(ret != AMY_OK)
        return;

    // print a debugging power status every few seconds to the monitor 
    /*
    char buf[100];
    snprintf(buf, sizeof(buf),
        "powerStatus: power_source=\"%s\",charge_status=\"%s\",wall_v=%0.3f,battery_v=%0.3f\n",
        (power_status.power_source == POWER_SOURCE_WALL ? "wall" : "battery"),
        (power_status.charge_status == POWER_CHARGE_STATUS_CHARGING ? "charging" :
            (power_status.charge_status == POWER_CHARGE_STATUS_CHARGED ? "charged" : " discharging")),
        power_status.wall_voltage/1000.0,
        power_status.battery_voltage/1000.0
        );

    printf(buf);
    */

    battery_mask = 0;

    switch(power_status.charge_status) {
        case POWER_CHARGE_STATUS_CHARGED:
            battery_mask = battery_mask | BATTERY_STATE_CHARGED;
            break;
        case POWER_CHARGE_STATUS_CHARGING:
            battery_mask = battery_mask | BATTERY_STATE_CHARGING;
            break;
        case POWER_CHARGE_STATUS_DISCHARGING:
            battery_mask = battery_mask | BATTERY_STATE_DISCHARGING;
            break;        
    }

    battery_mask = battery_mask | battery_level_bits(power_status.battery_voltage/1000.0);
}

#endif

void turn_off() {
    debleep();
    delay_ms(500);
#ifdef ATOM_VOICES3R
    // No power switch and the user button (GPIO41) can't wake from deep sleep, so just reboot
    esp_restart();
#else
    // TODO: Where did these come from? JTAG?
    gpio_pullup_dis(14);
    gpio_pullup_dis(15);
    esp_sleep_enable_ext1_wakeup((1ULL<<BUTTON_WAKEUP),ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
#endif
}
void app_main() {
    const esp_app_desc_t * app_desc = esp_app_get_description(); // esp_ota_get_app_description();
    // version comes back as version "v0.1-alpha-259-g371d500-dirty"
    // the v0.1-alpha seems hardcoded, setting cmake PROJECT_VER replaces the more useful git describe line
    // so we'll have to parse the commit ID out
    // or maybe just get date time as YYYYMMDDHHMMSS? 

    if(strlen(app_desc->version) > 20) {
        if(app_desc->version[strlen(app_desc->version)-1] == 'y') {
            strncpy(githash, app_desc->version + strlen(app_desc->version)-13, 7);
        } else {
            strncpy(githash, app_desc->version + strlen(app_desc->version)-7, 7);            
        }
        githash[7] = 0;
    }
    printf("Welcome to %s -- date %s time %s version %s [%s]\n", app_desc->project_name, app_desc->date, app_desc->time, app_desc->version, githash);

    for(uint8_t i=0;i<MAX_TASKS;i++) last_task_counters[i] = 0;
    ESP_ERROR_CHECK(esp_event_loop_create_default());
#ifdef ATOM_VOICES3R
    printf("M5Stack Atom VoiceS3R\n");
    if(atom_battery_init() == ESP_OK) {
        printf("battery %d mV\n", atom_battery_read_mv());
        atom_battery_monitor();
        TimerHandle_t battery_timer = xTimerCreate("battery", pdMS_TO_TICKS(5000), pdTRUE, NULL, atom_battery_monitor);
        xTimerStart(battery_timer, 0);
    } else {
        printf("battery ADC init failed\n");
    }
#else
    // TODO -- this does not properly detect DEVBOARD anymore, not a big deal for now, doesn't impact anything
    // if power init fails, we don't have blinkinlabs board, set board level to 0
    if(power_init() != ESP_OK) {
        printf("No power IC, assuming DIY Alles\n");
        board_level = DEVBOARD; 
    }
    if(board_level == ALLES_BOARD_V2) {
        printf("Detected revB+ Alles\n");
        TimerHandle_t power_monitor_timer = xTimerCreate(
            "power_monitor",
            pdMS_TO_TICKS(5000),
            pdTRUE,
            NULL,
            power_monitor);
        xTimerStart(power_monitor_timer, 0);

    }
#endif

    // TODO: one of these interferes with the power monitor, not a big deal, we don't use both at once
    // Setup GPIO outputs for watching CPU usage on an oscilloscope 
    /*
    const gpio_config_t out_conf = {
         .mode = GPIO_MODE_OUTPUT,            
         .pin_bit_mask = (1ULL<<CPU_MONITOR_0) | (1ULL<<CPU_MONITOR_1) | (1ULL<<CPU_MONITOR_2),
    };
    gpio_config(&out_conf); 

    // Set them all to low
    gpio_set_level(CPU_MONITOR_0, 0); // use 0 as ground for the scope 
    gpio_set_level(CPU_MONITOR_1, 0); // use 1 for the rendering loop 
    gpio_set_level(CPU_MONITOR_2, 0); // use 2 for whatever you want 
    */

    check_init(&sync_init, "sync"); 
    esp_amy_init();
#ifdef ATOM_VOICES3R
    if(codec_init() != ESP_OK) printf("codec init failed\n");
#endif
    if(buttons_init() != ESP_OK) printf("buttons init failed\n"); // only one button for the protoboard and the Atom, 4 for the blinkinlabs

    wifi_manager_start();
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &wifi_connected);


    // A funny story: in the early development of Alles, I had four prototype speakers with me as I flew a little
    // prop plane from YBL to YVR. The bag with the speakers went into the cargo hold of the prop plane and when we
    // landed everyone could hear this weird chiming noise -- it turns out all four speakers got turned on during the 
    // flight and at the time there was a bug where if it couldn't connect to the stored wifi, the power button
    // wouldn't be active either and all four speakers were stuck making the wifi chime. Quite a fun noise to hear in an
    // airplane! I didn't have any programming cables or a screwdriver with me so I had to put them all in a hotel 
    // safe covered with a pillow to get their batteries to die. 

    //So now they shut off after MAX_WIFI_WAIT_S if they can't connect.
    // The Atom VoiceS3R runs off USB power and can't turn itself off, so it just stops chiming and keeps waiting.
    uint32_t start_time = amy_sysclock();
    delay_ms(250);
    //heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);

    while((!(status & WIFI_MANAGER_OK) && (status & RUNNING) )) {
        uint8_t timed_out = (amy_sysclock() - start_time > (MAX_WIFI_WAIT_S*1000));
#ifdef ATOM_VOICES3R
        if(!timed_out) wifi_tone();
#else
        if(timed_out) turn_off();
        wifi_tone();
#endif
        for(uint8_t i=0;i<250;i++) { 
            if(!(status & RUNNING)) turn_off();
            delay_ms(10);
        }
    }

    // We check for RUNNING as someone could have pressed power already
    if(!(status & RUNNING)) turn_off();

    // was + held down right now? if so check for updates
    if(status & UPDATE) {
        xTaskCreatePinnedToCore(&firmware_upgrade, "upgrade", 8192, NULL, 0, &upgradeTask, 0);
        while(1) {
            upgrade_tone();
            delay_ms(2000);
        }
    }


    delay_ms(500);
    amy_reset_oscs();

    // Setup the socket
    create_multicast_ipv4_socket();

    // Create the task that listens fro new incoming UDP messages (core 2)
    xTaskCreatePinnedToCore(&mcast_listen_task, ALLES_RECEIVE_TASK_NAME, ALLES_RECEIVE_TASK_STACK_SIZE, NULL, ALLES_RECEIVE_TASK_PRIORITY, &mcastTask, ALLES_RECEIVE_TASK_COREID);
    delay_ms(100);

    // Create the task that waits for UDP messages, parses them and puts them on the sequencer queue (core 1)
    xTaskCreatePinnedToCore(&esp_parse_task, ALLES_PARSE_TASK_NAME, ALLES_PARSE_TASK_STACK_SIZE, NULL, ALLES_PARSE_TASK_PRIORITY, &parseTask, ALLES_PARSE_TASK_COREID);

    // Schedule a "turning on" sound
    bleep(amy_sysclock());

    // Print free RAm
    //heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);

    // Spin this core until the power off button is pressed, parsing events and making sounds
    while(status & RUNNING) {
        delay_ms(10);
    }

    // If we got here, the power off button was pressed 
    turn_off();
}

