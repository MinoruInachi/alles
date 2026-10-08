// ns4168.c
// NS4168 I2S amp setup for the M5Stack Atom Voice
#include "alles.h"

#ifdef ATOM_VOICE
#include "driver/i2s_std.h"

// AMY's I2S output channel (amy/src/i2s.c)
extern i2s_chan_handle_t tx_handle;

// AMY sets up its I2S output with MSB (left justified) slots, but the NS4168 only takes Philips I2S, where each
// sample starts one BCLK after LRCLK changes. Call after amy_start() to switch AMY's channel over. While the channel
// is disabled, AMY's audio task waits in i2s_channel_write() until it's enabled again.
esp_err_t ns4168_init() {
    i2s_std_slot_config_t slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
    esp_err_t err = i2s_channel_disable(tx_handle);
    if(err != ESP_OK) return err;
    err = i2s_channel_reconfig_std_slot(tx_handle, &slot_cfg);
    esp_err_t enable_err = i2s_channel_enable(tx_handle);
    return (err != ESP_OK) ? err : enable_err;
}
#endif
