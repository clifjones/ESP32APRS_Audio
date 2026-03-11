#include "webservice.h"  // extern Configuration config
#include <AFSK.h>        // afskSet*, AFSK_init, setPtt, AFSK_Poll, LED_TX_PIN, LED_RX_PIN

extern bool AFSKInitAct;

void taskAPRSPoll(void *pvParameters)
{
    afskSetModem(config.modem_type, config.audio_lpf, config.tx_timeslot, config.preamble * 100, config.fx25_mode);
    afskSetSQL(config.rf_sql_gpio, config.rf_sql_active);
    afskSetPTT(config.rf_ptt_gpio, config.rf_ptt_active);
    afskSetPWR(config.rf_pwr_gpio, config.rf_pwr_active);

    // afskSetDCOffset(config.adc_dc_offset);
    afskSetADCAtten(config.adc_atten);

#ifdef STRIP_PIN
    AFSK_init(config.adc_gpio, config.dac_gpio, config.rf_ptt_gpio, config.rf_sql_gpio, config.rf_pwr_gpio, -1, -1, STRIP_PIN, config.rf_ptt_active, config.rf_sql_active, config.rf_pwr_active);
#else
    AFSK_init(config.adc_gpio, config.dac_gpio, config.rf_ptt_gpio, config.rf_sql_gpio, config.rf_pwr_gpio, LED_TX_PIN, LED_RX_PIN, -1, config.rf_ptt_active, config.rf_sql_active, config.rf_pwr_active);
#endif
    setPtt(false);
    log_d("APRS Polling Task Start on Core %d.", xPortGetCoreID());
    for (;;)
    {
        if (config.modem_type == 3)
            vTaskDelay(1 / portTICK_PERIOD_MS);
        else
            vTaskDelay(3 / portTICK_PERIOD_MS);

        if (AFSKInitAct == true)
        {
            AFSK_Poll(false, LOW);
        }
    }
}
