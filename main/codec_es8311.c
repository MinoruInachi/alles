// codec_es8311.c
// ES8311 codec + NS4150B amp setup for the M5Stack Atom VoiceS3R
#include "alles.h"

#ifdef ATOM_VOICES3R
#include "driver/i2c_master.h"
#include "esp_codec_dev_defaults.h"
#include "es8311_codec.h"

#define ES8311_REG_SDPIN 0x09
#define ES8311_REG_SDPOUT 0x0A
#define ES8311_REG_DAC_VOLUME 0x32
#define ES8311_SDP_KEEP_MASK 0xE0  // channel select and mute bits
#define ES8311_SDP_WL_16BIT 0x0C
#define ES8311_SDP_LEFT_JUSTIFIED 0x01
#define ES8311_DAC_VOLUME_0DB 0xBF

static const char *TAG = "codec";
static const audio_codec_if_t *codec_if = NULL;

// AMY drives I2S with 32-bit MSB (left justified) slots holding 16-bit samples, but esp_codec_dev always puts
// the ES8311 in Philips I2S mode, and its 32-bit word length setting leaves a reserved value in the register.
static int codec_set_serial_format() {
    const int regs[] = { ES8311_REG_SDPIN, ES8311_REG_SDPOUT };
    int ret = 0;
    for(int i=0;i<2;i++) {
        int v = 0;
        ret |= codec_if->get_reg(codec_if, regs[i], &v);
        ret |= codec_if->set_reg(codec_if, regs[i], (v & ES8311_SDP_KEEP_MASK) | ES8311_SDP_WL_16BIT | ES8311_SDP_LEFT_JUSTIFIED);
    }
    return ret;
}

// Call after amy_start(), so MCLK is already running.
esp_err_t codec_init() {
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = ATOM_I2C_SDA,
        .scl_io_num = ATOM_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle = NULL;
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus_handle);
    if(err != ESP_OK) return err;

    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = I2C_NUM_0,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .bus_handle = bus_handle,
    };
    const audio_codec_ctrl_if_t *ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();
    if(ctrl_if == NULL || gpio_if == NULL) return ESP_FAIL;

    es8311_codec_cfg_t es_cfg = {
        .ctrl_if = ctrl_if,
        .gpio_if = gpio_if,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .pa_pin = ATOM_PA_EN,
        .pa_reverted = false,
        .master_mode = false,
        .use_mclk = true,
        .mclk_div = 256,
    };
    codec_if = es8311_codec_new(&es_cfg);
    if(codec_if == NULL) {
        ESP_LOGE(TAG, "ES8311 not found");
        return ESP_FAIL;
    }

    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = 16,
        .channel = 2,
        .sample_rate = AMY_SAMPLE_RATE,
    };
    int ret = codec_if->set_fs(codec_if, &fs);
    ret |= codec_set_serial_format();
    ret |= codec_if->enable(codec_if, true);
    // set_vol() needs the board's hw_gain voltages to land on 0dB, so set the register directly
    ret |= codec_if->set_reg(codec_if, ES8311_REG_DAC_VOLUME, ES8311_DAC_VOLUME_0DB);
    ret |= codec_if->mute(codec_if, false);
    if(ret != 0) {
        ESP_LOGE(TAG, "ES8311 setup failed (%d)", ret);
        return ESP_FAIL;
    }
    return ESP_OK;
}

#endif
