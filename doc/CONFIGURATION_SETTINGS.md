# Configuration Settings (Runtime)

This document lists all **runtime configuration settings** that alter ESP32APRS_Audio behavior. These are stored in `/default.cfg` on LITTLEFS (JSON) and loaded at boot. They can be changed via the web UI or by editing the config file.

## Storage

- **File:** `/default.cfg`
- **Format:** JSON
- **Load/Save:** `loadConfiguration()`, `saveConfiguration()` in `src/config.cpp`

---

## General / System

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `txTimeSlot` | `tx_timeslot` | uint16_t | 2000 | TX timeslot in ms; prevents collisions |
| `syncTime` | `synctime` | bool | true | Sync time from NTP |
| `timeZone` | `timeZone` | float | 7 | Timezone offset (hours) |
| `ntpHost` | `ntp_host` | string | "ntp.nakhonthai.net" | NTP server hostname |
| `hostName` | `host_name` | string | "ESP32APRS_Audio" | Device hostname for web UI title |
| `resetTimeout` | `reset_timeout` | uint16_t | 0 | Auto-reset interval in minutes; 0 = disabled |
| `logFile` | `log` | uint16_t | 0 | Logging bitmask (LOG_TRACKER, LOG_IGATE, etc.) |
| `httpUser` | `http_username` | string | "admin" | Web UI login |
| `httpPass` | `http_password` | string | "admin" | Web UI password |

---

## WiFi

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `WiFiMode` | `wifi_mode` | uint8_t | WIFI_AP_STA_FIX (3) | 0=Off, 1=AP, 2=STA, 3=AP+STA |
| `WiFiPwr` | `wifi_power` | int8_t | 44 | WiFi TX power (dBm) |
| `WiFiAPCH` | `wifi_ap_ch` | uint8_t | 6 | AP channel |
| `WiFiAP_SSID` | `wifi_ap_ssid` | string | "ESP32APRS_Audio" | AP SSID |
| `WiFiAP_PASS` | `wifi_ap_pass` | string | "aprsthnetwork" | AP password |
| `WiFiSTA` | `wifi_sta[]` | array | [enable, ssid, pass] ×5 | Up to 5 STA networks |

---

## Bluetooth (when `BLUETOOTH` defined)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| (BT fields in config struct) | `bt_slave`, `bt_master`, `bt_mode`, `bt_name`, etc. | — | — | BLE TNC2/KISS mode; not persisted in JSON in current config.cpp |

---

## RF / Modem

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `rfEnable` | `rf_en` | bool | false | Enable RF module (SA868/SR) |
| `rfType` | `rf_type` | uint8_t | RF_SA868_VHF | RF module type |
| `rfModem` | `modem_type` | uint8_t | 1 | 0=300, 1=1200, 2=1200v23, 3=GFSK9600 |
| `fx25Mode` | `fx25_mode` | uint8_t | 2 | 0=None, 1=RX, 2=RX+TX |
| `rfPreamble` | `preamble` | uint8_t | 3 | Preamble length |
| `rfFreqRX` | `freq_rx` | float | 144.39 | RX frequency (MHz) |
| `rfFreqTX` | `freq_tx` | float | 144.39 | TX frequency (MHz) |
| `rfToneRX` | `tone_rx` | int | 0 | RX CTCSS tone index |
| `rfToneTX` | `tone_tx` | int | 0 | TX CTCSS tone index |
| `rfSql` | `sql_level` | uint8_t | 1 | Squelch level |
| `rfVolume` | `volume` | uint8_t | 6 | Audio volume |
| `rfBand` | `band` | uint8_t | 0 | Band |
| `rfPwr` | `rf_power` | bool | false | RF power (LOW/HIGH) |
| `audioLPF` | `audio_lpf` | bool | false | Audio low-pass filter |
| `rfTx`, `rfRx`, `rfSQL`, `rfPD`, `rfPWR`, `rfPTT` | `rf_*_gpio` | int8_t | board-dependent | GPIO pins |
| `rfSQLAct`, `rfPDAct`, `rfPWRAct`, `rfPTTAct` | `rf_*_active` | bool | — | Active polarity |
| `adcAtten` | `adc_atten` | uint8_t | 0 | ADC attenuation |
| `adcOffset` | `adc_dc_offset` | uint16_t | 600 | ADC DC offset |
| `rfBaudrate` | `rf_baudrate` | ulong | 9600 | Serial baud to RF module |

---

## IGate

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `igateEn` | `igate_en` | bool | false | Enable IGate |
| `igateBcn` | `igate_bcn` | bool | false | Send beacon |
| `rf2inet` | `rf2inet` | bool | true | RF → Internet |
| `inet2rf` | `inet2rf` | bool | false | Internet → RF |
| `igatePos2rf` | `igate_loc2rf` | bool | false | Position to RF |
| `igatePos2inet` | `igate_loc2inet` | bool | true | Position to Internet |
| `rf2inetFilter` | `rf2inetFilter` | uint16_t | 0xFFF | Packet filter for RF→INET |
| `inet2rfFiltger` | `inet2rfFilter` | uint16_t | (see FILTER_*) | Packet filter for INET→RF |
| `igateSSID` | `aprs_ssid` | uint8_t | 1 | SSID (0–15) |
| `igatePort` | `aprs_port` | uint16_t | 14580 | APRS-IS port |
| `igateMycall` | `aprs_mycall` | string | "NOCALL" | Callsign |
| `igateHost` | `aprs_host` | string | "aprs.nakhonthai.net" | APRS-IS host |
| `igateFilter` | `aprs_filter` | string | "m/10" | APRS-IS filter |
| `igateGPS` | `igate_gps` | bool | false | Use GPS for position |
| `igateLAT`, `igateLON`, `igateALT` | `igate_lat`, `igate_lon`, `igate_alt` | float | 13.7555, 100.4930, 0 | Fixed position |
| `igateINV` | `igate_interval` | uint16_t | 600 | Beacon interval (sec) |
| `igateSymbol` | `igate_symbol` | string | "A&" | Table/symbol chars |
| `igateObject` | `igate_object` | string | "" | Object name (3–9 chars) |
| `igatePath` | `igate_path` | uint8_t | 8 | Path index |
| `igateComment` | `igate_comment` | string | "" | Comment |
| `igateSTSIntv` | `igate_sts_interval` | uint16_t | 1800 | Status interval (sec) |
| `igateStatus` | `igate_status` | string | (URL) | Status text |
| `igatePHG` | `igate_phg` | string | "" | Power/height/gain |
| `igateTlmAvg`, `igateTlmSen`, etc. | `igate_tlm_*` | arrays | — | Telemetry for IGate |
| `igateTlmInv` | `igate_tlm_interval` | uint8_t | 0 | IGate telemetry interval |

---

## Digi Repeater

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `digiEn` | `digi_en` | bool | false | Enable digipeater |
| `digiAuto` | `digi_auto` | bool | false | Auto path |
| `digiPos2rf` | `digi_loc2rf` | bool | true | Add position to RF |
| `digiPos2inet` | `digi_loc2inet` | bool | false | Add position to Internet |
| `digiTime` | `digi_timestamp` | bool | false | Add timestamp |
| `digiSSID` | `digi_ssid` | uint8_t | 3 | SSID |
| `digiMycall` | `digi_mycall` | string | "NOCALL" | Callsign |
| `digiPath` | `digi_path` | uint8_t | 8 | Path index |
| `digiDelay` | `digi_delay` | uint16_t | 0 | Digi delay (ms) |
| `digiFilter` | `digiFilter` | uint16_t | (FILTER_*) | Packet filter |
| `digiBcn` | `digi_bcn` | bool | — | Beacon |
| `digiGPS`, `digiLAT`, `digiLON`, `digiALT` | — | — | — | Position |
| `digiINV` | `digi_interval` | uint16_t | 600 | Beacon interval |
| `digiSymbol`, `digiPHG`, `digiComment`, `digiStatus` | — | — | — | Display/beacon |
| `digiTlmAvg`, etc. | `digi_tlm_*` | arrays | — | Telemetry |
| `digiTlmInv` | `digi_tlm_interval` | uint8_t | 0 | Digi telemetry interval |

---

## Tracker

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `trkEn` | `trk_en` | bool | false | Enable tracker |
| `trkPos2rf` | `trk_loc2rf` | bool | true | Position to RF |
| `trkPos2inet` | `trk_loc2inet` | bool | false | Position to Internet |
| `trkTime` | `trk_timestamp` | bool | false | Add timestamp |
| `trkSSID` | `trk_ssid` | uint8_t | 7 | SSID |
| `trkMycall` | `trk_mycall` | string | "NOCALL" | Callsign |
| `trkPath` | `trk_path` | uint8_t | 2 | Path index |
| `trkGPS` | `trk_gps` | bool | false | Use GPS |
| `trkLAT`, `trkLON`, `trkALT` | — | float | — | Fixed position |
| `trkINV` | `trk_interval` | uint16_t | 600 | Beacon interval (sec) |
| `trkSmart` | `trk_smartbeacon` | bool | true | Smart beacon |
| `trkCompress` | `trk_compress` | bool | true | Compressed position |
| `trkOptAlt` | `trk_altitude` | bool | true | Include altitude |
| `trkLog` | `trk_log` | bool | true | Logging |
| `trkOptRSSI` | `trk_rssi` | bool | false | Include RSSI |
| `trkLSpeed`, `trkHSpeed` | `trk_lspeed`, `trk_hspeed` | uint8_t | 5, 120 | Smart beacon speeds |
| `trkMaxInv`, `trkMinInv` | `trk_maxinterval`, `trk_mininterval` | uint8_t | 30, 5 | Smart beacon intervals |
| `trkMinDir` | `trk_minangle` | uint8_t | 25 | Min angle (deg) |
| `trkSlowInv` | `trk_slowinterval` | uint16_t | 600 | Slow interval |
| `trkSymbol`, `trkSymbolMove`, `trkSymbolStop` | — | string | — | Symbols |
| `trkItem` | `trk_item` | string | "" | Item name |
| `trkComment`, `trkStatus` | — | string | — | Comment/status |
| `trkMicEType` | `trk_mice_type` | uint8_t | 7 | MIC-E type |
| `trkTlmInv` | `trk_tlm_interval` | uint8_t | 0 | Tracker telemetry interval |

---

## Weather (WX)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `wxEn` | `wx_en` | bool | false | Enable WX |
| `wxTx2rf` | `wx_2rf` | bool | true | WX to RF |
| `wxTx2inet` | `wx_2inet` | bool | true | WX to Internet |
| `wxTime` | `wx_timestamp` | bool | false | Add timestamp |
| `wxSSID` | `wx_ssid` | uint8_t | 13 | SSID |
| `wxMycall` | `wx_mycall` | string | "NOCALL" | Callsign |
| `wxPath` | `wx_path` | uint8_t | 8 | Path index |
| `wxGPS` | `wx_gps` | bool | false | Use GPS |
| `wxLAT`, `wxLON`, `wxALT` | — | float | — | Fixed position |
| `wxInv` | `wx_interval` | uint16_t | 600 | Report interval |
| `wxFlage` | `wx_flage` | uint32_t | 0 | Sensor flags |
| `wxObject` | `wx_object` | string | "" | Object name |
| `wxComment` | `wx_comment` | string | "WX MODE" | Comment |
| `wxSenEn`, `wxSenAvg`, `wxSenCH` | arrays | — | — | Per-sensor enable/avg/channel |

---

## Telemetry (TLM)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `tlmEn` | `tlm0_en` | bool | false | Enable telemetry |
| `tlmTx2rf` | `tlm0_2rf` | bool | true | TLM to RF |
| `tlmTx2inet` | `tlm0_2inet` | bool | true | TLM to Internet |
| `tlmSSID` | `tlm0_ssid` | uint8_t | 0 | SSID |
| `tlmMycall` | `tlm0_mycall` | string | "NOCALL" | Callsign |
| `tlmPath` | `tlm0_path` | uint8_t | 0 | Path index |
| `tlmInfoInv` | `tlm0_info_interval` | uint16_t | 3600 | Info interval (sec) |
| `tlmDataInv` | `tlm0_data_interval` | uint16_t | 600 | Data interval (sec) |
| `tlmBIT` | `tlm0_BITS_Active` | uint8_t | 0xFF | BITS flags |
| `tlmEQNS`, `tlmPARM`, `tlmUNIT`, `tlmDataCH` | arrays | — | — | EQNS, params, units, channels |

---

## Display (OLED)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `dspEn` | `oled_enable` | bool | true (if OLED) | Enable display |
| `dspTOut` | `oled_timeout` | int | 60 | Screen timeout (sec) |
| `dspDim` | `dim` | uint8_t | 0 | Dim level |
| `dspContrast` | `contrast` | uint8_t | 0 | Contrast |
| `dspBright` | `disp_brightness` | uint8_t | 250 | Brightness (0–255) |
| `dspStartUp` | `startup` | uint8_t | 0 | Startup screen |
| `dspDelay` | `dispDelay` | uint | 3 | Popup display duration (sec) |
| `dspDxFilter` | `filterDistant` | uint | 0 | DX filter |
| `dspHUp` | `h_up` | bool | true | Header orientation |
| `dspTX` | `tx_display` | bool | true | Show TX on display |
| `dspRX` | `rx_display` | bool | true | Show RX on display |
| `dspFilter` | `dispFilter` | uint16_t | (FILTER_*) | Display packet filter |
| `dspRF` | `dispRF` | bool | true | Show RF |
| `dspINET` | `dispINET` | bool | false | Show Internet |
| `dspFlip` | `disp_flip` | bool | false | Flip display |

---

## Path Presets

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `path` | `path[4]` | array of strings | WIDE1-1, WIDE1-1,WIDE2-1, TRACK3-3, RS0ISS | Path presets |

---

## VPN (WireGuard)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `vpnEn` | `vpn` | bool | false | Enable WireGuard |
| `vpnPort` | `wg_port` | uint16_t | 51820 | VPN port |
| `vpnPeer` | `wg_peer_address` | string | "vpn.nakhonthai.net" | Peer address |
| `vpnLocal` | `wg_local_address` | string | "192.168.1.2" | Local address |
| `vpnNetmark` | `wg_netmask_address` | string | "255.255.255.0" | Netmask |
| `vpnGW` | `wg_gw_address` | string | "192.168.1.1" | Gateway |
| `vpnPubKey` | `wg_public_key` | string | "" | Peer public key |
| `vpnPriKey` | `wg_private_key` | string | "" | Private key |

---

## GNSS

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `gnssEn` | `gnss_enable` | bool | false | Enable GNSS |
| `gnssCH` | `gnss_channel` | int8_t | 0 | Channel (0=NONE, 1=UART0, etc.) |
| `gnssPPS` | `gnss_pps_gpio` | int8_t | -1 | PPS GPIO |
| `gnssTCPPort` | `gnss_tcp_port` | uint16_t | 8080 | TCP port for NMEA |
| `gnssTCPHost` | `gnss_tcp_host` | string | "192.168.0.1" | TCP host |
| `gnssAT` | `gnss_at_command` | string | "" | AT command for GNSS |

---

## I2C

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `i2cEn` | `i2c_enable` | bool | board-dependent | Enable I2C bus 0 |
| `i2cSDA`, `i2cSCK` | `i2c_sda_pin`, `i2c_sck_pin` | int8_t | board-dependent | GPIO pins |
| `i2cFreq` | `i2c_freq` | uint32_t | 400000 | Frequency (Hz) |
| `i2c1En`, `i2c1SDA`, `i2c1SCK`, `i2c1Freq` | — | — | — | Second I2C bus |

---

## 1-Wire

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `oneWireEn` | `onewire_enable` | bool | false | Enable 1-Wire |
| `oneWireIO` | `onewire_gpio` | int8_t | -1 | GPIO pin |

---

## UART

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `uart0En`, `uart0BR`, `uart0TX`, `uart0RX`, `uart0RTS` | — | — | board-dependent | UART0 |
| `uart1En`, `uart1BR`, `uart1TX`, `uart1RX`, `uart1RTS` | — | — | board-dependent | UART1 |
| `uart2En`, `uart2BR`, `uart2TX`, `uart2RX`, `uart2RTS` | — | — | — | UART2 (if SOC_UART_NUM>2) |

---

## Modbus

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `modbusEn` | `modbus_enable` | bool | false | Enable Modbus |
| `modbusAddr` | `modbus_address` | uint8_t | 0 | Slave address |
| `modbusCh` | `modbus_channel` | int8_t | 0 | UART channel |
| `modbusDE` | `modbus_de_gpio` | int8_t | -1 | DE/RTS GPIO |

---

## Counters

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `cnt0En`, `cnt0Act`, `cnt0IO` | — | — | — | Counter 0 |
| `cnt1En`, `cnt1Act`, `cnt1IO` | — | — | — | Counter 1 |

---

## External TNC

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `extTNCEn` | `ext_tnc_enable` | bool | false | Enable external TNC |
| `extTNCCh` | `ext_tnc_channel` | int8_t | 0 | UART channel |
| `extTNCMode` | `ext_tnc_mode` | int8_t | 2 | 0=None, 1=KISS, 2=TNC2, 3=YAESU |

---

## Power

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `pwrEn` | `pwr_en` | bool | false | Enable power control |
| `pwrMode` | `pwr_mode` | uint8_t | MODE_A | A=Continue, B=Wait, C=Send+Sleep |
| `pwrSleep` | `pwr_sleep_interval` | uint16_t | 600 | Sleep interval (sec) |
| `pwrStanby` | `pwr_stanby_delay` | uint16_t | 300 | Standby delay (sec) |
| `pwrSleepAct` | `pwr_sleep_activate` | uint8_t | ACTIVATE_* | What activates sleep |
| `pwrIO` | `pwr_gpio` | int8_t | -1 | Power control GPIO |
| `pwrIOAct` | `pwr_active` | bool | 1 | Active polarity |

---

## Sensors

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `Sensor` | `sensor[]` | array | [enable, port, address, samplerate, averagerate, eqns[3], type, parm, unit] × SENSOR_NUMBER | I2C/Modbus sensors (temp, humidity, PM2.5, etc.) |

---

## PPP Modem (when `PPPOS` defined)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `pppEn` | `ppp_enable` | bool | false | Enable PPP modem |
| `pppAPN` | `ppp_apn` | string | "internet" | APN |
| `pppPin` | `ppp_pin` | string | "0000" | SIM PIN |
| `pppRST`, `pppRSTAct`, `pppRSTDelay` | — | — | — | Reset GPIO |
| `pppTX`, `pppRX`, `pppRTS`, `pppDTR`, `pppCTS`, `pppRI` | — | — | — | Modem GPIOs |
| `pppPWR`, `pppPWRAct` | — | — | — | Power GPIO |
| `pppSerial`, `pppSerialBaudrate` | — | — | — | Serial port |
| `pppModel`, `pppFlow` | — | — | — | Modem model, flow control |
| `pppGNSS`, `pppNAPT` | — | — | — | GNSS, NAPT |

---

## MQTT (when `MQTT` defined)

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `mqttEnable` | `en_mqtt` | bool | false | Enable MQTT |
| `mqttHost` | `mqtt_host` | string | "mqtt.nakhonthai.net" | Broker host |
| `mqttTopic` | `mqtt_topic` | string | "/{chipid}/TX" | Publish topic |
| `mqttSub` | `mqtt_subscribe` | string | "/{chipid}/RX" | Subscribe topic |
| `mqttTopicFlag`, `mqttSubFlag` | — | uint16_t | — | Topic flags |
| `mqttPort` | `mqtt_port` | uint16_t | 1883 | Broker port |
| `mqttUser`, `mqttPass` | — | string | — | Credentials |

---

## AT Command / System

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `cmdOnMqtt` | `at_cmd_mqtt` | bool | — | AT commands over MQTT |
| `cmdOnMsg` | `at_cmd_msg` | bool | — | AT commands over APRS messages |
| `cmdOnBluetooth` | `at_cmd_bluetooth` | bool | — | AT commands over Bluetooth |
| `cmdOnUart` | `at_cmd_uart` | uint8_t | — | AT commands on UART (0–4) |

---

## Message

| JSON Key | Config Field | Type | Default | Effect |
|----------|--------------|------|---------|--------|
| `msgEnable` | `msg_enable` | bool | true | Enable APRS messaging |
| `msgMycall` | `msg_mycall` | string | "NOCALL" | Callsign for messages |
| `msgPath` | `msg_path` | uint8_t | 9 | Path index |
| `msgRf` | `msg_rf` | bool | true | Messages via RF |
| `msgInet` | `msg_inet` | bool | true | Messages via Internet |
| `msgEncrypt` | `msg_encrypt` | bool | false | AES encryption |
| `msgAESKey` | `msg_key` | string | (32-char hex) | AES key |
| `msgRetry` | `msg_retry` | uint8_t | 3 | Retry count |
| `msgInterval` | `msg_interval` | uint16_t | 30 | Retry interval (sec) |

---

## Filter Constants (for `*Filter` fields)

`include/main.h` defines:

- `FILTER_OBJECT`, `FILTER_ITEM`, `FILTER_MESSAGE`, `FILTER_WX`, `FILTER_TELEMETRY`

- `FILTER_QUERY`, `FILTER_STATUS`, `FILTER_POSITION`, `FILTER_BUOY`, `FILTER_MICE`, `FILTER_THIRDPARTY`

Combine with bitwise OR to build filter masks.

---

## How to Change

1. **Web UI:** Connect to the device via WiFi and use the configuration pages.
2. **File:** Mount LITTLEFS, edit `/default.cfg` (JSON), then reboot.
3. **AT commands:** When enabled, use `AT+...` over serial/MQTT/Bluetooth (see `handleATCommand.cpp`).
