/*
 * handleATCommand.cpp
 * Table-driven AT command dispatcher for ESP32 APRS
 */

#include <Arduino.h>
#include "handleATCommand.h"
#include "config.h"
#include "main.h"
#include "core_version.h"
#include <WiFi.h>
#include <modem.h>

extern Configuration config;

// ---------------------------------------------------------------------------
// ATParam type enum and struct
// ---------------------------------------------------------------------------

enum ATParamType {
    ATP_BOOL,    // bool*
    ATP_INT8,    // int8_t*
    ATP_UINT8,   // uint8_t*
    ATP_INT,     // int*
    ATP_UINT16,  // uint16_t*
    ATP_UINT32,  // uint32_t*
    ATP_FLOAT6,  // float* (6 decimal places)
    ATP_STR,     // char[] with strLen
};

struct ATParam {
    const char* name;    // command name without "AT+" prefix, e.g. "WIFI_MODE"
    const char* desc;    // brief description for help
    ATParamType type;
    void*       ptr;
    uint16_t    strLen;  // for ATP_STR only
};

// ---------------------------------------------------------------------------
// Generic dispatch helper
// ---------------------------------------------------------------------------

static String dispatchParam(const ATParam& p, const String& cmd)
{
    String prefix = String("AT+") + p.name;
    String queryCmd  = prefix + "?";
    String assignPfx = prefix + "=";

    if (cmd == queryCmd) {
        switch (p.type) {
            case ATP_BOOL:   return String(*(bool*)p.ptr ? "1" : "0");
            case ATP_INT8:   return String((int)(*(int8_t*)p.ptr));
            case ATP_UINT8:  return String(*(uint8_t*)p.ptr);
            case ATP_INT:    return String(*(int*)p.ptr);
            case ATP_UINT16: return String(*(uint16_t*)p.ptr);
            case ATP_UINT32: return String(*(uint32_t*)p.ptr);
            case ATP_FLOAT6: return String(*(float*)p.ptr, 6);
            case ATP_STR:    return String((char*)p.ptr);
        }
        return "";
    }

    if (cmd.startsWith(assignPfx)) {
        String val = cmd.substring(assignPfx.length());
        val.replace("\"", "");
        switch (p.type) {
            case ATP_BOOL: {
                bool v = (val == "1" || val.equalsIgnoreCase("true"));
                *(bool*)p.ptr = v;
                break;
            }
            case ATP_INT8:
                *(int8_t*)p.ptr = (int8_t)val.toInt();
                break;
            case ATP_UINT8:
                *(uint8_t*)p.ptr = (uint8_t)val.toInt();
                break;
            case ATP_INT:
                *(int*)p.ptr = val.toInt();
                break;
            case ATP_UINT16:
                *(uint16_t*)p.ptr = (uint16_t)val.toInt();
                break;
            case ATP_UINT32:
                *(uint32_t*)p.ptr = (uint32_t)val.toInt();
                break;
            case ATP_FLOAT6:
                *(float*)p.ptr = val.toFloat();
                break;
            case ATP_STR:
                strncpy((char*)p.ptr, val.c_str(), p.strLen - 1);
                ((char*)p.ptr)[p.strLen - 1] = '\0';
                break;
        }
        return "OK";
    }

    return "";
}

// ---------------------------------------------------------------------------
// Indexed array dispatch helpers (bool arrays, uint8 arrays, float arrays)
// ---------------------------------------------------------------------------

static String dispatchBoolArray(const char* baseName, const char* desc,
                                 bool* arr, uint8_t count, const String& cmd)
{
    for (uint8_t n = 0; n < count; n++) {
        String prefix = String("AT+") + baseName + String(n);
        if (cmd == prefix + "?")
            return String(arr[n] ? "1" : "0");
        if (cmd == prefix + "=1") { arr[n] = true;  return "OK"; }
        if (cmd == prefix + "=0") { arr[n] = false; return "OK"; }
    }
    return "";
}

static String dispatchUint8Array(const char* baseName, const char* desc,
                                  uint8_t* arr, uint8_t count, const String& cmd)
{
    for (uint8_t n = 0; n < count; n++) {
        String prefix = String("AT+") + baseName + String(n);
        if (cmd == prefix + "?")
            return String(arr[n]);
        if (cmd.startsWith(prefix + "=")) {
            arr[n] = (uint8_t)cmd.substring(prefix.length() + 1).toInt();
            return "OK";
        }
    }
    return "";
}

static String dispatchFloatArray(const char* baseName, const char* desc,
                                  float* arr, uint8_t count, const String& cmd)
{
    for (uint8_t n = 0; n < count; n++) {
        String prefix = String("AT+") + baseName + String(n);
        if (cmd == prefix + "?")
            return String(arr[n], 6);
        if (cmd.startsWith(prefix + "=")) {
            arr[n] = cmd.substring(prefix.length() + 1).toFloat();
            return "OK";
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: System
// ---------------------------------------------------------------------------

static String handleSystemParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "TIMEZONE",      "Timezone offset (float)",       ATP_FLOAT6, &config.timeZone,      0 },
        { "SYNCTIME",      "Enable NTP sync (0/1)",         ATP_BOOL,   &config.synctime,       0 },
        { "TITLE",         "Show title on display (0/1)",   ATP_BOOL,   &config.title,          0 },
        { "LOG",           "Log level bitmask",             ATP_UINT16, &config.log,            0 },
        { "HOST_NAME",     "mDNS hostname",                 ATP_STR,    config.host_name,      sizeof(config.host_name) },
        { "RESET_TIMEOUT", "Watchdog reset timeout (min)",  ATP_UINT16, &config.reset_timeout,  0 },
        { "NTP_HOST",      "NTP server hostname",           ATP_STR,    config.ntp_host,       sizeof(config.ntp_host) },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: WiFi
// ---------------------------------------------------------------------------

static String handleWifiParams(const String& cmd)
{
    // Scalar WiFi params
    static ATParam scalar[] = {
        { "WIFI_MODE",    "WiFi mode (0=OFF,1=STA,2=AP,3=AP+STA)", ATP_UINT8, &config.wifi_mode,    0 },
        { "WIFI_POWER",   "WiFi TX power (int8)",                   ATP_INT8,  &config.wifi_power,   0 },
        { "WIFI_AP_CH",   "AP channel",                             ATP_UINT8, &config.wifi_ap_ch,   0 },
        { "WIFI_AP_SSID", "AP SSID",                                ATP_STR,   config.wifi_ap_ssid, sizeof(config.wifi_ap_ssid) },
        { "WIFI_AP_PASS", "AP password",                            ATP_STR,   config.wifi_ap_pass, sizeof(config.wifi_ap_pass) },
    };
    for (auto& p : scalar) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // WiFi STA 0..4 entries
    for (int i = 0; i < 5; i++) {
        String enName   = String("WIFI") + String(i) + "EN";
        String ssidName = String("WIFI") + String(i) + "SSID";
        String passName = String("WIFI") + String(i) + "PASS";

        // Enable
        {
            String prefix = String("AT+") + enName;
            if (cmd == prefix + "?")  return String(config.wifi_sta[i].enable ? "1" : "0");
            if (cmd == prefix + "=1") { config.wifi_sta[i].enable = true;  return "OK"; }
            if (cmd == prefix + "=0") { config.wifi_sta[i].enable = false; return "OK"; }
        }
        // SSID
        {
            String prefix = String("AT+") + ssidName;
            if (cmd == prefix + "?") return String(config.wifi_sta[i].wifi_ssid);
            if (cmd.startsWith(prefix + "=")) {
                String val = cmd.substring(prefix.length() + 1);
                val.replace("\"", "");
                strncpy(config.wifi_sta[i].wifi_ssid, val.c_str(), sizeof(config.wifi_sta[i].wifi_ssid) - 1);
                config.wifi_sta[i].wifi_ssid[sizeof(config.wifi_sta[i].wifi_ssid) - 1] = '\0';
                return "OK";
            }
        }
        // Pass
        {
            String prefix = String("AT+") + passName;
            if (cmd == prefix + "?") return String(config.wifi_sta[i].wifi_pass);
            if (cmd.startsWith(prefix + "=")) {
                String val = cmd.substring(prefix.length() + 1);
                val.replace("\"", "");
                strncpy(config.wifi_sta[i].wifi_pass, val.c_str(), sizeof(config.wifi_sta[i].wifi_pass) - 1);
                config.wifi_sta[i].wifi_pass[sizeof(config.wifi_sta[i].wifi_pass) - 1] = '\0';
                return "OK";
            }
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Bluetooth
// ---------------------------------------------------------------------------

static String handleBtParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "BT_SLAVE",    "BT slave mode (0/1)",  ATP_BOOL,   &config.bt_slave,   0 },
        { "BT_MASTER",   "BT master mode (0/1)", ATP_BOOL,   &config.bt_master,  0 },
        { "BT_MODE",     "BT mode",              ATP_UINT8,  &config.bt_mode,    0 },
        { "BT_UUID",     "BT service UUID",      ATP_STR,    config.bt_uuid,    sizeof(config.bt_uuid) },
        { "BT_UUID_RX",  "BT RX char UUID",      ATP_STR,    config.bt_uuid_rx, sizeof(config.bt_uuid_rx) },
        { "BT_UUID_TX",  "BT TX char UUID",      ATP_STR,    config.bt_uuid_tx, sizeof(config.bt_uuid_tx) },
        { "BT_NAME",     "BT device name",       ATP_STR,    config.bt_name,    sizeof(config.bt_name) },
        { "BT_PIN",      "BT pairing PIN",       ATP_UINT32, &config.bt_pin,    0 },
        { "BT_POWER",    "BT TX power level",    ATP_UINT8,  &config.bt_power,  0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: RF
// ---------------------------------------------------------------------------

static String handleRfParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "RF_EN",       "RF module enable (0/1)",   ATP_BOOL,   &config.rf_en,         0 },
        { "RF_TYPE",     "RF module type",           ATP_UINT8,  &config.rf_type,        0 },
        { "FREQ_RX",     "RX frequency (MHz)",       ATP_FLOAT6, &config.freq_rx,        0 },
        { "FREQ_TX",     "TX frequency (MHz)",       ATP_FLOAT6, &config.freq_tx,        0 },
        { "OFFSET_RX",   "RX offset (Hz)",           ATP_INT,    &config.offset_rx,      0 },
        { "OFFSET_TX",   "TX offset (Hz)",           ATP_INT,    &config.offset_tx,      0 },
        { "TONE_RX",     "RX CTCSS tone",            ATP_INT,    &config.tone_rx,        0 },
        { "TONE_TX",     "TX CTCSS tone",            ATP_INT,    &config.tone_tx,        0 },
        { "BAND",        "RF band",                  ATP_UINT8,  &config.band,           0 },
        { "SQL_LEVEL",   "Squelch level",            ATP_UINT8,  &config.sql_level,      0 },
        { "RF_POWER",    "RF power high (0/1)",      ATP_BOOL,   &config.rf_power,       0 },
        { "VOLUME",      "Audio volume",             ATP_UINT8,  &config.volume,         0 },
        { "MIC",         "Mic gain",                 ATP_UINT8,  &config.mic,            0 },
        { "RF_TX_GPIO",  "RF TX GPIO pin",           ATP_INT8,   &config.rf_tx_gpio,     0 },
        { "RF_RX_GPIO",  "RF RX GPIO pin",           ATP_INT8,   &config.rf_rx_gpio,     0 },
        { "RF_SQL_GPIO", "RF squelch GPIO pin",      ATP_INT8,   &config.rf_sql_gpio,    0 },
        { "RF_PD_GPIO",  "RF power-down GPIO pin",   ATP_INT8,   &config.rf_pd_gpio,     0 },
        { "RF_PWR_GPIO", "RF power GPIO pin",        ATP_INT8,   &config.rf_pwr_gpio,    0 },
        { "RF_PTT_GPIO", "RF PTT GPIO pin",          ATP_INT8,   &config.rf_ptt_gpio,    0 },
        { "RF_SQL_ACTIVE","RF squelch active level", ATP_BOOL,   &config.rf_sql_active,  0 },
        { "RF_PD_ACTIVE", "RF power-down active lvl",ATP_BOOL,   &config.rf_pd_active,   0 },
        { "RF_PWR_ACTIVE","RF power active level",   ATP_BOOL,   &config.rf_pwr_active,  0 },
        { "RF_PTT_ACTIVE","RF PTT active level",     ATP_BOOL,   &config.rf_ptt_active,  0 },
        { "ADC_GPIO",    "ADC GPIO pin",             ATP_INT8,   &config.adc_gpio,       0 },
        { "DAC_GPIO",    "DAC GPIO pin",             ATP_INT8,   &config.dac_gpio,       0 },
        { "ADC_SEL_GPIO","ADC select GPIO pin",      ATP_INT8,   &config.adc_sel_gpio,   0 },
        { "DAC_SEL_GPIO","DAC select GPIO pin",      ATP_INT8,   &config.dac_sel_gpio,   0 },
        { "ADC_ATTEN",   "ADC attenuation",          ATP_UINT8,  &config.adc_atten,      0 },
        { "ADC_DC_OFFSET","ADC DC offset",           ATP_UINT16, &config.adc_dc_offset,  0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: IGATE
// ---------------------------------------------------------------------------

static String handleIgateParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "IGATE_EN",          "iGate enable (0/1)",           ATP_BOOL,   &config.igate_en,         0 },
        { "RF2INET",           "RF to internet (0/1)",         ATP_BOOL,   &config.rf2inet,           0 },
        { "INET2RF",           "Internet to RF (0/1)",         ATP_BOOL,   &config.inet2rf,           0 },
        { "IGATE_LOC2RF",      "iGate loc beacon to RF (0/1)", ATP_BOOL,   &config.igate_loc2rf,      0 },
        { "IGATE_LOC2INET",    "iGate loc beacon to IS (0/1)", ATP_BOOL,   &config.igate_loc2inet,    0 },
        { "RF2INETFILTER",     "RF-to-IS packet filter",       ATP_UINT16, &config.rf2inetFilter,     0 },
        { "INET2RFFILTER",     "IS-to-RF packet filter",       ATP_UINT16, &config.inet2rfFilter,     0 },
        { "APRS_SSID",         "APRS-IS SSID",                 ATP_UINT8,  &config.aprs_ssid,         0 },
        { "APRS_PORT",         "APRS-IS port",                 ATP_UINT16, &config.aprs_port,         0 },
        { "APRS_MYCALL",       "APRS callsign",                ATP_STR,    config.aprs_mycall,       sizeof(config.aprs_mycall) },
        { "APRS_HOST",         "APRS-IS server host",          ATP_STR,    config.aprs_host,         sizeof(config.aprs_host) },
        { "APRS_PASSCODE",     "APRS-IS passcode",             ATP_STR,    config.aprs_passcode,     sizeof(config.aprs_passcode) },
        { "APRS_MONICALL",     "APRS monitor callsign",        ATP_STR,    config.aprs_moniCall,     sizeof(config.aprs_moniCall) },
        { "APRS_FILTER",       "APRS-IS server filter",        ATP_STR,    config.aprs_filter,       sizeof(config.aprs_filter) },
        { "IGATE_BCN",         "iGate beacon enable (0/1)",    ATP_BOOL,   &config.igate_bcn,         0 },
        { "IGATE_GPS",         "iGate use GPS (0/1)",          ATP_BOOL,   &config.igate_gps,         0 },
        { "IGATE_TIMESTAMP",   "iGate timestamp (0/1)",        ATP_BOOL,   &config.igate_timestamp,   0 },
        { "IGATE_LAT",         "iGate latitude",               ATP_FLOAT6, &config.igate_lat,         0 },
        { "IGATE_LON",         "iGate longitude",              ATP_FLOAT6, &config.igate_lon,         0 },
        { "IGATE_ALT",         "iGate altitude (m)",           ATP_FLOAT6, &config.igate_alt,         0 },
        { "IGATE_INTERVAL",    "iGate beacon interval (sec)",  ATP_UINT16, &config.igate_interval,    0 },
        { "IGATE_SYMBOL",      "iGate symbol (2 chars)",       ATP_STR,    config.igate_symbol,      sizeof(config.igate_symbol) },
        { "IGATE_OBJECT",      "iGate object name",            ATP_STR,    config.igate_object,      sizeof(config.igate_object) },
        { "IGATE_PHG",         "iGate PHG string",             ATP_STR,    config.igate_phg,         sizeof(config.igate_phg) },
        { "IGATE_PATH",        "iGate path index",             ATP_UINT8,  &config.igate_path,        0 },
        { "IGATE_COMMENT",     "iGate beacon comment",         ATP_STR,    config.igate_comment,     sizeof(config.igate_comment) },
        { "IGATE_STS_INTERVAL","iGate status interval (sec)",  ATP_UINT16, &config.igate_sts_interval,0 },
        { "IGATE_STATUS",      "iGate status text",            ATP_STR,    config.igate_status,      sizeof(config.igate_status) },
        { "IGATE_TLM_INTERVAL","iGate telemetry interval",     ATP_UINT8,  &config.igate_tlm_interval,0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // Indexed igate TLM arrays (N=0..4)
    {
        String r;
        r = dispatchBoolArray ("IGATE_TLM_AVG",       "iGate TLM averaging ch",  config.igate_tlm_avg,       5, cmd); if (r.length()) return r;
        r = dispatchUint8Array("IGATE_TLM_SENSOR",    "iGate TLM sensor ch",     config.igate_tlm_sensor,    5, cmd); if (r.length()) return r;
        r = dispatchUint8Array("IGATE_TLM_PRECISION", "iGate TLM precision ch",  config.igate_tlm_precision, 5, cmd); if (r.length()) return r;
        r = dispatchFloatArray("IGATE_TLM_OFFSET",    "iGate TLM offset ch",     config.igate_tlm_offset,    5, cmd); if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: DIGI
// ---------------------------------------------------------------------------

static String handleDigiParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "DIGI_EN",          "Digipeater enable (0/1)",      ATP_BOOL,   &config.digi_en,          0 },
        { "DIGI_AUTO",        "Digi auto mode (0/1)",         ATP_BOOL,   &config.digi_auto,         0 },
        { "DIGI_LOC2RF",      "Digi loc to RF (0/1)",         ATP_BOOL,   &config.digi_loc2rf,       0 },
        { "DIGI_LOC2INET",    "Digi loc to IS (0/1)",         ATP_BOOL,   &config.digi_loc2inet,     0 },
        { "DIGI_TIMESTAMP",   "Digi timestamp (0/1)",         ATP_BOOL,   &config.digi_timestamp,    0 },
        { "DIGI_SSID",        "Digi SSID",                    ATP_UINT8,  &config.digi_ssid,         0 },
        { "DIGI_MYCALL",      "Digi callsign",                ATP_STR,    config.digi_mycall,       sizeof(config.digi_mycall) },
        { "DIGI_PATH",        "Digi path index",              ATP_UINT8,  &config.digi_path,         0 },
        { "DIGI_DELAY",       "Digi TX delay (ms)",           ATP_UINT16, &config.digi_delay,        0 },
        { "DIGIFILTER",       "Digi packet filter",           ATP_UINT16, &config.digiFilter,        0 },
        { "DIGI_BCN",         "Digi beacon enable (0/1)",     ATP_BOOL,   &config.digi_bcn,          0 },
        { "DIGI_GPS",         "Digi use GPS (0/1)",           ATP_BOOL,   &config.digi_gps,          0 },
        { "DIGI_LAT",         "Digi latitude",                ATP_FLOAT6, &config.digi_lat,          0 },
        { "DIGI_LON",         "Digi longitude",               ATP_FLOAT6, &config.digi_lon,          0 },
        { "DIGI_ALT",         "Digi altitude (m)",            ATP_FLOAT6, &config.digi_alt,          0 },
        { "DIGI_INTERVAL",    "Digi beacon interval (sec)",   ATP_UINT16, &config.digi_interval,     0 },
        { "DIGI_SYMBOL",      "Digi symbol (2 chars)",        ATP_STR,    config.digi_symbol,       sizeof(config.digi_symbol) },
        { "DIGI_PHG",         "Digi PHG string",              ATP_STR,    config.digi_phg,          sizeof(config.digi_phg) },
        { "DIGI_COMMENT",     "Digi beacon comment",          ATP_STR,    config.digi_comment,      sizeof(config.digi_comment) },
        { "DIGI_STS_INTERVAL","Digi status interval (sec)",   ATP_UINT16, &config.digi_sts_interval, 0 },
        { "DIGI_STATUS",      "Digi status text",             ATP_STR,    config.digi_status,       sizeof(config.digi_status) },
        { "DIGI_TLM_INTERVAL","Digi telemetry interval",      ATP_UINT8,  &config.digi_tlm_interval, 0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // Indexed digi TLM arrays (N=0..4)
    {
        String r;
        r = dispatchBoolArray ("DIGI_TLM_AVG",       "Digi TLM averaging ch",  config.digi_tlm_avg,       5, cmd); if (r.length()) return r;
        r = dispatchUint8Array("DIGI_TLM_SENSOR",    "Digi TLM sensor ch",     config.digi_tlm_sensor,    5, cmd); if (r.length()) return r;
        r = dispatchUint8Array("DIGI_TLM_PRECISION", "Digi TLM precision ch",  config.digi_tlm_precision, 5, cmd); if (r.length()) return r;
        r = dispatchFloatArray("DIGI_TLM_OFFSET",    "Digi TLM offset ch",     config.digi_tlm_offset,    5, cmd); if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Tracker
// ---------------------------------------------------------------------------

static String handleTrackerParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "TRK_EN",           "Tracker enable (0/1)",         ATP_BOOL,   &config.trk_en,            0 },
        { "TRK_LOC2RF",       "Tracker loc to RF (0/1)",      ATP_BOOL,   &config.trk_loc2rf,        0 },
        { "TRK_LOC2INET",     "Tracker loc to IS (0/1)",      ATP_BOOL,   &config.trk_loc2inet,      0 },
        { "TRK_TIMESTAMP",    "Tracker timestamp (0/1)",      ATP_BOOL,   &config.trk_timestamp,     0 },
        { "TRK_SSID",         "Tracker SSID",                 ATP_UINT8,  &config.trk_ssid,          0 },
        { "TRK_MYCALL",       "Tracker callsign",             ATP_STR,    config.trk_mycall,        sizeof(config.trk_mycall) },
        { "TRK_PATH",         "Tracker path index",           ATP_UINT8,  &config.trk_path,          0 },
        { "TRK_GPS",          "Tracker use GPS (0/1)",        ATP_BOOL,   &config.trk_gps,           0 },
        { "TRK_LAT",          "Tracker latitude",             ATP_FLOAT6, &config.trk_lat,           0 },
        { "TRK_LON",          "Tracker longitude",            ATP_FLOAT6, &config.trk_lon,           0 },
        { "TRK_ALT",          "Tracker altitude (m)",         ATP_FLOAT6, &config.trk_alt,           0 },
        { "TRK_INTERVAL",     "Tracker beacon interval (sec)",ATP_UINT16, &config.trk_interval,      0 },
        { "TRK_SMARTBEACON",  "SmartBeacon enable (0/1)",     ATP_BOOL,   &config.trk_smartbeacon,   0 },
        { "TRK_COMPRESS",     "Compressed position (0/1)",    ATP_BOOL,   &config.trk_compress,      0 },
        { "TRK_ALTITUDE",     "Include altitude (0/1)",       ATP_BOOL,   &config.trk_altitude,      0 },
        { "TRK_LOG",          "Tracker logging (0/1)",        ATP_BOOL,   &config.trk_log,           0 },
        { "TRK_RSSI",         "Include RSSI (0/1)",           ATP_BOOL,   &config.trk_rssi,          0 },
        { "TRK_SAT",          "Include sat count (0/1)",      ATP_BOOL,   &config.trk_sat,           0 },
        { "TRK_DX",           "Include DX info (0/1)",        ATP_BOOL,   &config.trk_dx,            0 },
        { "TRK_HSPEED",       "SmartBeacon high speed (kph)", ATP_UINT16, &config.trk_hspeed,        0 },
        { "TRK_LSPEED",       "SmartBeacon low speed (kph)",  ATP_UINT8,  &config.trk_lspeed,        0 },
        { "TRK_MAXINTERVAL",  "SmartBeacon max interval",     ATP_UINT8,  &config.trk_maxinterval,   0 },
        { "TRK_MININTERVAL",  "SmartBeacon min interval",     ATP_UINT8,  &config.trk_mininterval,   0 },
        { "TRK_MINANGLE",     "SmartBeacon min turn angle",   ATP_UINT8,  &config.trk_minangle,      0 },
        { "TRK_SLOWINTERVAL", "SmartBeacon slow interval",    ATP_UINT16, &config.trk_slowinterval,  0 },
        { "TRK_SYMBOL",       "Tracker symbol (2 chars)",     ATP_STR,    config.trk_symbol,        sizeof(config.trk_symbol) },
        { "TRK_SYMMOVE",      "Tracker moving symbol",        ATP_STR,    config.trk_symmove,       sizeof(config.trk_symmove) },
        { "TRK_SYMSTOP",      "Tracker stopped symbol",       ATP_STR,    config.trk_symstop,       sizeof(config.trk_symstop) },
        { "TRK_COMMENT",      "Tracker beacon comment",       ATP_STR,    config.trk_comment,       sizeof(config.trk_comment) },
        { "TRK_ITEM",         "Tracker item name",            ATP_STR,    config.trk_item,          sizeof(config.trk_item) },
        { "TRK_STS_INTERVAL", "Tracker status interval (sec)",ATP_UINT16, &config.trk_sts_interval,  0 },
        { "TRK_STATUS",       "Tracker status text",          ATP_STR,    config.trk_status,        sizeof(config.trk_status) },
        { "TRK_MICE_TYPE",    "MicE message type",            ATP_UINT8,  &config.trk_mice_type,     0 },
        { "TRK_TLM_INTERVAL", "Tracker TLM interval",        ATP_UINT8,  &config.trk_tlm_interval,  0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // Indexed tracker TLM arrays (N=0..4)
    {
        String r;
        r = dispatchBoolArray ("TRK_TLM_AVG",       "Tracker TLM averaging ch",  config.trk_tlm_avg,       5, cmd); if (r.length()) return r;
        r = dispatchUint8Array("TRK_TLM_SENSOR",    "Tracker TLM sensor ch",     config.trk_tlm_sensor,    5, cmd); if (r.length()) return r;
        r = dispatchUint8Array("TRK_TLM_PRECISION", "Tracker TLM precision ch",  config.trk_tlm_precision, 5, cmd); if (r.length()) return r;
        r = dispatchFloatArray("TRK_TLM_OFFSET",    "Tracker TLM offset ch",     config.trk_tlm_offset,    5, cmd); if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: WX (Weather)
// ---------------------------------------------------------------------------

static String handleWxParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "WX_EN",          "WX enable (0/1)",          ATP_BOOL,   &config.wx_en,          0 },
        { "WX_2RF",         "WX to RF (0/1)",           ATP_BOOL,   &config.wx_2rf,          0 },
        { "WX_2INET",       "WX to internet (0/1)",     ATP_BOOL,   &config.wx_2inet,        0 },
        { "WX_TIMESTAMP",   "WX timestamp (0/1)",       ATP_BOOL,   &config.wx_timestamp,    0 },
        { "WX_SSID",        "WX SSID",                  ATP_UINT8,  &config.wx_ssid,         0 },
        { "WX_MYCALL",      "WX callsign",              ATP_STR,    config.wx_mycall,       sizeof(config.wx_mycall) },
        { "WX_PATH",        "WX path index",            ATP_UINT8,  &config.wx_path,         0 },
        { "WX_GPS",         "WX use GPS (0/1)",         ATP_BOOL,   &config.wx_gps,          0 },
        { "WX_LAT",         "WX latitude",              ATP_FLOAT6, &config.wx_lat,          0 },
        { "WX_LON",         "WX longitude",             ATP_FLOAT6, &config.wx_lon,          0 },
        { "WX_ALT",         "WX altitude (m)",          ATP_FLOAT6, &config.wx_alt,          0 },
        { "WX_INTERVAL",    "WX beacon interval (sec)", ATP_UINT16, &config.wx_interval,     0 },
        { "WX_FLAGE",       "WX sensor flag bitmask",   ATP_UINT32, &config.wx_flage,        0 },
        { "WX_OBJECT",      "WX object name",           ATP_STR,    config.wx_object,       sizeof(config.wx_object) },
        { "WX_COMMENT",     "WX beacon comment",        ATP_STR,    config.wx_comment,      sizeof(config.wx_comment) },
        { "WX_TLM_INTERVAL","WX telemetry interval",    ATP_UINT8,  &config.wx_tlm_interval, 0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // Indexed WX sensor arrays (N=0..WX_SENSOR_NUM-1 = 0..25)
    {
        String r;
        r = dispatchBoolArray ("WX_SENSOR_ENABLE", "WX sensor enable ch",  config.wx_sensor_enable, WX_SENSOR_NUM, cmd); if (r.length()) return r;
        r = dispatchBoolArray ("WX_SENSOR_AVG",    "WX sensor averaging ch",config.wx_sensor_avg,   WX_SENSOR_NUM, cmd); if (r.length()) return r;
        r = dispatchUint8Array("WX_SENSOR_CH",     "WX sensor channel ch",  config.wx_sensor_ch,    WX_SENSOR_NUM, cmd); if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: TLM0 (Telemetry 0)
// ---------------------------------------------------------------------------

static String handleTlm0Params(const String& cmd)
{
    static ATParam tbl[] = {
        { "TLM0_EN",            "TLM0 enable (0/1)",           ATP_BOOL,   &config.tlm0_en,            0 },
        { "TLM0_2RF",           "TLM0 to RF (0/1)",            ATP_BOOL,   &config.tlm0_2rf,           0 },
        { "TLM0_2INET",         "TLM0 to internet (0/1)",      ATP_BOOL,   &config.tlm0_2inet,         0 },
        { "TLM0_SSID",          "TLM0 SSID",                   ATP_UINT8,  &config.tlm0_ssid,          0 },
        { "TLM0_MYCALL",        "TLM0 callsign",               ATP_STR,    config.tlm0_mycall,        sizeof(config.tlm0_mycall) },
        { "TLM0_PATH",          "TLM0 path index",             ATP_UINT8,  &config.tlm0_path,          0 },
        { "TLM0_DATA_INTERVAL", "TLM0 data tx interval (sec)", ATP_UINT16, &config.tlm0_data_interval, 0 },
        { "TLM0_INFO_INTERVAL", "TLM0 info tx interval (sec)", ATP_UINT16, &config.tlm0_info_interval, 0 },
        { "TLM0_BITS_ACTIVE",   "TLM0 BITS active mask",       ATP_UINT8,  &config.tlm0_BITS_Active,   0 },
        { "TLM0_COMMENT",       "TLM0 comment",                ATP_STR,    config.tlm0_comment,       sizeof(config.tlm0_comment) },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // Indexed TML0 data channel array (N=0..12)
    {
        String r = dispatchUint8Array("TML0_DATA_CHANNEL", "TLM0 data channel N", config.tml0_data_channel, 13, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: TLM1 (Telemetry 1)
// ---------------------------------------------------------------------------

static String handleTlm1Params(const String& cmd)
{
    static ATParam tbl[] = {
        { "TLM1_EN",            "TLM1 enable (0/1)",           ATP_BOOL,   &config.tlm1_en,            0 },
        { "TLM1_2RF",           "TLM1 to RF (0/1)",            ATP_BOOL,   &config.tlm1_2rf,           0 },
        { "TLM1_2INET",         "TLM1 to internet (0/1)",      ATP_BOOL,   &config.tlm1_2inet,         0 },
        { "TLM1_SSID",          "TLM1 SSID",                   ATP_UINT8,  &config.tlm1_ssid,          0 },
        { "TLM1_MYCALL",        "TLM1 callsign",               ATP_STR,    config.tlm1_mycall,        sizeof(config.tlm1_mycall) },
        { "TLM1_PATH",          "TLM1 path index",             ATP_UINT8,  &config.tlm1_path,          0 },
        { "TLM1_DATA_INTERVAL", "TLM1 data tx interval (sec)", ATP_UINT16, &config.tlm1_data_interval, 0 },
        { "TLM1_INFO_INTERVAL", "TLM1 info tx interval (sec)", ATP_UINT16, &config.tlm1_info_interval, 0 },
        { "TLM1_BITS_ACTIVE",   "TLM1 BITS active mask",       ATP_UINT8,  &config.tlm1_BITS_Active,   0 },
        { "TLM1_COMMENT",       "TLM1 comment",                ATP_STR,    config.tlm1_comment,       sizeof(config.tlm1_comment) },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }

    // Indexed TML1 data channel array (N=0..12)
    {
        String r = dispatchUint8Array("TML1_DATA_CHANNEL", "TLM1 data channel N", config.tml1_data_channel, 13, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Display
// ---------------------------------------------------------------------------

static String handleDisplayParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "OLED_ENABLE",      "OLED enable (0/1)",          ATP_BOOL,   &config.oled_enable,     0 },
        { "OLED_TIMEOUT",     "OLED timeout (sec)",         ATP_INT,    &config.oled_timeout,    0 },
        { "DIM",              "Display dim level",          ATP_UINT8,  &config.dim,             0 },
        { "CONTRAST",         "Display contrast",           ATP_UINT8,  &config.contrast,        0 },
        { "STARTUP",          "Startup display page",       ATP_UINT8,  &config.startup,         0 },
        { "H_UP",             "Heading-up mode (0/1)",      ATP_BOOL,   &config.h_up,            0 },
        { "TX_DISPLAY",       "Show TX packets (0/1)",      ATP_BOOL,   &config.tx_display,      0 },
        { "RX_DISPLAY",       "Show RX packets (0/1)",      ATP_BOOL,   &config.rx_display,      0 },
        { "DISPFILTER",       "Display packet filter",      ATP_UINT16, &config.dispFilter,      0 },
        { "DISPRF",           "Display RF packets (0/1)",   ATP_BOOL,   &config.dispRF,          0 },
        { "DISPINET",         "Display INET packets (0/1)", ATP_BOOL,   &config.dispINET,        0 },
        { "DISP_FLIP",        "Flip display (0/1)",         ATP_BOOL,   &config.disp_flip,       0 },
        { "DISP_BRIGHTNESS",  "Display brightness",         ATP_UINT8,  &config.disp_brightness, 0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Modem/Audio
// ---------------------------------------------------------------------------

static String handleModemParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "AUDIO_HPF",    "Audio high-pass filter (0/1)",  ATP_BOOL,   &config.audio_hpf,   0 },
        { "AUDIO_LPF",    "Audio low-pass filter (0/1)",   ATP_BOOL,   &config.audio_lpf,   0 },
        { "PREAMBLE",     "TX preamble length",            ATP_UINT8,  &config.preamble,    0 },
        { "MODEM_TYPE",   "Modem type",                    ATP_UINT8,  &config.modem_type,  0 },
        { "FX25_MODE",    "FX.25 FEC mode",                ATP_UINT8,  &config.fx25_mode,   0 },
        { "TX_TIMESLOT",  "TX CSMA time slot (ms)",        ATP_UINT16, &config.tx_timeslot, 0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Network / VPN / PPP
// ---------------------------------------------------------------------------

static String handleNetworkParams(const String& cmd)
{
    static ATParam tbl[] = {
        // VPN / WireGuard
        { "VPN",               "WireGuard VPN enable (0/1)",   ATP_BOOL,   &config.vpn,                  0 },
        { "MODEM",             "PPP modem enable (0/1)",       ATP_BOOL,   &config.modem,                 0 },
        { "WG_PORT",           "WireGuard UDP port",           ATP_UINT16, &config.wg_port,               0 },
        { "WG_PEER_ADDRESS",   "WireGuard peer address",       ATP_STR,    config.wg_peer_address,       sizeof(config.wg_peer_address) },
        { "WG_LOCAL_ADDRESS",  "WireGuard local IP",           ATP_STR,    config.wg_local_address,      sizeof(config.wg_local_address) },
        { "WG_NETMASK_ADDRESS","WireGuard netmask",            ATP_STR,    config.wg_netmask_address,    sizeof(config.wg_netmask_address) },
        { "WG_GW_ADDRESS",     "WireGuard gateway",            ATP_STR,    config.wg_gw_address,         sizeof(config.wg_gw_address) },
        { "WG_PUBLIC_KEY",     "WireGuard peer public key",    ATP_STR,    config.wg_public_key,         sizeof(config.wg_public_key) },
        { "WG_PRIVATE_KEY",    "WireGuard local private key",  ATP_STR,    config.wg_private_key,        sizeof(config.wg_private_key) },
        // HTTP auth
        { "HTTP_USERNAME",     "Web interface username",       ATP_STR,    config.http_username,         sizeof(config.http_username) },
        { "HTTP_PASSWORD",     "Web interface password",       ATP_STR,    config.http_password,         sizeof(config.http_password) },
        // PPP
        { "PPP_ENABLE",        "PPP enable (0/1)",             ATP_BOOL,   &config.ppp_enable,            0 },
        { "PPP_APN",           "PPP APN",                      ATP_STR,    config.ppp_apn,               sizeof(config.ppp_apn) },
        { "PPP_PIN",           "PPP SIM PIN",                  ATP_STR,    config.ppp_pin,               sizeof(config.ppp_pin) },
        { "PPP_RST_GPIO",      "PPP modem reset GPIO",         ATP_INT8,   &config.ppp_rst_gpio,          0 },
        { "PPP_TX_GPIO",       "PPP TX GPIO",                  ATP_INT8,   &config.ppp_tx_gpio,           0 },
        { "PPP_RX_GPIO",       "PPP RX GPIO",                  ATP_INT8,   &config.ppp_rx_gpio,           0 },
        { "PPP_RTS_GPIO",      "PPP RTS GPIO",                 ATP_INT8,   &config.ppp_rts_gpio,          0 },
        { "PPP_CTS_GPIO",      "PPP CTS GPIO",                 ATP_INT8,   &config.ppp_cts_gpio,          0 },
        { "PPP_DTR_GPIO",      "PPP DTR GPIO",                 ATP_INT8,   &config.ppp_dtr_gpio,          0 },
        { "PPP_RI_GPIO",       "PPP RI GPIO",                  ATP_INT8,   &config.ppp_ri_gpio,           0 },
        { "PPP_RST_ACTIVE",    "PPP reset active level (0/1)", ATP_BOOL,   &config.ppp_rst_active,        0 },
        { "PPP_RST_DELAY",     "PPP reset delay (ms)",         ATP_UINT16, &config.ppp_rst_delay,         0 },
        { "PPP_PWR_GPIO",      "PPP power GPIO",               ATP_INT8,   &config.ppp_pwr_gpio,          0 },
        { "PPP_PWR_ACTIVE",    "PPP power active level (0/1)", ATP_BOOL,   &config.ppp_pwr_active,        0 },
        { "PPP_SERIAL",        "PPP serial port index",        ATP_UINT8,  &config.ppp_serial,            0 },
        { "PPP_MODEL",         "PPP modem model",              ATP_UINT8,  &config.ppp_model,             0 },
        { "PPP_FLOW_CTRL",     "PPP flow control",             ATP_UINT8,  &config.ppp_flow_ctrl,         0 },
        { "PPP_GNSS",          "PPP modem has GNSS (0/1)",     ATP_BOOL,   &config.ppp_gnss,              0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: GNSS
// ---------------------------------------------------------------------------

static String handleGnssParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "GNSS_ENABLE",     "GNSS enable (0/1)",        ATP_BOOL,   &config.gnss_enable,    0 },
        { "GNSS_PPS_GPIO",   "GNSS PPS GPIO pin",        ATP_INT8,   &config.gnss_pps_gpio,  0 },
        { "GNSS_CHANNEL",    "GNSS serial channel",      ATP_INT8,   &config.gnss_channel,   0 },
        { "GNSS_TCP_PORT",   "GNSS TCP forwarding port", ATP_UINT16, &config.gnss_tcp_port,  0 },
        { "GNSS_TCP_HOST",   "GNSS TCP forwarding host", ATP_STR,    config.gnss_tcp_host,  sizeof(config.gnss_tcp_host) },
        { "GNSS_AT_COMMAND", "GNSS init AT command",     ATP_STR,    config.gnss_at_command,sizeof(config.gnss_at_command) },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: HW I/O (I2C, UART, OneWire, Modbus, Counter, EXT TNC)
// ---------------------------------------------------------------------------

static String handleHwIoParams(const String& cmd)
{
    static ATParam tbl[] = {
        // I2C bus 0
        { "I2C_ENABLE",   "I2C bus 0 enable (0/1)",   ATP_BOOL,   &config.i2c_enable,    0 },
        { "I2C_SDA_PIN",  "I2C bus 0 SDA pin",         ATP_INT8,   &config.i2c_sda_pin,   0 },
        { "I2C_SCK_PIN",  "I2C bus 0 SCK pin",         ATP_INT8,   &config.i2c_sck_pin,   0 },
        { "I2C_RST_PIN",  "I2C bus 0 reset pin",       ATP_INT8,   &config.i2c_rst_pin,   0 },
        { "I2C_FREQ",     "I2C bus 0 frequency (Hz)",  ATP_UINT32, &config.i2c_freq,      0 },
        // I2C bus 1
        { "I2C1_ENABLE",  "I2C bus 1 enable (0/1)",   ATP_BOOL,   &config.i2c1_enable,   0 },
        { "I2C1_SDA_PIN", "I2C bus 1 SDA pin",         ATP_INT8,   &config.i2c1_sda_pin,  0 },
        { "I2C1_SCK_PIN", "I2C bus 1 SCK pin",         ATP_INT8,   &config.i2c1_sck_pin,  0 },
        { "I2C1_FREQ",    "I2C bus 1 frequency (Hz)",  ATP_UINT32, &config.i2c1_freq,     0 },
        // OneWire
        { "ONEWIRE_ENABLE", "OneWire enable (0/1)",    ATP_BOOL,   &config.onewire_enable, 0 },
        { "ONEWIRE_GPIO",   "OneWire GPIO pin",         ATP_INT8,   &config.onewire_gpio,   0 },
        // UART 0
        { "UART0_ENABLE",   "UART0 enable (0/1)",      ATP_BOOL,   &config.uart0_enable,   0 },
        { "UART0_TX_GPIO",  "UART0 TX GPIO pin",        ATP_INT8,   &config.uart0_tx_gpio,  0 },
        { "UART0_RX_GPIO",  "UART0 RX GPIO pin",        ATP_INT8,   &config.uart0_rx_gpio,  0 },
        { "UART0_RTS_GPIO", "UART0 RTS GPIO pin",       ATP_INT8,   &config.uart0_rts_gpio, 0 },
        // UART 1
        { "UART1_ENABLE",   "UART1 enable (0/1)",      ATP_BOOL,   &config.uart1_enable,   0 },
        { "UART1_TX_GPIO",  "UART1 TX GPIO pin",        ATP_INT8,   &config.uart1_tx_gpio,  0 },
        { "UART1_RX_GPIO",  "UART1 RX GPIO pin",        ATP_INT8,   &config.uart1_rx_gpio,  0 },
        { "UART1_RTS_GPIO", "UART1 RTS GPIO pin",       ATP_INT8,   &config.uart1_rts_gpio, 0 },
        // Modbus
        { "MODBUS_ENABLE",   "Modbus enable (0/1)",    ATP_BOOL,   &config.modbus_enable,  0 },
        { "MODBUS_ADDRESS",  "Modbus device address",  ATP_UINT8,  &config.modbus_address, 0 },
        { "MODBUS_CHANNEL",  "Modbus serial channel",  ATP_INT8,   &config.modbus_channel, 0 },
        { "MODBUS_DE_GPIO",  "Modbus DE/RE GPIO pin",  ATP_INT8,   &config.modbus_de_gpio, 0 },
        // Counter 0
        { "COUNTER0_ENABLE", "Counter0 enable (0/1)",  ATP_BOOL,   &config.counter0_enable, 0 },
        { "COUNTER0_ACTIVE", "Counter0 active level",  ATP_BOOL,   &config.counter0_active, 0 },
        { "COUNTER0_GPIO",   "Counter0 GPIO pin",       ATP_INT8,   &config.counter0_gpio,   0 },
        // Counter 1
        { "COUNTER1_ENABLE", "Counter1 enable (0/1)",  ATP_BOOL,   &config.counter1_enable, 0 },
        { "COUNTER1_ACTIVE", "Counter1 active level",  ATP_BOOL,   &config.counter1_active, 0 },
        { "COUNTER1_GPIO",   "Counter1 GPIO pin",       ATP_INT8,   &config.counter1_gpio,   0 },
        // External TNC
        { "EXT_TNC_ENABLE",  "Ext TNC enable (0/1)",   ATP_BOOL,   &config.ext_tnc_enable,  0 },
        { "EXT_TNC_CHANNEL", "Ext TNC serial channel", ATP_INT8,   &config.ext_tnc_channel, 0 },
        { "EXT_TNC_MODE",    "Ext TNC mode",            ATP_INT8,   &config.ext_tnc_mode,    0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Power
// ---------------------------------------------------------------------------

static String handlePowerParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "PWR_EN",             "Power management enable (0/1)",   ATP_BOOL,   &config.pwr_en,              0 },
        { "PWR_MODE",           "Power mode",                      ATP_UINT8,  &config.pwr_mode,            0 },
        { "PWR_SLEEP_INTERVAL", "Sleep interval (sec)",            ATP_UINT16, &config.pwr_sleep_interval,  0 },
        { "PWR_STANBY_DELAY",   "Standby delay (sec)",             ATP_UINT16, &config.pwr_stanby_delay,    0 },
        { "PWR_SLEEP_ACTIVATE", "Sleep activate source",           ATP_UINT8,  &config.pwr_sleep_activate,  0 },
        { "PWR_GPIO",           "Power GPIO pin",                  ATP_INT8,   &config.pwr_gpio,            0 },
        { "PWR_ACTIVE",         "Power active level (0/1)",        ATP_BOOL,   &config.pwr_active,          0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: MQTT (conditional)
// ---------------------------------------------------------------------------

static String handleMqttParams(const String& cmd)
{
#ifdef MQTT
    static ATParam tbl[] = {
        { "EN_MQTT",             "MQTT enable (0/1)",           ATP_BOOL,   &config.en_mqtt,             0 },
        { "MQTT_HOST",           "MQTT broker hostname",        ATP_STR,    config.mqtt_host,           sizeof(config.mqtt_host) },
        { "MQTT_TOPIC",          "MQTT publish topic",          ATP_STR,    config.mqtt_topic,          sizeof(config.mqtt_topic) },
        { "MQTT_SUBSCRIBE",      "MQTT subscribe topic",        ATP_STR,    config.mqtt_subscribe,      sizeof(config.mqtt_subscribe) },
        { "MQTT_USER",           "MQTT username",               ATP_STR,    config.mqtt_user,           sizeof(config.mqtt_user) },
        { "MQTT_PASS",           "MQTT password",               ATP_STR,    config.mqtt_pass,           sizeof(config.mqtt_pass) },
        { "MQTT_PORT",           "MQTT broker port",            ATP_UINT16, &config.mqtt_port,           0 },
        { "MQTT_TOPIC_FLAG",     "MQTT topic filter flags",     ATP_UINT16, &config.mqtt_topic_flag,     0 },
        { "MQTT_SUBSCRIBE_FLAG", "MQTT subscribe filter flags", ATP_UINT16, &config.mqtt_subscribe_flag, 0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
#endif
    return "";
}

// ---------------------------------------------------------------------------
// Group handler: Message
// ---------------------------------------------------------------------------

static String handleMessageParams(const String& cmd)
{
    static ATParam tbl[] = {
        { "MSG_ENABLE",   "Message enable (0/1)",       ATP_BOOL,  &config.msg_enable,  0 },
        { "MSG_MYCALL",   "Message callsign",           ATP_STR,   config.msg_mycall,  sizeof(config.msg_mycall) },
        { "MSG_PATH",     "Message path index",         ATP_UINT8, &config.msg_path,   0 },
        { "MSG_RF",       "Message via RF (0/1)",       ATP_BOOL,  &config.msg_rf,     0 },
        { "MSG_INET",     "Message via internet (0/1)", ATP_BOOL,  &config.msg_inet,   0 },
        { "MSG_ENCRYPT",  "Message encryption (0/1)",   ATP_BOOL,  &config.msg_encrypt,0 },
        { "MSG_KEY",      "Message encryption key",     ATP_STR,   config.msg_key,    sizeof(config.msg_key) },
        { "MSG_RETRY",    "Message retry count",        ATP_UINT8, &config.msg_retry,  0 },
        { "MSG_INTERVAL", "Message retry interval (s)", ATP_UINT16,&config.msg_interval,0 },
    };
    for (auto& p : tbl) {
        String r = dispatchParam(p, cmd);
        if (r.length()) return r;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Time command handler (kept as special case due to getValue() dependency)
// ---------------------------------------------------------------------------

static String handleTimeCommand(const String& cmd)
{
    if (cmd == "AT+TIME?") {
        struct tm tmstruct;
        char strTime[20];
        tmstruct.tm_year = 0;
        getLocalTime(&tmstruct, 100);
        sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d",
                (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday,
                tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
        return String(strTime);
    }

    if (cmd.startsWith("AT+TIME=")) {
        String timeStr = cmd.substring(8);
        timeStr.replace("\"", "");
        char strTime[20];
        strncpy(strTime, timeStr.c_str(), sizeof(strTime) - 1);
        strTime[sizeof(strTime) - 1] = '\0';

        String date = getValue(strTime, ' ', 0);
        String time = getValue(strTime, ' ', 1);
        int yyyy = getValue(date, '-', 0).toInt();
        int mm   = getValue(date, '-', 1).toInt();
        int dd   = getValue(date, '-', 2).toInt();
        int hh   = getValue(time, ':', 0).toInt();
        int ii   = getValue(time, ':', 1).toInt();
        int ss   = getValue(time, ':', 2).toInt();

        tmElements_t timeinfo;
        timeinfo.Year   = yyyy - 1970;
        timeinfo.Month  = mm;
        timeinfo.Day    = dd;
        timeinfo.Hour   = hh;
        timeinfo.Minute = ii;
        timeinfo.Second = ss;
        time_t timeStamp = makeTime(timeinfo);
        time_t rtc = timeStamp - (config.timeZone * 3600);
        timeval tv = {rtc, 0};
        timezone tz = {(0) + DST_MN, 0};
        settimeofday(&tv, &tz);
        log_d("Set Time: %s", strTime);
        return String("Set Time: ") + String(strTime);
    }

    return "";
}

// ---------------------------------------------------------------------------
// buildTopLevelHelp() - list special commands and groups only
// ---------------------------------------------------------------------------

static String buildTopLevelHelp()
{
    String out;

    out += "=== Special Commands ===\n";
    out += "AT                     OK test\n";
    out += "AT?                    Show this help\n";
    out += "AT+HELP                Show this help\n";
    out += "AT+RESET               Restart device\n";
    out += "AT+RESTART             Restart device\n";
    out += "AT+VERSION?            Firmware/SDK version\n";
    out += "AT+CHIPID?             Chip unique ID\n";
    out += "AT+SAVECONFIG          Save config to flash\n";
    out += "AT+LOADCONFIG          Load config from flash\n";
    out += "AT+WIFI_DISCONNECT     Disconnect WiFi\n";
    out += "AT+WIFI_CONNECT        Reconnect WiFi\n";
    out += "AT+WIFI_STATUS?        WiFi connection status\n";
    out += "AT+WIFI_SCAN           Scan WiFi networks\n";
    out += "AT+WIFI?               Current WiFi mode/SSID/RSSI\n";
    out += "AT+MODE?               Current operational modes\n";
    out += "AT+TIME?               Current RTC time\n";
    out += "AT+TIME=YYYY-MM-DD HH:MM:SS  Set RTC time\n";

    out += "\n=== Command Groups (use AT+<GROUP>? for group help) ===\n";
    out += "AT+SYSTEM?             System (timezone, NTP, hostname, etc.)\n";
    out += "AT+WIFI_HELP?          WiFi config (AT+WIFI? = status)\n";
    out += "AT+BT?                 Bluetooth\n";
    out += "AT+RF?                 RF module\n";
    out += "AT+IGATE?              iGate\n";
    out += "AT+DIGI?               Digipeater\n";
    out += "AT+TRK?                Tracker\n";
    out += "AT+WX?                 Weather\n";
    out += "AT+TLM0?               Telemetry 0\n";
    out += "AT+TLM1?               Telemetry 1\n";
    out += "AT+DISPLAY?            Display\n";
    out += "AT+MODEM?              Modem/Audio\n";
    out += "AT+NETWORK?            Network/VPN/PPP\n";
    out += "AT+GNSS?               GNSS\n";
    out += "AT+HWIO?               HW I/O (I2C, UART, OneWire, etc.)\n";
    out += "AT+PWR?                Power\n";
#ifdef MQTT
    out += "AT+MQTT?               MQTT\n";
#endif
    out += "AT+MSG?                Message\n";

    return out;
}

// ---------------------------------------------------------------------------
// buildGroupHelp() - help for a specific command group
// ---------------------------------------------------------------------------

static String buildGroupHelp(const char* group)
{
    String out;

    out += "=== ";
    out += group;
    out += " ===\n";

    // Helper lambda to print one group's params
    auto printGroup = [&out](const ATParam* tbl, size_t n) {
        for (size_t i = 0; i < n; i++) {
            char buf[80];
            snprintf(buf, sizeof(buf), "AT+%-30s %s\n", tbl[i].name, tbl[i].desc);
            out += buf;
        }
    };

    if (strcmp(group, "System") == 0) {
        static ATParam tbl[] = {
            { "TIMEZONE",      "Timezone offset (float)",       ATP_FLOAT6, nullptr, 0 },
            { "SYNCTIME",      "Enable NTP sync (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "TITLE",         "Show title on display (0/1)",   ATP_BOOL,   nullptr, 0 },
            { "LOG",           "Log level bitmask",             ATP_UINT16, nullptr, 0 },
            { "HOST_NAME",     "mDNS hostname",                 ATP_STR,    nullptr, 0 },
            { "RESET_TIMEOUT", "Watchdog reset timeout (min)",  ATP_UINT16, nullptr, 0 },
            { "NTP_HOST",      "NTP server hostname",           ATP_STR,    nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "WiFi") == 0) {
        out += "AT+WIFI_MODE              WiFi mode (0=OFF,1=STA,2=AP,3=AP+STA)\n";
        out += "AT+WIFI_POWER             WiFi TX power\n";
        for (int i = 0; i < 5; i++) {
            out += String("AT+WIFI") + i + "EN              WiFi STA " + i + " enable\n";
            out += String("AT+WIFI") + i + "SSID            WiFi STA " + i + " SSID\n";
            out += String("AT+WIFI") + i + "PASS            WiFi STA " + i + " password\n";
        }
        out += "AT+WIFI_AP_CH             AP channel\n";
        out += "AT+WIFI_AP_SSID           AP SSID\n";
        out += "AT+WIFI_AP_PASS           AP password\n";
    }
    else if (strcmp(group, "Bluetooth") == 0) {
        static ATParam tbl[] = {
            { "BT_SLAVE",    "BT slave mode (0/1)",  ATP_BOOL,   nullptr, 0 },
            { "BT_MASTER",   "BT master mode (0/1)", ATP_BOOL,   nullptr, 0 },
            { "BT_MODE",     "BT mode",              ATP_UINT8,  nullptr, 0 },
            { "BT_UUID",     "BT service UUID",      ATP_STR,    nullptr, 0 },
            { "BT_UUID_RX",  "BT RX char UUID",      ATP_STR,    nullptr, 0 },
            { "BT_UUID_TX",  "BT TX char UUID",      ATP_STR,    nullptr, 0 },
            { "BT_NAME",     "BT device name",       ATP_STR,    nullptr, 0 },
            { "BT_PIN",      "BT pairing PIN",       ATP_UINT32, nullptr, 0 },
            { "BT_POWER",    "BT TX power level",    ATP_UINT8,  nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "RF") == 0) {
        static ATParam tbl[] = {
            { "RF_EN",        "RF module enable (0/1)",    ATP_BOOL,   nullptr, 0 },
            { "RF_TYPE",      "RF module type",            ATP_UINT8,  nullptr, 0 },
            { "FREQ_RX",      "RX frequency (MHz)",        ATP_FLOAT6, nullptr, 0 },
            { "FREQ_TX",      "TX frequency (MHz)",        ATP_FLOAT6, nullptr, 0 },
            { "OFFSET_RX",    "RX offset (Hz)",            ATP_INT,    nullptr, 0 },
            { "OFFSET_TX",    "TX offset (Hz)",            ATP_INT,    nullptr, 0 },
            { "TONE_RX",      "RX CTCSS tone",             ATP_INT,    nullptr, 0 },
            { "TONE_TX",      "TX CTCSS tone",             ATP_INT,    nullptr, 0 },
            { "BAND",         "RF band",                   ATP_UINT8,  nullptr, 0 },
            { "SQL_LEVEL",    "Squelch level",             ATP_UINT8,  nullptr, 0 },
            { "RF_POWER",     "RF power high (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "VOLUME",       "Audio volume",              ATP_UINT8,  nullptr, 0 },
            { "MIC",          "Mic gain",                  ATP_UINT8,  nullptr, 0 },
            { "RF_TX_GPIO",   "RF TX GPIO pin",            ATP_INT8,   nullptr, 0 },
            { "RF_RX_GPIO",   "RF RX GPIO pin",            ATP_INT8,   nullptr, 0 },
            { "RF_SQL_GPIO",  "RF squelch GPIO pin",       ATP_INT8,   nullptr, 0 },
            { "RF_PD_GPIO",   "RF power-down GPIO pin",    ATP_INT8,   nullptr, 0 },
            { "RF_PWR_GPIO",  "RF power GPIO pin",         ATP_INT8,   nullptr, 0 },
            { "RF_PTT_GPIO",  "RF PTT GPIO pin",           ATP_INT8,   nullptr, 0 },
            { "RF_SQL_ACTIVE","RF squelch active level",   ATP_BOOL,   nullptr, 0 },
            { "RF_PD_ACTIVE", "RF power-down active level",ATP_BOOL,   nullptr, 0 },
            { "RF_PWR_ACTIVE","RF power active level",     ATP_BOOL,   nullptr, 0 },
            { "RF_PTT_ACTIVE","RF PTT active level",       ATP_BOOL,   nullptr, 0 },
            { "ADC_GPIO",     "ADC GPIO pin",              ATP_INT8,   nullptr, 0 },
            { "DAC_GPIO",     "DAC GPIO pin",              ATP_INT8,   nullptr, 0 },
            { "ADC_SEL_GPIO", "ADC select GPIO pin",       ATP_INT8,   nullptr, 0 },
            { "DAC_SEL_GPIO", "DAC select GPIO pin",       ATP_INT8,   nullptr, 0 },
            { "ADC_ATTEN",    "ADC attenuation",           ATP_UINT8,  nullptr, 0 },
            { "ADC_DC_OFFSET","ADC DC offset",             ATP_UINT16, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "IGATE") == 0) {
        static ATParam tbl[] = {
            { "IGATE_EN",           "iGate enable (0/1)",           ATP_BOOL,   nullptr, 0 },
            { "RF2INET",            "RF to internet (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "INET2RF",            "Internet to RF (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "IGATE_LOC2RF",       "iGate loc beacon to RF (0/1)", ATP_BOOL,   nullptr, 0 },
            { "IGATE_LOC2INET",     "iGate loc beacon to IS (0/1)", ATP_BOOL,   nullptr, 0 },
            { "RF2INETFILTER",      "RF-to-IS packet filter",       ATP_UINT16, nullptr, 0 },
            { "INET2RFFILTER",      "IS-to-RF packet filter",       ATP_UINT16, nullptr, 0 },
            { "APRS_SSID",          "APRS-IS SSID",                 ATP_UINT8,  nullptr, 0 },
            { "APRS_PORT",          "APRS-IS port",                 ATP_UINT16, nullptr, 0 },
            { "APRS_MYCALL",        "APRS callsign",                ATP_STR,    nullptr, 0 },
            { "APRS_HOST",          "APRS-IS server host",          ATP_STR,    nullptr, 0 },
            { "APRS_PASSCODE",      "APRS-IS passcode",             ATP_STR,    nullptr, 0 },
            { "APRS_MONICALL",      "APRS monitor callsign",        ATP_STR,    nullptr, 0 },
            { "APRS_FILTER",        "APRS-IS server filter",        ATP_STR,    nullptr, 0 },
            { "IGATE_BCN",          "iGate beacon enable (0/1)",    ATP_BOOL,   nullptr, 0 },
            { "IGATE_GPS",          "iGate use GPS (0/1)",          ATP_BOOL,   nullptr, 0 },
            { "IGATE_TIMESTAMP",    "iGate timestamp (0/1)",        ATP_BOOL,   nullptr, 0 },
            { "IGATE_LAT",          "iGate latitude",               ATP_FLOAT6, nullptr, 0 },
            { "IGATE_LON",          "iGate longitude",              ATP_FLOAT6, nullptr, 0 },
            { "IGATE_ALT",          "iGate altitude (m)",           ATP_FLOAT6, nullptr, 0 },
            { "IGATE_INTERVAL",     "iGate beacon interval (sec)",  ATP_UINT16, nullptr, 0 },
            { "IGATE_SYMBOL",       "iGate symbol (2 chars)",       ATP_STR,    nullptr, 0 },
            { "IGATE_OBJECT",       "iGate object name",            ATP_STR,    nullptr, 0 },
            { "IGATE_PHG",          "iGate PHG string",             ATP_STR,    nullptr, 0 },
            { "IGATE_PATH",         "iGate path index",             ATP_UINT8,  nullptr, 0 },
            { "IGATE_COMMENT",      "iGate beacon comment",         ATP_STR,    nullptr, 0 },
            { "IGATE_STS_INTERVAL", "iGate status interval (sec)",  ATP_UINT16, nullptr, 0 },
            { "IGATE_STATUS",       "iGate status text",            ATP_STR,    nullptr, 0 },
            { "IGATE_TLM_INTERVAL", "iGate telemetry interval",     ATP_UINT8,  nullptr, 0 },
            { "IGATE_TLM_AVG<N>",   "iGate TLM averaging ch (N=0..4)",   ATP_BOOL,   nullptr, 0 },
            { "IGATE_TLM_SENSOR<N>","iGate TLM sensor ch (N=0..4)",      ATP_UINT8,  nullptr, 0 },
            { "IGATE_TLM_PRECISION<N>","iGate TLM precision ch (N=0..4)",ATP_UINT8,  nullptr, 0 },
            { "IGATE_TLM_OFFSET<N>","iGate TLM offset ch (N=0..4)",      ATP_FLOAT6, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "DIGI") == 0) {
        static ATParam tbl[] = {
            { "DIGI_EN",           "Digipeater enable (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "DIGI_AUTO",         "Digi auto mode (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "DIGI_LOC2RF",       "Digi loc to RF (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "DIGI_LOC2INET",     "Digi loc to IS (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "DIGI_TIMESTAMP",    "Digi timestamp (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "DIGI_SSID",         "Digi SSID",                    ATP_UINT8,  nullptr, 0 },
            { "DIGI_MYCALL",       "Digi callsign",                ATP_STR,    nullptr, 0 },
            { "DIGI_PATH",         "Digi path index",              ATP_UINT8,  nullptr, 0 },
            { "DIGI_DELAY",        "Digi TX delay (ms)",           ATP_UINT16, nullptr, 0 },
            { "DIGIFILTER",        "Digi packet filter",           ATP_UINT16, nullptr, 0 },
            { "DIGI_BCN",          "Digi beacon enable (0/1)",     ATP_BOOL,   nullptr, 0 },
            { "DIGI_GPS",          "Digi use GPS (0/1)",           ATP_BOOL,   nullptr, 0 },
            { "DIGI_LAT",          "Digi latitude",                ATP_FLOAT6, nullptr, 0 },
            { "DIGI_LON",          "Digi longitude",               ATP_FLOAT6, nullptr, 0 },
            { "DIGI_ALT",          "Digi altitude (m)",            ATP_FLOAT6, nullptr, 0 },
            { "DIGI_INTERVAL",     "Digi beacon interval (sec)",   ATP_UINT16, nullptr, 0 },
            { "DIGI_SYMBOL",       "Digi symbol (2 chars)",        ATP_STR,    nullptr, 0 },
            { "DIGI_PHG",          "Digi PHG string",              ATP_STR,    nullptr, 0 },
            { "DIGI_COMMENT",      "Digi beacon comment",          ATP_STR,    nullptr, 0 },
            { "DIGI_STS_INTERVAL", "Digi status interval (sec)",   ATP_UINT16, nullptr, 0 },
            { "DIGI_STATUS",       "Digi status text",             ATP_STR,    nullptr, 0 },
            { "DIGI_TLM_INTERVAL", "Digi telemetry interval",      ATP_UINT8,  nullptr, 0 },
            { "DIGI_TLM_AVG<N>",   "Digi TLM averaging ch (N=0..4)",   ATP_BOOL,   nullptr, 0 },
            { "DIGI_TLM_SENSOR<N>","Digi TLM sensor ch (N=0..4)",      ATP_UINT8,  nullptr, 0 },
            { "DIGI_TLM_PRECISION<N>","Digi TLM precision ch (N=0..4)",ATP_UINT8,  nullptr, 0 },
            { "DIGI_TLM_OFFSET<N>","Digi TLM offset ch (N=0..4)",      ATP_FLOAT6, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "Tracker") == 0) {
        static ATParam tbl[] = {
            { "TRK_EN",           "Tracker enable (0/1)",          ATP_BOOL,   nullptr, 0 },
            { "TRK_LOC2RF",       "Tracker loc to RF (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "TRK_LOC2INET",     "Tracker loc to IS (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "TRK_TIMESTAMP",    "Tracker timestamp (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "TRK_SSID",         "Tracker SSID",                  ATP_UINT8,  nullptr, 0 },
            { "TRK_MYCALL",       "Tracker callsign",              ATP_STR,    nullptr, 0 },
            { "TRK_PATH",         "Tracker path index",            ATP_UINT8,  nullptr, 0 },
            { "TRK_GPS",          "Tracker use GPS (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "TRK_LAT",          "Tracker latitude",              ATP_FLOAT6, nullptr, 0 },
            { "TRK_LON",          "Tracker longitude",             ATP_FLOAT6, nullptr, 0 },
            { "TRK_ALT",          "Tracker altitude (m)",          ATP_FLOAT6, nullptr, 0 },
            { "TRK_INTERVAL",     "Tracker beacon interval (sec)", ATP_UINT16, nullptr, 0 },
            { "TRK_SMARTBEACON",  "SmartBeacon enable (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "TRK_COMPRESS",     "Compressed position (0/1)",     ATP_BOOL,   nullptr, 0 },
            { "TRK_ALTITUDE",     "Include altitude (0/1)",        ATP_BOOL,   nullptr, 0 },
            { "TRK_LOG",          "Tracker logging (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "TRK_RSSI",         "Include RSSI (0/1)",            ATP_BOOL,   nullptr, 0 },
            { "TRK_SAT",          "Include sat count (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "TRK_DX",           "Include DX info (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "TRK_HSPEED",       "SmartBeacon high speed (kph)",  ATP_UINT16, nullptr, 0 },
            { "TRK_LSPEED",       "SmartBeacon low speed (kph)",   ATP_UINT8,  nullptr, 0 },
            { "TRK_MAXINTERVAL",  "SmartBeacon max interval",      ATP_UINT8,  nullptr, 0 },
            { "TRK_MININTERVAL",  "SmartBeacon min interval",      ATP_UINT8,  nullptr, 0 },
            { "TRK_MINANGLE",     "SmartBeacon min turn angle",    ATP_UINT8,  nullptr, 0 },
            { "TRK_SLOWINTERVAL", "SmartBeacon slow interval",     ATP_UINT16, nullptr, 0 },
            { "TRK_SYMBOL",       "Tracker symbol (2 chars)",      ATP_STR,    nullptr, 0 },
            { "TRK_SYMMOVE",      "Tracker moving symbol",         ATP_STR,    nullptr, 0 },
            { "TRK_SYMSTOP",      "Tracker stopped symbol",        ATP_STR,    nullptr, 0 },
            { "TRK_COMMENT",      "Tracker beacon comment",        ATP_STR,    nullptr, 0 },
            { "TRK_ITEM",         "Tracker item name",             ATP_STR,    nullptr, 0 },
            { "TRK_STS_INTERVAL", "Tracker status interval (sec)", ATP_UINT16, nullptr, 0 },
            { "TRK_STATUS",       "Tracker status text",           ATP_STR,    nullptr, 0 },
            { "TRK_MICE_TYPE",    "MicE message type",             ATP_UINT8,  nullptr, 0 },
            { "TRK_TLM_INTERVAL", "Tracker TLM interval",         ATP_UINT8,  nullptr, 0 },
            { "TRK_TLM_AVG<N>",   "Tracker TLM averaging ch (N=0..4)",   ATP_BOOL,   nullptr, 0 },
            { "TRK_TLM_SENSOR<N>","Tracker TLM sensor ch (N=0..4)",      ATP_UINT8,  nullptr, 0 },
            { "TRK_TLM_PRECISION<N>","Tracker TLM precision ch (N=0..4)",ATP_UINT8,  nullptr, 0 },
            { "TRK_TLM_OFFSET<N>","Tracker TLM offset ch (N=0..4)",      ATP_FLOAT6, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "WX (Weather)") == 0) {
        static ATParam tbl[] = {
            { "WX_EN",           "WX enable (0/1)",           ATP_BOOL,   nullptr, 0 },
            { "WX_2RF",          "WX to RF (0/1)",            ATP_BOOL,   nullptr, 0 },
            { "WX_2INET",        "WX to internet (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "WX_TIMESTAMP",    "WX timestamp (0/1)",        ATP_BOOL,   nullptr, 0 },
            { "WX_SSID",         "WX SSID",                   ATP_UINT8,  nullptr, 0 },
            { "WX_MYCALL",       "WX callsign",               ATP_STR,    nullptr, 0 },
            { "WX_PATH",         "WX path index",             ATP_UINT8,  nullptr, 0 },
            { "WX_GPS",          "WX use GPS (0/1)",          ATP_BOOL,   nullptr, 0 },
            { "WX_LAT",          "WX latitude",               ATP_FLOAT6, nullptr, 0 },
            { "WX_LON",          "WX longitude",              ATP_FLOAT6, nullptr, 0 },
            { "WX_ALT",          "WX altitude (m)",           ATP_FLOAT6, nullptr, 0 },
            { "WX_INTERVAL",     "WX beacon interval (sec)",  ATP_UINT16, nullptr, 0 },
            { "WX_FLAGE",        "WX sensor flag bitmask",    ATP_UINT32, nullptr, 0 },
            { "WX_OBJECT",       "WX object name",            ATP_STR,    nullptr, 0 },
            { "WX_COMMENT",      "WX beacon comment",         ATP_STR,    nullptr, 0 },
            { "WX_TLM_INTERVAL", "WX telemetry interval",     ATP_UINT8,  nullptr, 0 },
            { "WX_SENSOR_ENABLE<N>","WX sensor enable ch (N=0..25)",    ATP_BOOL,  nullptr, 0 },
            { "WX_SENSOR_AVG<N>",   "WX sensor averaging ch (N=0..25)", ATP_BOOL,  nullptr, 0 },
            { "WX_SENSOR_CH<N>",    "WX sensor channel ch (N=0..25)",   ATP_UINT8, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "TLM0 (Telemetry 0)") == 0) {
        static ATParam tbl[] = {
            { "TLM0_EN",             "TLM0 enable (0/1)",           ATP_BOOL,   nullptr, 0 },
            { "TLM0_2RF",            "TLM0 to RF (0/1)",            ATP_BOOL,   nullptr, 0 },
            { "TLM0_2INET",          "TLM0 to internet (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "TLM0_SSID",           "TLM0 SSID",                   ATP_UINT8,  nullptr, 0 },
            { "TLM0_MYCALL",         "TLM0 callsign",               ATP_STR,    nullptr, 0 },
            { "TLM0_PATH",           "TLM0 path index",             ATP_UINT8,  nullptr, 0 },
            { "TLM0_DATA_INTERVAL",  "TLM0 data tx interval (sec)", ATP_UINT16, nullptr, 0 },
            { "TLM0_INFO_INTERVAL",  "TLM0 info tx interval (sec)", ATP_UINT16, nullptr, 0 },
            { "TLM0_BITS_ACTIVE",    "TLM0 BITS active mask",       ATP_UINT8,  nullptr, 0 },
            { "TLM0_COMMENT",        "TLM0 comment",                ATP_STR,    nullptr, 0 },
            { "TML0_DATA_CHANNEL<N>","TLM0 data channel N (N=0..12)",ATP_UINT8, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "TLM1 (Telemetry 1)") == 0) {
        static ATParam tbl[] = {
            { "TLM1_EN",             "TLM1 enable (0/1)",           ATP_BOOL,   nullptr, 0 },
            { "TLM1_2RF",            "TLM1 to RF (0/1)",            ATP_BOOL,   nullptr, 0 },
            { "TLM1_2INET",          "TLM1 to internet (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "TLM1_SSID",           "TLM1 SSID",                   ATP_UINT8,  nullptr, 0 },
            { "TLM1_MYCALL",         "TLM1 callsign",               ATP_STR,    nullptr, 0 },
            { "TLM1_PATH",           "TLM1 path index",             ATP_UINT8,  nullptr, 0 },
            { "TLM1_DATA_INTERVAL",  "TLM1 data tx interval (sec)", ATP_UINT16, nullptr, 0 },
            { "TLM1_INFO_INTERVAL",  "TLM1 info tx interval (sec)", ATP_UINT16, nullptr, 0 },
            { "TLM1_BITS_ACTIVE",    "TLM1 BITS active mask",       ATP_UINT8,  nullptr, 0 },
            { "TLM1_COMMENT",        "TLM1 comment",                ATP_STR,    nullptr, 0 },
            { "TML1_DATA_CHANNEL<N>","TLM1 data channel N (N=0..12)",ATP_UINT8, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "Display") == 0) {
        static ATParam tbl[] = {
            { "OLED_ENABLE",     "OLED enable (0/1)",          ATP_BOOL,   nullptr, 0 },
            { "OLED_TIMEOUT",    "OLED timeout (sec)",         ATP_INT,    nullptr, 0 },
            { "DIM",             "Display dim level",          ATP_UINT8,  nullptr, 0 },
            { "CONTRAST",        "Display contrast",           ATP_UINT8,  nullptr, 0 },
            { "STARTUP",         "Startup display page",       ATP_UINT8,  nullptr, 0 },
            { "H_UP",            "Heading-up mode (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "TX_DISPLAY",      "Show TX packets (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "RX_DISPLAY",      "Show RX packets (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "DISPFILTER",      "Display packet filter",      ATP_UINT16, nullptr, 0 },
            { "DISPRF",          "Display RF packets (0/1)",   ATP_BOOL,   nullptr, 0 },
            { "DISPINET",        "Display INET packets (0/1)", ATP_BOOL,   nullptr, 0 },
            { "DISP_FLIP",       "Flip display (0/1)",         ATP_BOOL,   nullptr, 0 },
            { "DISP_BRIGHTNESS", "Display brightness",         ATP_UINT8,  nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "Modem/Audio") == 0) {
        static ATParam tbl[] = {
            { "AUDIO_HPF",      "Audio high-pass filter (0/1)",                 ATP_BOOL,   nullptr, 0 },
            { "AUDIO_LPF",      "Audio low-pass filter (0/1)",                  ATP_BOOL,   nullptr, 0 },
            { "PREAMBLE",       "TX preamble length",                           ATP_UINT8,  nullptr, 0 },
            { "MODEM_TYPE",     "Modem type",                                   ATP_UINT8,  nullptr, 0 },
            { "FX25_MODE",      "FX.25 FEC mode",                               ATP_UINT8,  nullptr, 0 },
            { "TX_TIMESLOT",    "TX CSMA time slot (ms)",                       ATP_UINT16, nullptr, 0 },
            { "TXTEST?",         "TX test state (DIS/MARK/SPACE/ALT)",  ATP_BOOL, nullptr, 0 },
            { "TXTEST=<MODE>",   "Set TX test mode (DIS/MARK/SPACE/ALT)", ATP_BOOL, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "Network/VPN/PPP") == 0) {
        static ATParam tbl[] = {
            { "VPN",                "WireGuard VPN enable (0/1)",   ATP_BOOL,   nullptr, 0 },
            { "MODEM",              "PPP modem enable (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "WG_PORT",            "WireGuard UDP port",           ATP_UINT16, nullptr, 0 },
            { "WG_PEER_ADDRESS",    "WireGuard peer address",       ATP_STR,    nullptr, 0 },
            { "WG_LOCAL_ADDRESS",   "WireGuard local IP",           ATP_STR,    nullptr, 0 },
            { "WG_NETMASK_ADDRESS", "WireGuard netmask",            ATP_STR,    nullptr, 0 },
            { "WG_GW_ADDRESS",      "WireGuard gateway",            ATP_STR,    nullptr, 0 },
            { "WG_PUBLIC_KEY",      "WireGuard peer public key",    ATP_STR,    nullptr, 0 },
            { "WG_PRIVATE_KEY",     "WireGuard local private key",  ATP_STR,    nullptr, 0 },
            { "HTTP_USERNAME",      "Web interface username",       ATP_STR,    nullptr, 0 },
            { "HTTP_PASSWORD",      "Web interface password",       ATP_STR,    nullptr, 0 },
            { "PPP_ENABLE",         "PPP enable (0/1)",             ATP_BOOL,   nullptr, 0 },
            { "PPP_APN",            "PPP APN",                      ATP_STR,    nullptr, 0 },
            { "PPP_PIN",            "PPP SIM PIN",                  ATP_STR,    nullptr, 0 },
            { "PPP_RST_GPIO",       "PPP modem reset GPIO",         ATP_INT8,   nullptr, 0 },
            { "PPP_TX_GPIO",        "PPP TX GPIO",                  ATP_INT8,   nullptr, 0 },
            { "PPP_RX_GPIO",        "PPP RX GPIO",                  ATP_INT8,   nullptr, 0 },
            { "PPP_RTS_GPIO",       "PPP RTS GPIO",                 ATP_INT8,   nullptr, 0 },
            { "PPP_CTS_GPIO",       "PPP CTS GPIO",                 ATP_INT8,   nullptr, 0 },
            { "PPP_DTR_GPIO",       "PPP DTR GPIO",                 ATP_INT8,   nullptr, 0 },
            { "PPP_RI_GPIO",        "PPP RI GPIO",                  ATP_INT8,   nullptr, 0 },
            { "PPP_RST_ACTIVE",     "PPP reset active level (0/1)", ATP_BOOL,   nullptr, 0 },
            { "PPP_RST_DELAY",      "PPP reset delay (ms)",         ATP_UINT16, nullptr, 0 },
            { "PPP_PWR_GPIO",       "PPP power GPIO",               ATP_INT8,   nullptr, 0 },
            { "PPP_PWR_ACTIVE",     "PPP power active level (0/1)", ATP_BOOL,   nullptr, 0 },
            { "PPP_SERIAL",         "PPP serial port index",        ATP_UINT8,  nullptr, 0 },
            { "PPP_MODEL",          "PPP modem model",              ATP_UINT8,  nullptr, 0 },
            { "PPP_FLOW_CTRL",      "PPP flow control",             ATP_UINT8,  nullptr, 0 },
            { "PPP_GNSS",           "PPP modem has GNSS (0/1)",     ATP_BOOL,   nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "GNSS") == 0) {
        static ATParam tbl[] = {
            { "GNSS_ENABLE",     "GNSS enable (0/1)",        ATP_BOOL,   nullptr, 0 },
            { "GNSS_PPS_GPIO",   "GNSS PPS GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "GNSS_CHANNEL",    "GNSS serial channel",      ATP_INT8,   nullptr, 0 },
            { "GNSS_TCP_PORT",   "GNSS TCP forwarding port", ATP_UINT16, nullptr, 0 },
            { "GNSS_TCP_HOST",   "GNSS TCP forwarding host", ATP_STR,    nullptr, 0 },
            { "GNSS_AT_COMMAND", "GNSS init AT command",     ATP_STR,    nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "HW I/O") == 0) {
        static ATParam tbl[] = {
            { "I2C_ENABLE",      "I2C bus 0 enable (0/1)",   ATP_BOOL,   nullptr, 0 },
            { "I2C_SDA_PIN",     "I2C bus 0 SDA pin",        ATP_INT8,   nullptr, 0 },
            { "I2C_SCK_PIN",     "I2C bus 0 SCK pin",        ATP_INT8,   nullptr, 0 },
            { "I2C_RST_PIN",     "I2C bus 0 reset pin",      ATP_INT8,   nullptr, 0 },
            { "I2C_FREQ",        "I2C bus 0 frequency (Hz)", ATP_UINT32, nullptr, 0 },
            { "I2C1_ENABLE",     "I2C bus 1 enable (0/1)",   ATP_BOOL,   nullptr, 0 },
            { "I2C1_SDA_PIN",    "I2C bus 1 SDA pin",        ATP_INT8,   nullptr, 0 },
            { "I2C1_SCK_PIN",    "I2C bus 1 SCK pin",        ATP_INT8,   nullptr, 0 },
            { "I2C1_FREQ",       "I2C bus 1 frequency (Hz)", ATP_UINT32, nullptr, 0 },
            { "ONEWIRE_ENABLE",  "OneWire enable (0/1)",     ATP_BOOL,   nullptr, 0 },
            { "ONEWIRE_GPIO",    "OneWire GPIO pin",         ATP_INT8,   nullptr, 0 },
            { "UART0_ENABLE",    "UART0 enable (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "UART0_TX_GPIO",   "UART0 TX GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "UART0_RX_GPIO",   "UART0 RX GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "UART0_RTS_GPIO",  "UART0 RTS GPIO pin",       ATP_INT8,   nullptr, 0 },
            { "UART1_ENABLE",    "UART1 enable (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "UART1_TX_GPIO",   "UART1 TX GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "UART1_RX_GPIO",   "UART1 RX GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "UART1_RTS_GPIO",  "UART1 RTS GPIO pin",       ATP_INT8,   nullptr, 0 },
            { "MODBUS_ENABLE",   "Modbus enable (0/1)",      ATP_BOOL,   nullptr, 0 },
            { "MODBUS_ADDRESS",  "Modbus device address",    ATP_UINT8,  nullptr, 0 },
            { "MODBUS_CHANNEL",  "Modbus serial channel",    ATP_INT8,   nullptr, 0 },
            { "MODBUS_DE_GPIO",  "Modbus DE/RE GPIO pin",    ATP_INT8,   nullptr, 0 },
            { "COUNTER0_ENABLE", "Counter0 enable (0/1)",    ATP_BOOL,   nullptr, 0 },
            { "COUNTER0_ACTIVE", "Counter0 active level",    ATP_BOOL,   nullptr, 0 },
            { "COUNTER0_GPIO",   "Counter0 GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "COUNTER1_ENABLE", "Counter1 enable (0/1)",    ATP_BOOL,   nullptr, 0 },
            { "COUNTER1_ACTIVE", "Counter1 active level",    ATP_BOOL,   nullptr, 0 },
            { "COUNTER1_GPIO",   "Counter1 GPIO pin",        ATP_INT8,   nullptr, 0 },
            { "EXT_TNC_ENABLE",  "Ext TNC enable (0/1)",     ATP_BOOL,   nullptr, 0 },
            { "EXT_TNC_CHANNEL", "Ext TNC serial channel",   ATP_INT8,   nullptr, 0 },
            { "EXT_TNC_MODE",    "Ext TNC mode",             ATP_INT8,   nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else if (strcmp(group, "Power") == 0) {
        static ATParam tbl[] = {
            { "PWR_EN",             "Power management enable (0/1)", ATP_BOOL,   nullptr, 0 },
            { "PWR_MODE",           "Power mode",                    ATP_UINT8,  nullptr, 0 },
            { "PWR_SLEEP_INTERVAL", "Sleep interval (sec)",          ATP_UINT16, nullptr, 0 },
            { "PWR_STANBY_DELAY",   "Standby delay (sec)",           ATP_UINT16, nullptr, 0 },
            { "PWR_SLEEP_ACTIVATE", "Sleep activate source",         ATP_UINT8,  nullptr, 0 },
            { "PWR_GPIO",           "Power GPIO pin",                ATP_INT8,   nullptr, 0 },
            { "PWR_ACTIVE",         "Power active level (0/1)",      ATP_BOOL,   nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
#ifdef MQTT
    else if (strcmp(group, "MQTT") == 0) {
        static ATParam tbl[] = {
            { "EN_MQTT",             "MQTT enable (0/1)",           ATP_BOOL,   nullptr, 0 },
            { "MQTT_HOST",           "MQTT broker hostname",        ATP_STR,    nullptr, 0 },
            { "MQTT_TOPIC",          "MQTT publish topic",          ATP_STR,    nullptr, 0 },
            { "MQTT_SUBSCRIBE",      "MQTT subscribe topic",        ATP_STR,    nullptr, 0 },
            { "MQTT_USER",           "MQTT username",               ATP_STR,    nullptr, 0 },
            { "MQTT_PASS",           "MQTT password",               ATP_STR,    nullptr, 0 },
            { "MQTT_PORT",           "MQTT broker port",            ATP_UINT16, nullptr, 0 },
            { "MQTT_TOPIC_FLAG",     "MQTT topic filter flags",     ATP_UINT16, nullptr, 0 },
            { "MQTT_SUBSCRIBE_FLAG", "MQTT subscribe filter flags", ATP_UINT16, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
#endif
    else if (strcmp(group, "Message") == 0) {
        static ATParam tbl[] = {
            { "MSG_ENABLE",   "Message enable (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "MSG_MYCALL",   "Message callsign",           ATP_STR,    nullptr, 0 },
            { "MSG_PATH",     "Message path index",         ATP_UINT8,  nullptr, 0 },
            { "MSG_RF",       "Message via RF (0/1)",       ATP_BOOL,   nullptr, 0 },
            { "MSG_INET",     "Message via internet (0/1)", ATP_BOOL,   nullptr, 0 },
            { "MSG_ENCRYPT",  "Message encryption (0/1)",   ATP_BOOL,   nullptr, 0 },
            { "MSG_KEY",      "Message encryption key",     ATP_STR,    nullptr, 0 },
            { "MSG_RETRY",    "Message retry count",        ATP_UINT8,  nullptr, 0 },
            { "MSG_INTERVAL", "Message retry interval (s)", ATP_UINT16, nullptr, 0 },
        };
        printGroup(tbl, sizeof(tbl)/sizeof(tbl[0]));
    }
    else {
        return "";  // Unknown group
    }

    return out;
}

// ---------------------------------------------------------------------------
// Main entry point
// ---------------------------------------------------------------------------

String handleATCommand(String cmd)
{
    cmd.trim();
    if (!cmd.startsWith("AT"))
        return "";
    if (cmd == "AT")
        return "OK";

    // Help - top-level only
    if (cmd == "AT?" || cmd == "AT+HELP")
        return buildTopLevelHelp();

    // Group help - must be checked before group param handlers
    {
        String groupHelp;
        if      (cmd == "AT+SYSTEM?")     groupHelp = buildGroupHelp("System");
        else if (cmd == "AT+WIFI_HELP?")  groupHelp = buildGroupHelp("WiFi");
        else if (cmd == "AT+BT?")         groupHelp = buildGroupHelp("Bluetooth");
        else if (cmd == "AT+RF?")         groupHelp = buildGroupHelp("RF");
        else if (cmd == "AT+IGATE?")      groupHelp = buildGroupHelp("IGATE");
        else if (cmd == "AT+DIGI?")       groupHelp = buildGroupHelp("DIGI");
        else if (cmd == "AT+TRK?")        groupHelp = buildGroupHelp("Tracker");
        else if (cmd == "AT+WX?")         groupHelp = buildGroupHelp("WX (Weather)");
        else if (cmd == "AT+TLM0?")       groupHelp = buildGroupHelp("TLM0 (Telemetry 0)");
        else if (cmd == "AT+TLM1?")       groupHelp = buildGroupHelp("TLM1 (Telemetry 1)");
        else if (cmd == "AT+DISPLAY?")    groupHelp = buildGroupHelp("Display");
        else if (cmd == "AT+MODEM?")      groupHelp = buildGroupHelp("Modem/Audio");
        else if (cmd == "AT+NETWORK?")    groupHelp = buildGroupHelp("Network/VPN/PPP");
        else if (cmd == "AT+GNSS?")       groupHelp = buildGroupHelp("GNSS");
        else if (cmd == "AT+HWIO?")       groupHelp = buildGroupHelp("HW I/O");
        else if (cmd == "AT+PWR?")        groupHelp = buildGroupHelp("Power");
#ifdef MQTT
        else if (cmd == "AT+MQTT?")       groupHelp = buildGroupHelp("MQTT");
#endif
        else if (cmd == "AT+MSG?")        groupHelp = buildGroupHelp("Message");
        if (groupHelp.length()) return groupHelp;
    }

    // Special action commands
    if (cmd == "AT+RESET" || cmd == "AT+RESTART") {
        log_d("CMD Reset System");
        delay(3000);
        esp_restart();
    }

    if (cmd == "AT+VERSION?") {
        String ver = "Firmware Version: " + String(VERSION);
        ver += ", SDK Version: " + String(ESP.getSdkVersion());
        ver += ", Core Version: " + String(ARDUINO_ESP32_RELEASE);
        return ver;
    }

    if (cmd == "AT+CHIPID?") {
        uint64_t chipid = ESP.getEfuseMac();
        String id = String((uint16_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
        id.toUpperCase();
        return id;
    }

    if (cmd == "AT+SAVECONFIG") {
        if (saveConfiguration("/default.cfg", config))
            return "Configuration Saved";
        else
            return "Failed to Save Configuration";
    }

    // Support both LOADCONFIG (legacy) and AT+LOADCONFIG
    if (cmd == "LOADCONFIG" || cmd == "AT+LOADCONFIG") {
        if (loadConfiguration("/default.cfg", config))
            return "Configuration Loaded";
        else
            return "Failed to Load Configuration";
    }

    if (cmd == "AT+WIFI_DISCONNECT") {
        WiFi.disconnect(true);
        return "WiFi Disconnected";
    }

    if (cmd == "AT+WIFI_CONNECT") {
        WiFi.reconnect();
        return "WiFi Reconnecting";
    }

    if (cmd == "AT+WIFI_STATUS?") {
        String status = "WiFi Status: ";
        status += (WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected";
        return status;
    }

    if (cmd == "AT+WIFI_SCAN") {
        int16_t n = WiFi.scanNetworks();
        String scanResult = "Scan Complete: " + String(n) + " networks found.\n";
        for (int i = 0; i < n; ++i) {
            scanResult += String(i + 1) + ": ";
            scanResult += WiFi.SSID(i);
            scanResult += ", RSSI: " + String(WiFi.RSSI(i)) + "dBm";
            scanResult += ", BSSID: " + WiFi.BSSIDstr(i);
            scanResult += ", Channel: " + String(WiFi.channel(i));
            scanResult += ", Encryption: " + String((int)WiFi.encryptionType(i)) + "\n";
        }
        WiFi.scanDelete();
        return scanResult;
    }

    if (cmd == "AT+WIFI?") {
        String mode;
        if (config.wifi_mode == WIFI_AP)          mode = "AP";
        else if (config.wifi_mode == WIFI_STA)     mode = "STA";
        else if (config.wifi_mode == WIFI_AP_STA)  mode = "AP+STA";
        else                                        mode = "OFF";
        mode = "Mode:" + mode + ",SSID:" + String(WiFi.SSID()) + ",RSSI:" + String(WiFi.RSSI()) + "dBm";
        return mode;
    }

    if (cmd == "AT+MODE?") {
        String mode = "MODE: ";
        if (config.igate_en) mode += "IGATE";
        if (config.digi_en)  mode += ",DIGI";
        if (config.trk_en)   mode += ",TRK";
        if (config.wx_en)    mode += ",WX";
        if (config.tlm0_en)  mode += ",TLM0";
        return mode;
    }

    // TX test commands
    if (cmd == "AT+TXTEST?") {
        switch (ModemTxTestGetState()) {
            case TEST_DISABLED:    return "DIS";
            case TEST_MARK:        return "MARK";
            case TEST_SPACE:       return "SPACE";
            case TEST_ALTERNATING: return "ALT";
        }
        return "DIS";
    }

    if (cmd.startsWith("AT+TXTEST=")) {
        String mode = cmd.substring(10);
        mode.toUpperCase();
        if (mode == "DIS") {
            ModemTxTestStop();
            return "OK";
        } else if (mode == "MARK") {
            ModemTxTestStart(TEST_MARK);
            return "OK";
        } else if (mode == "SPACE") {
            ModemTxTestStart(TEST_SPACE);
            return "OK";
        } else if (mode == "ALT") {
            ModemTxTestStart(TEST_ALTERNATING);
            return "OK";
        }
        return "ERR: mode must be DIS, MARK, SPACE, or ALT";
    }

    // Time commands
    if (cmd.startsWith("AT+TIME")) {
        String r = handleTimeCommand(cmd);
        if (r.length()) return r;
    }

    // Parameterized group handlers
    String result;

    result = handleSystemParams(cmd);  if (result.length()) return result;
    result = handleWifiParams(cmd);    if (result.length()) return result;
    result = handleBtParams(cmd);      if (result.length()) return result;
    result = handleRfParams(cmd);      if (result.length()) return result;
    result = handleIgateParams(cmd);   if (result.length()) return result;
    result = handleDigiParams(cmd);    if (result.length()) return result;
    result = handleTrackerParams(cmd); if (result.length()) return result;
    result = handleWxParams(cmd);      if (result.length()) return result;
    result = handleTlm0Params(cmd);    if (result.length()) return result;
    result = handleTlm1Params(cmd);    if (result.length()) return result;
    result = handleDisplayParams(cmd); if (result.length()) return result;
    result = handleModemParams(cmd);   if (result.length()) return result;
    result = handleNetworkParams(cmd); if (result.length()) return result;
    result = handleGnssParams(cmd);    if (result.length()) return result;
    result = handleHwIoParams(cmd);    if (result.length()) return result;
    result = handlePowerParams(cmd);   if (result.length()) return result;
    result = handleMqttParams(cmd);    if (result.length()) return result;
    result = handleMessageParams(cmd); if (result.length()) return result;

    return "ERR";
}
