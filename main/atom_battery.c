// atom_battery.c
// Battery voltage of the M5Stack Atomic Battery Base, read on the Atom VoiceS3R
#include "alles.h"

#ifdef ATOM_VOICES3R
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// The base divides the battery voltage by 2 (1M / 1M) onto G8, but the ADC input loads that divider.
// Measured on the hardware: a fully charged battery (~4.2V) reads ~1809mV at G8, so the effective ratio is ~2.32.
#define ATOM_BATTERY_RATIO_X1000 2322
#define ATOM_BATTERY_SAMPLES 16

static const char *TAG = "battery";
static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t cali_handle = NULL;
static adc_channel_t adc_channel;

esp_err_t atom_battery_init() {
    adc_unit_t unit;
    esp_err_t err = adc_oneshot_io_to_channel(ATOM_BATTERY_ADC, &unit, &adc_channel);
    if(err != ESP_OK) return err;

    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = unit };
    err = adc_oneshot_new_unit(&unit_cfg, &adc_handle);
    if(err != ESP_OK) return err;

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_oneshot_config_channel(adc_handle, adc_channel, &chan_cfg);
    if(err != ESP_OK) return err;

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = unit,
        .chan = adc_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle);
    if(err != ESP_OK) ESP_LOGW(TAG, "no ADC calibration, battery voltage will be approximate");
    return ESP_OK;
}

// Battery voltage in millivolts, or -1 if it can't be read
int atom_battery_read_mv() {
    if(adc_handle == NULL) return -1;
    int total = 0;
    for(int i=0;i<ATOM_BATTERY_SAMPLES;i++) {
        int raw = 0;
        if(adc_oneshot_read(adc_handle, adc_channel, &raw) != ESP_OK) return -1;
        total += raw;
    }
    int raw = total / ATOM_BATTERY_SAMPLES;
    int mv = 0;
    if(cali_handle == NULL || adc_cali_raw_to_voltage(cali_handle, raw, &mv) != ESP_OK) {
        mv = raw * 3300 / 4095;
    }
    return mv * ATOM_BATTERY_RATIO_X1000 / 1000;
}
#endif
