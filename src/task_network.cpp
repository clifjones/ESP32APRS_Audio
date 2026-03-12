#include "main.h"
#include "webservice.h"   // config, aprsClient, webService, event_lastHeard
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include "ESP32Ping.h"
#include "wireguard_vpn.h"  // wireguard_active, wireguard_setup, wireguard_change_device, wireguard_remove
#include "pkg_list.h"       // pkgType, pkgListUpdate, pkgTxPush
#include "message.h"        // handleIncomingAPRS

#ifdef PPPOS
#include <PPP.h>
#endif

#ifdef MQTT
#include <PubSubClient.h>
#endif

#ifdef OLED
#include "cppQueue.h"
extern cppQueue dispBuffer;
#endif

// Globals defined in main.cpp
extern WiFiMulti wifiMulti;
extern IPAddress ap_ip, ap_mask, ap_leaseStart, ap_dns;
extern unsigned long pingTimeout;
extern unsigned long NTP_Timeout;
extern uint8_t APStationNum;
extern unsigned long waitISRetry;
extern unsigned long lastHeardTimeout;
extern bool lastHeard_Flag;
extern uint8_t wifiStatus;
extern bool vpnConnected;
extern unsigned long vpnTimeout;
extern unsigned long mitiWifiTimeout;
extern uint16_t wifiDisCount;
extern unsigned long timerNetwork, timerNetwork_old;
extern long timeNetwork;
extern unsigned long iGatetickInterval;
extern statusType status;
extern RTC_DATA_ATTR igateTLMType igateTLM;
extern time_t systemUptime;

#ifdef PPPOS
extern long int pppTimeout;
extern pppType pppStatus;
#endif

#ifdef MQTT
extern PubSubClient clientMQTT;
#endif

// Forward declarations for functions defined in main.cpp
void wifiConnection();
void onEvent(arduino_event_id_t event, arduino_event_info_t info);
bool APRSConnect();
void mqtt_reconnect();
#ifdef PPPOS
void PPPOS_Start();
#endif
#ifdef OLED
void pushTxDisp(uint8_t ch, const char *name, char *info);
#endif

void taskNetwork(void *pvParameters)
{
    int c = 0;
    // char raw[500];
    log_d("Task Network has been start");
    // manualWiFi = true;

    // Listen for modem events
    Network.onEvent(onEvent);

    // WiFi.onEvent(Wifi_connected,SYSTEM_EVENT_STA_CONNECTED);
    // WiFi.onEvent(Get_IPAddress, SYSTEM_EVENT_STA_GOT_IP);
    // WiFi.onEvent(Wifi_disconnected, SYSTEM_EVENT_STA_DISCONNECTED);

    if (config.wifi_mode == WIFI_STA_FIX)
    { /**< WiFi station mode */
        WiFi.mode(WIFI_MODE_STA);
        WiFi.setTxPower((wifi_power_t)config.wifi_power);
    }
    else if (config.wifi_mode == WIFI_AP_FIX)
    { /**< WiFi soft-AP mode */
        WiFi.mode(WIFI_MODE_AP);
        WiFi.setTxPower((wifi_power_t)config.wifi_power);
    }
    else if (config.wifi_mode == WIFI_AP_STA_FIX)
    { /**< WiFi station + soft-AP mode */
        WiFi.mode(WIFI_MODE_APSTA);
        WiFi.setTxPower((wifi_power_t)config.wifi_power);
    }
    else
    {
        WiFi.mode(WIFI_MODE_NULL);
    }

    if (config.wifi_mode & WIFI_STA_FIX)
    {
        // for (int i = 0; i < 5; i++)
        // {
        //     if (config.wifi_sta[i].enable)
        //     {
        //         wifiMulti.addAP(config.wifi_sta[i].wifi_ssid, config.wifi_sta[i].wifi_pass);
        //     }
        // }
        // WiFi.setHostname(config.host_name);
        // if (wifiMulti.run() == WL_CONNECTED)
        // {
        //     NTP_Timeout = millis() + 2000;
        // }
        wifiConnection();

        wifiMulti.setStrictMode(false); // Default is true.  Library will disconnect and forget currently connected AP if it's not in the AP list.
        wifiMulti.setAllowOpenAP(true); // Default is false.  True adds open APs to the AP list.
    }

    if (config.wifi_mode & WIFI_AP_FIX)
    {
        // manualWiFi = true;
        log_d("Access point running. IP address: ");
        log_d("%s", WiFi.softAPIP().toString().c_str());
        // Start the Access Point
        WiFi.AP.begin();
        WiFi.AP.config(ap_ip, ap_ip, ap_mask, ap_leaseStart, ap_dns);
        WiFi.AP.create(config.wifi_ap_ssid, config.wifi_ap_pass);
        if (!WiFi.AP.waitStatusBits(ESP_NETIF_STARTED_BIT, 1000))
        {
            Serial.println("Failed to start AP!");
            // return;
        }
    }

    pingTimeout = millis() + 10000;
    unsigned long timeNetworkOld = millis();
    timeNetwork = 0;

#ifdef PPPOS
    PPPOS_Start(); // Start PPP connection if enabled
    pppTimeout = millis() + (600 * 1000);
#endif

#ifdef PPPOS
    if (config.ppp_enable || (config.wifi_mode & WIFI_AP_STA_FIX))
#else
    if (config.wifi_mode & WIFI_AP_STA_FIX)
#endif
        webService();

    // wireguard_ctx_t ctx = {0};
    esp_err_t err;
    // #ifdef PPPOS
    // PPPOS_Start();    // Start PPP connection if enabled
    // #endif

// #ifdef BLUETOOTH
//     bluetooth_init(); // Initialize Bluetooth if enabled
// #endif

    for (;;)
    {
        unsigned long now = millis();
        timeNetwork = now - timeNetworkOld;
        timeNetworkOld = now;
        // wdtNetworkTimer = millis();
        // serviceHandle();
        // esp_task_wdt_reset();
        timerNetwork = micros() - timerNetwork_old;
        vTaskDelay(10 / portTICK_PERIOD_MS);
        timerNetwork_old = micros();

#ifdef PPPOS
        if (config.ppp_enable)
        {
            if (!PPP.connected())
            {
                if (millis() > pppTimeout)
                {
                    log_d("PPP connection timeout!");
                    PPPOS_Start(); // Restart PPP connection
                    pppTimeout = millis() + (600 * 1000);
                }
            }
        }

        if (WiFi.isConnected() == true || WiFi.softAPgetStationNum() > 0 || PPP.connected())
#else
        if (WiFi.isConnected() == true || WiFi.softAPgetStationNum() > 0)
#endif
        {
            if (lastHeard_Flag)
            {
                if (millis() > lastHeardTimeout)
                {
                    lastHeard_Flag = false;
                    lastHeardTimeout = millis() + 1000;
                    event_lastHeard(false);
                }
            }
        }

        if (config.wifi_mode & WIFI_AP_FIX)
        {
            APStationNum = WiFi.softAPgetStationNum();
            if (APStationNum > 0)
            {
// config.pwr_sleep_activate |= ACTIVATE_WIFI;
#ifdef PPPOS
                if ((WiFi.isConnected() == false) && (PPP.connected() == false))
#else
                if (WiFi.isConnected() == false)
#endif
                {
                    vTaskDelay(9 / portTICK_PERIOD_MS);
                    continue;
                }
            }
        }

        wifiStatus = WL_DISCONNECTED;
        if (config.wifi_mode & WIFI_STA_FIX)
        {
            if (WiFi.isConnected() == false)
            {
                if (millis() > mitiWifiTimeout)
                {
                    mitiWifiTimeout = millis() + 30000;
                    log_d("WiFi Check Connection!");
                    wifiStatus = wifiMulti.run();
                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                }
            }
            else
            {
                wifiStatus = WL_CONNECTED;
            }
        }
#ifdef PPPOS
        if ((wifiStatus == WL_CONNECTED) || (PPP.connected()))
#else
        if ((wifiStatus == WL_CONNECTED))
#endif
        {
            // config.pwr_sleep_activate |= ACTIVATE_WIFI;
            if (millis() > NTP_Timeout)
            {
                NTP_Timeout = millis() + 86400000;
                // setSyncProvider(getNtpTime);
                log_d("Contacting Time Server\n");
                configTime(3600 * config.timeZone, 0, config.ntp_host);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                struct tm tmstruct;
                if (getLocalTime(&tmstruct, 1000))
                {
                    time_t systemTime;
                    time(&systemTime);
                    setTime(systemTime);
                    if (systemUptime == 0)
                    {
                        systemUptime = time(NULL);
                    }
                    pingTimeout = millis() + 2000;
                    if (config.vpn)
                    {
                        // if (wireguard_up())
                        //     log_d("Wireguard UP Link");
                        // else
                        //     log_d("Wireguard Connect Fail!");
                        log_d("Setup Wireguard Setup!");
                        vpnTimeout = millis() + 10000;
                        // if (wireguard_active()) wireguard_remove();
                        if (!wireguard_active())
                        {
                            // if (wireguard_active()) wireguard_remove();
                            log_d("Setup Wireguard VPN!");
                            if (WiFi.isConnected())
                            {
                                wireguard_setup();
                                vpnConnected = true;
                            }
                            else
                            {
#ifdef PPPOS
                                if (PPP.connected())
                                {
                                    wireguard_setup();
                                    vpnConnected = true;
                                }
#endif
                            }
                        }
                    }
                }
                else
                {
                    NTP_Timeout = millis() + 5000;
                }
            }

            if (millis() > vpnTimeout && !vpnConnected && config.vpn)
            {
                vpnTimeout = millis() + 10000;
                log_d("RENEW Device Wireguard VPN!");
                wireguard_change_device();
                vpnConnected = true;
            }

            if (config.igate_en)
            {
                if (aprsClient.connected() == false)
                {
                    if (millis() > waitISRetry)
                    {
                        waitISRetry = millis() + 30000; // Retry connect 30Sec
                        if (APRSConnect())
                        {
                            if (config.igate_bcn)
                            {
                                iGatetickInterval = millis() + 10000; // send position after 10sec
                            }
                        }
                    }
                }
                else
                {
                    if (aprsClient.available())
                    {
                        pingTimeout = millis() + 300000;                // Reset ping timout
                        String line = aprsClient.readStringUntil('\n'); // อ่านค่าที่ Server ตอบหลับมาทีละบรรทัด
                        status.isCount++;
                        int start_val = line.indexOf(">", 0); // หาตำแหน่งแรกของ >
                        if (start_val > 3)
                        {
                            String src_call = line.substring(0, start_val);
                            String msg_call = "::" + src_call;

                            status.allCount++;
                            status.rxCount++;
                            igateTLM.RX++;

                            log_d("INET: %s\n", line.c_str());
                            start_val = line.indexOf(":", 10); // Search of info in ax25
                            if (start_val > 5)
                            {
                                String info = line.substring(start_val + 1);
                                size_t rawSize = line.length();
                                char *raw = (char *)calloc(rawSize + 1, sizeof(char));
                                if (raw)
                                {
                                    memset(raw, 0, rawSize + 1);
                                    memcpy(raw, info.c_str(), info.length());

                                    uint16_t type = pkgType(&raw[0]);
                                    if (type & FILTER_MESSAGE)
                                    {
                                        handleIncomingAPRS(line);
                                    }
                                    int start_dstssid = line.indexOf("-", 1); // get SSID -
                                    if (start_dstssid < 0)
                                        start_dstssid = line.indexOf(" ", 1); // get ssid space
                                    char ssid = 0;
                                    if (start_dstssid > 0)
                                        ssid = line.charAt(start_dstssid + 1);

                                    if (ssid > 47 && ssid < 58)
                                    {
                                        size_t len = src_call.length();
                                        char call[15];
                                        memset(call, 0, sizeof(call));
                                        if (len > 15)
                                            len = 15;
                                        memcpy(call, src_call.c_str(), len);
                                        call[14] = 0;
                                        memset(raw, 0, rawSize + 1);
                                        memcpy(raw, line.c_str(), line.length());
                                        if (type & config.dispFilter)
                                        {
                                            int idx = pkgListUpdate(call, raw, type, 1, 0);

#if defined OLED || defined ST7735_160x80
                                            if (idx > -1)
                                            {
                                                // Put queue affter filter for display popup
                                                if (config.rx_display && config.dispINET && (type & config.dispFilter))
                                                {
                                                    dispBuffer.push(line.c_str());
                                                    log_d("INET_putQueueDisp:[pkgList_idx=%d/queue=%d,Type=%d] %s\n", idx, dispBuffer.getCount(), type, call);
                                                }
                                            }
#endif
                                        }
                                        // INET2RF affter filter
                                        if (config.inet2rf)
                                        {
                                            if (type & config.inet2rfFilter)
                                            {
                                                String tnc2Raw = "";
                                                char *strtmp = (char *)calloc(350, sizeof(char));
                                                if (strtmp)
                                                {
                                                    if (config.aprs_ssid == 0)
                                                        sprintf(strtmp, "%s>APE32A", config.aprs_mycall);
                                                    else
                                                        sprintf(strtmp, "%s-%d>APE32A", config.aprs_mycall, config.aprs_ssid);
                                                    tnc2Raw = String(strtmp);
                                                    tnc2Raw += ",RFONLY"; // fix path to rf only not send loop to inet
                                                    tnc2Raw += ":}";      // 3rd-party frame
                                                    tnc2Raw += line;
                                                    pkgTxPush(tnc2Raw.c_str(), tnc2Raw.length(), 0, RF_CHANNEL);
                                                    char sts[50];
                                                    sprintf(sts, "--SRC CALL--\n%s\n", src_call.c_str());
#if defined OLED || defined ST7735_160x80
                                                    if (config.oled_enable)
                                                        pushTxDisp(TXCH_3PTY, "TX INET->RF", sts);
#endif
                                                    status.inet2rf++;
                                                    igateTLM.INET2RF++;
                                                    log_d("INET2RF: %s\n", line);
                                                    free(strtmp);
                                                }
                                                tnc2Raw.clear();
                                            }
                                        }
                                    }
                                    free(raw);
                                }
                                info.clear();
                            }
                            src_call.clear();
                            msg_call.clear();
                        }
                        line.clear();
                    }
                }
            }

#ifdef MQTT
            if (config.en_mqtt)
            {
                if (!clientMQTT.connected())
                {
                    mqtt_reconnect();
                }
                else
                {
                    clientMQTT.loop();
                }
            }
#endif

            if (millis() > pingTimeout)
            {
                pingTimeout = millis() + 600000;
                if (config.wifi_mode & WIFI_STA_FIX)
                {
                    log_d("Ping WiFi to %s\n", WiFi.gatewayIP().toString().c_str());
                    IPAddress wifiIP;
                    wifiIP.fromString(String(WiFi.gatewayIP().toString()));
                    if (ping_start(wifiIP, 2, 0, 0, 10) == true)
                    {
                        log_d("Ping WiFi Success!!\n");
                    }
                    else
                    {
                        log_d("Ping WiFi Fail!\n");
                        wifiConnection();
                        // WiFi.disconnect(true, true, 500);
                        // WiFi.persistent(false);
                        // WiFi.mode(WIFI_OFF); // Switch WiFi off

                        // wifiTTL = 0;
                        // delay(3000);
                        // if (config.wifi_mode == WIFI_STA_FIX)
                        // { /**< WiFi station mode */
                        //     WiFi.mode(WIFI_MODE_STA);
                        //     //WiFi.setTxPower((wifi_power_t)config.wifi_power);
                        // }
                        // else if (config.wifi_mode == WIFI_AP_FIX)
                        // { /**< WiFi soft-AP mode */
                        //     WiFi.mode(WIFI_MODE_AP);
                        //     //WiFi.setTxPower((wifi_power_t)config.wifi_power);
                        // }
                        // else if (config.wifi_mode == WIFI_AP_STA_FIX)
                        // { /**< WiFi station + soft-AP mode */
                        //     WiFi.mode(WIFI_MODE_APSTA);
                        //     //WiFi.setTxPower((wifi_power_t)config.wifi_power);
                        // }
                        // else
                        // {
                        //     WiFi.mode(WIFI_MODE_NULL);
                        // }
                        // wifiMulti.APlistClean(); // Clean AP list
                        // for (int i = 0; i < 5; i++)
                        // {
                        //     if (config.wifi_sta[i].enable)
                        //     {
                        //         wifiMulti.addAP(config.wifi_sta[i].wifi_ssid, config.wifi_sta[i].wifi_pass);
                        //     }
                        // }
                        // WiFi.setHostname(config.host_name);
                        // if (wifiMulti.run() == WL_CONNECTED)
                        // {
                        //     wifiDisCount=0;
                        //     pingTimeout = millis() + 60000;
                        //     //NTP_Timeout = millis() + 2000;
                        // }
                        // wifiMulti.run(5000,true); // Timeout 5 sec
                        // WiFi.reconnect();
                    }
                }
// if (config.vpn)
// {
//     if (!wireguard_active())
//     {
//         log_d("Reconnection Wireguard VPN!");
//         wireguard_remove();
//         delay(1000);
//         if(WiFi.isConnected()){
//                 wireguard_setup(NULL);
//             }else{
//                 if(PPP.connected()){
//                     wireguard_setup((netif *)PPP.netif());
//                 }
//             }
//     }
//     IPAddress vpnIP;
//     vpnIP.fromString(String(config.wg_local_address));
//     log_d("Ping VPN to %s", vpnIP.toString().c_str());
//     if (ping_start(vpnIP, 2, 0, 0, 10) == true)
//     {
//         log_d("VPN Ping Success!");
//     }
//     else
//     {
//         log_d("VPN Ping Fail!");
//         wireguard_remove();
//         delay(1000);
//         wireguard_setup(NULL);

//     }
// }
#ifdef PPPOS
                if (config.ppp_enable)
                {
                    if (PPP.connected())
                    {
                        // IPAddress pppIP(pppStatus.ip);
                        log_d("Ping PPP to %s", PPP.localIP().toString().c_str());
                        if ((PPP.linkUp() == true) && (ping_start(PPP.localIP(), 2, 0, 0, 30) == true))
                        {
                            log_d("PPP Ping Success!!");
                        }
                        else
                        {
                            log_d("PPP Ping Fail!");
                            PPPOS_Start();
                            pppTimeout = millis() + (600 * 1000);
                        }
                    }
                }
#endif
            }
        }
    } // for loop
}
