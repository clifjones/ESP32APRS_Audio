#include "main.h"
#include "webservice.h"   // config, aprsClient, handle_ws, initInterval (extern)
#include <LibAPRSesp.h>   // APRS_setCallsign, Ax25*, ax25_decode, packet2Raw
#include <AFSK.h>         // DAC_TimerEnable, adcEn, dacEn
#include <KISS.h>         // kiss_wrapper
#include "digirepeater.h" // digiProcess
#include "igate.h"        // igateProcess
#include "message.h"      // sendAPRSMessageRetry, handleIncomingAPRS
#include "sensor.h"       // sen[]
#include "pkg_list.h"     // pkgListUpdate, pkgType, pkgTxPush

#ifdef MQTT
#include <PubSubClient.h>
#endif

#ifdef OLED
#include "cppQueue.h"
extern cppQueue dispBuffer;
#endif

// Globals defined in main.cpp shared with other tasks/files
extern unsigned long timerAPRS, timerAPRS_old;
extern statusType status;
extern RTC_DATA_ATTR igateTLMType igateTLM;
extern RTC_DATA_ATTR dataTLMType systemTLM;
extern AX25Msg incomingPacket;
extern bool lastPkg;
extern SemaphoreHandle_t gpsMutex;
extern char EVENT_TX_POSITION;
extern unsigned char SB_SPEED, SB_SPEED_OLD;
extern int16_t SB_HEADING;
extern uint16_t tx_interval;
extern unsigned int tx_counter;
extern int16_t last_heading;
extern uint8_t Sleep_Activate;
extern unsigned long StandByTick;
extern long sendTimer;
extern bool AFSKInitAct;
extern RTC_NOINIT_ATTR uint16_t TLM_SEQ;
extern RTC_NOINIT_ATTR uint16_t IGATE_TLM_SEQ;
extern RTC_NOINIT_ATTR uint16_t DIGI_TLM_SEQ;
extern unsigned long iGatetickInterval; // also written by taskNetwork
extern long timeAprs;
extern int8_t adcEn;
extern int8_t dacEn;

#ifdef MQTT
extern PubSubClient clientMQTT;
#endif

// Forward declarations for functions defined in main.cpp
void telemetry_base91(char *cdata, char *output, size_t outputsize);
void logTracker(double lat, double lon, double speed, double course);
void logIGate(double lat, double lon, double speed, double course);
void logDigi(double lat, double lon, double speed, double course);
void logWeather(double lat, double lon, double speed, double course);
bool pkgTxDuplicate(AX25Msg ax25);
bool pkgTxSend();
String trk_gps_postion(String comment);
String trk_fix_position(String comment);
String igate_position(double lat, double lon, double alt, String comment);
String digi_position(double lat, double lon, double alt, String comment);
void tracker_status(char *text);
void igate_status(char *text);
void digi_status(char *text);
void sendTelemetry_0(char *raw, bool header);
void sendTelemetry_trk(char *raw);
void sendTelemetry_igate(char *raw);
void sendTelemetry_digi(char *raw);
void getTelemetry_0();
String wx_report(double lat, double lon, double alt, String comment);
void smartbeacon(void);
#ifdef OLED
void pushTxDisp(uint8_t ch, const char *name, char *info);
#endif

// Variables exclusively used by taskAPRS (moved from main.cpp)
long timeSlot;
unsigned long WxInterval;
unsigned long WxIntervalAvg = 0;
int trkTlmInvCount = 0;
int igateTlmInvCount = 0;
int digiTlmInvCount = 0;
unsigned long msgInterval = 0;

void taskAPRS(void *pvParameters)
{
    //	long start, stop;
    char sts[50];
    unsigned long tickInterval = 0;
    unsigned long DiGiInterval = 0;

    unsigned long igateSTSInterval = 0;
    unsigned long digiSTSInterval = 0;
    unsigned long trkSTSInterval = 0;

    uint16_t type = 0;
    bool newIGatePkg = false;
    bool newDigiPkg = false;
    uint8_t *buf;
    uint16_t size = 0;
    int8_t peak = 0;
    int8_t valley = 0;
    uint8_t signalLevel = 0;
    uint8_t fixed = 0;
    uint16_t mV = 0;

    // PacketBuffer.clean();
    adcEn = 0;
    dacEn = 0;

    APRS_setCallsign(config.aprs_mycall, config.aprs_ssid);
    sendTimer = millis() - (config.igate_interval * 1000) + 30000;
    igateTLM.TeleTimeout = millis() + 60000; // 1Min

    msgInterval = millis() + 30000;
    timeSlot = millis();
    timeAprs = 0;
    tx_interval = config.trk_interval;
    tx_counter = tx_interval - 10;

    initInterval = true;
    AFSKInitAct = true;
    log_d("Task APRS has been start");
    for (;;)
    {

        // if (adcEn == 1)
        // {
        //     AFSK_TimerEnable(true);
        //     adcEn = 0;
        // }
        // else if (adcEn == -1)
        // {
        //     AFSK_TimerEnable(false);
        //     adcEn = 0;
        // }

        if (dacEn == 1)
        {
            DAC_TimerEnable(true);
            dacEn = 0;
        }
        else if (dacEn == -1)
        {
            DAC_TimerEnable(false);
            dacEn = 0;
        }
        long now = millis();
        // wdtSensorTimer = now;
        // time_t timeStamp;
        // time(&timeStamp);
        if (initInterval)
        {
            tickInterval = WxInterval = DiGiInterval = igateSTSInterval = iGatetickInterval = digiSTSInterval = trkSTSInterval = millis() + 10000;
            systemTLM.ParmTimeout = millis() + 20000;
            systemTLM.TeleTimeout = millis() + 30000;
            initInterval = false;
            tx_interval = config.trk_interval;
            tx_counter = tx_interval - 10;
        }
        timerAPRS = micros() - timerAPRS_old;
        vTaskDelay(10 / portTICK_PERIOD_MS);
        timerAPRS_old = micros();

        if (now > msgInterval)
        {
            msgInterval = millis() + config.msg_interval;
            sendAPRSMessageRetry();
        }

// #ifdef BLUETOOTH
// #if !defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32C6)
//         if (NuSerial.isConnected())
//         {
//             if (NuSerial.available())
//             {
//                 log_d("Bluetooth RX Data: %d Byte", NuSerial.available());
//                 if (config.bt_mode == 1)
//                 { // TNC2RAW MODE
//                     String rxValue = NuSerial.readString();
//                     uint8_t SendMode = TNC_CHANNEL;
//                     if (config.igate_loc2rf)
//                         SendMode |= RF_CHANNEL;
//                     if (config.igate_loc2inet)
//                         SendMode |= INET_CHANNEL;
//                     pkgTxPush(rxValue.c_str(), rxValue.length(), 1, SendMode);
//                 }
//                 else if (config.bt_mode == 2)
//                 {
//                     // KISS MODE
//                     size_t num = NuSerial.available();
//                     for (int i = 0; i < num; i++)
//                     {
//                         kiss_serial((uint8_t)NuSerial.read());
//                     }
//                 }
//                 else if (config.bt_mode == 3)
//                 { // AT COMMAND
//                     String cmd = NuSerial.readStringUntil('\n');
//                     cmd.trim();
//                     String ret = handleATCommand(String((char *)cmd.c_str()));
//                     if (ret != "")
//                         NuSerial.println(ret);
//                     log_d("AT-Command response: %s", ret.c_str());
//                 }
//             }
//         }
// #else
//         if (SerialBT.available())
//         {
//             log_d("Bluetooth RX Data: %d Byte", SerialBT.available());
//             if (config.bt_mode == 1)
//             { // TNC2RAW MODE
//                 String rxValue = SerialBT.readString();
//                 uint8_t SendMode = TNC_CHANNEL;
//                 if (config.igate_loc2rf)
//                     SendMode |= RF_CHANNEL;
//                 if (config.igate_loc2inet)
//                     SendMode |= INET_CHANNEL;
//                 pkgTxPush(rxValue.c_str(), rxValue.length(), 1, SendMode);
//             }
//             else if (config.bt_mode == 2)
//             {
//                 // KISS MODE
//                 size_t num = SerialBT.available();
//                 for (int i = 0; i < num; i++)
//                 {
//                     kiss_serial((uint8_t)SerialBT.read());
//                 }
//             }
//             else if (config.bt_mode == 3)
//             { // AT COMMAND
//                 String cmd = SerialBT.readStringUntil('\n');
//                 cmd.trim();
//                 String ret = handleATCommand(String((char *)cmd.c_str()));
//                 if (ret != "")
//                     SerialBT.println(ret);
//                 log_d("AT-Command response: %s", ret.c_str());
//             }
//         }
// #endif
// #endif

        // SEND RF in time slot
        // if (now > timeSlot)
        // {
        // Transmit in timeslot if enabled
        pkgTxSend();
        //     timeSlot = millis() + 100;
        // }
        Ax25TransmitBuffer(); // transmit buffer (will return if nothing to be transmitted)
        Ax25TransmitCheck();  // check for pending transmission request

        if (config.trk_en)
        { // TRACKER MODE
            if (config.trk_sts_interval > 10)
            {
                if (millis() > trkSTSInterval)
                {
                    trkSTSInterval = millis() + (config.trk_sts_interval * 1000);
                    tracker_status(config.trk_status);
                }
            }
            if (millis() > tickInterval)
            {
                tickInterval = millis() + 1000;

                tx_counter++;
                // log_d("TRACKER tx_counter=%d\t INTERVAL=%d\n", tx_counter, tx_interval);
                //   Check interval timeout
                // Snapshot GPS quality + SmartBeacon inputs under a single mutex window.
                uint32_t sbSats;
                float sbHdop, sbSpeedKmph, sbCourseDeg;
                xSemaphoreTake(gpsMutex, portMAX_DELAY);
                sbSats      = gps.satellites.value();
                sbHdop      = gps.hdop.hdop();
                sbSpeedKmph = (float)gps.speed.kmph();
                sbCourseDeg = (float)gps.course.deg();
                xSemaphoreGive(gpsMutex);

                if (config.trk_smartbeacon && config.trk_gps)
                {
                    if ((sbSats > 3) && (sbHdop < 10))
                    {
                        if (tx_counter > tx_interval)
                        {
                            if (tx_counter > config.trk_mininterval)
                                EVENT_TX_POSITION = 4;
                        }
                        else
                        {
                            if (tx_counter >= (tx_interval + 5))
                            {
                                EVENT_TX_POSITION = 5;
                            }
                        }
                    }
                }
                else if (tx_counter > tx_interval)
                {
                    // if (tx_counter > config.trk_slowinterval)
                    //{
                    EVENT_TX_POSITION = 6;
                    tx_interval = config.trk_interval;
                    //}
                }

                // if (config.trk_gps && gps.speed.isValid() && gps.location.isValid() && gps.course.isValid() && (gps.hdop.hdop() < 10.0) && (gps.satellites.value() > 3))
                // if (config.trk_gps && gps.speed.isValid() && gps.location.isValid() && gps.course.isValid())
                if (config.trk_gps)
                {
                    SB_SPEED_OLD = SB_SPEED;
                    if (sbSats > 3 && sbHdop < 10)
                    {
                        SB_SPEED = (unsigned char)sbSpeedKmph;
                        if (sbSpeedKmph > config.trk_lspeed)
                            SB_HEADING = (int16_t)sbCourseDeg;
                    }
                    else
                    {
                        if (SB_SPEED > 0)
                            SB_SPEED--;
                    }
                    if (config.trk_smartbeacon) // SMART BEACON CAL
                    {
                        if (SB_SPEED < config.trk_lspeed && SB_SPEED_OLD > config.trk_lspeed) // Speed slow down to STOP
                        {                                                                     // STOPING
                            SB_SPEED_OLD = 0;
                            if (tx_counter > config.trk_mininterval)
                            {
                                EVENT_TX_POSITION = 7;
                                tx_interval = config.trk_slowinterval;
                            }
                        }
                        else
                        {
                            smartbeacon();
                        }
                    }
                    else if (tx_counter > tx_interval)
                    { // send gps location
                        if (gps.location.isValid() && gps.hdop.hdop() < 10)
                        {
                            EVENT_TX_POSITION = 8;
                            tx_interval = config.trk_interval;
                        }
                    }
                }
            }

            if (EVENT_TX_POSITION > 0)
            {
                String rawData;
                String cmn = "";
                Sleep_Activate &= ~ACTIVATE_TRACKER;
                StandByTick = millis() + (5000);
                if (config.trk_tlm_interval > 0)
                {
                    trkTlmInvCount++;
                    if (trkTlmInvCount >= config.trk_tlm_interval)
                    {
                        trkTlmInvCount = 0;
                        if (config.trk_tlm_sensor[0] | config.trk_tlm_sensor[1] | config.trk_tlm_sensor[2] | config.trk_tlm_sensor[3] | config.trk_tlm_sensor[4])
                        {
                            char tlm_result[100];
                            char tlm_data[200];
                            size_t tlm_sz;
                            if ((TLM_SEQ % 100) == 0)
                            {
                                char rawInfo[256];
                                char name[10];
                                sprintf(rawInfo, "PARM.");
                                int i, c = 0;
                                for (i = 0; i < 5; i++)
                                {
                                    if (config.trk_tlm_sensor[i] == 0)
                                    {
                                        c++;
                                        continue;
                                    }
                                    else
                                    {
                                        if (i > 0)
                                            strcat(rawInfo, ",");
                                        sprintf(name, "%s", config.trk_tlm_PARM[i]);
                                        strcat(rawInfo, name);
                                    }
                                }
                                for (int n = c + 8; n > 0; n--)
                                {
                                    strcat(rawInfo, ",");
                                }
                                sendTelemetry_trk(rawInfo);
                                memset(rawInfo, 0, sizeof(rawInfo));
                                sprintf(rawInfo, "UNIT.");
                                c = 0;
                                for (i = 0; i < 5; i++)
                                {
                                    if (config.trk_tlm_sensor[i] == 0)
                                    {
                                        c++;
                                        continue;
                                    }
                                    else
                                    {
                                        if (i > 0)
                                            strcat(rawInfo, ",");
                                        sprintf(name, "%s", config.trk_tlm_UNIT[i]);
                                        strcat(rawInfo, name);
                                    }
                                }
                                for (int n = c + 8; n > 0; n--)
                                {
                                    strcat(rawInfo, ",");
                                }
                                sendTelemetry_trk(rawInfo);
                                memset(rawInfo, 0, sizeof(rawInfo));
                                sprintf(rawInfo, "EQNS.");
                                c = 0;
                                for (i = 0; i < 5; i++)
                                {
                                    if (config.trk_tlm_sensor[i] == 0)
                                    {
                                        c++;
                                        continue;
                                    }
                                    else
                                    {
                                        if (i > 0)
                                            strcat(rawInfo, ",");
                                        if (fmod(config.trk_tlm_EQNS[i][0], 1) == 0)
                                            sprintf(name, "%0.f", config.trk_tlm_EQNS[i][0]);
                                        else
                                            sprintf(name, "%.3f", config.trk_tlm_EQNS[i][0]);
                                        strcat(rawInfo, name);
                                        if (fmod(config.trk_tlm_EQNS[i][1], 1) == 0)
                                            sprintf(name, ",%0.f", config.trk_tlm_EQNS[i][1]);
                                        else
                                            sprintf(name, ",%.3f", config.trk_tlm_EQNS[i][1]);
                                        strcat(rawInfo, name);
                                        if (fmod(config.trk_tlm_EQNS[i][2], 1) == 0)
                                            sprintf(name, ",%0.f", config.trk_tlm_EQNS[i][2]);
                                        else
                                            sprintf(name, ",%.3f", config.trk_tlm_EQNS[i][2]);
                                        strcat(rawInfo, name);
                                    }
                                }
                                for (int n = c; n > 0; n--)
                                {
                                    strcat(rawInfo, ",");
                                    sprintf(name, "0");
                                    strcat(rawInfo, name);
                                    sprintf(name, ",1");
                                    strcat(rawInfo, name);
                                    sprintf(name, ",0");
                                    strcat(rawInfo, name);
                                }
                                // strcat(rawInfo, ",");
                                sendTelemetry_trk(rawInfo);
                            }

                            if (++TLM_SEQ > 8279)
                                TLM_SEQ = 0;
                            memset(tlm_data, 0, 200);
                            memset(tlm_result, 0, 100);
                            int n = 0;
                            sprintf(tlm_data, "%i", TLM_SEQ);
                            for (int s = 0; s < 5; s++)
                            {
                                if (config.trk_tlm_sensor[s] == 0)
                                {
                                    continue;
                                    // strcat(tlm_data, "0");
                                }
                                else
                                {
                                    strcat(tlm_data, ",");
                                    int sen_idx = config.trk_tlm_sensor[s] - 1;
                                    double data = 0;
                                    if (sen[sen_idx].visable)
                                        data = sen[sen_idx].sample;
                                    double precision = pow(10.0f, (double)config.trk_tlm_precision[s]);
                                    int val = (int)((data + config.trk_tlm_offset[s]) * precision);
                                    // log_d("s:%d Data:%.2f /tPresion:%.5f /tOffset:%.5f/t Val:%d",s,sen[sen_idx].sample,precision,config.trk_tlm_offset[s],val);
                                    if (val > 8280)
                                        val = 8280;
                                    if (val < 0)
                                        val = 0;
                                    char strVal[10];
                                    sprintf(strVal, "%i", val);
                                    strcat(tlm_data, strVal);
                                }
                            }
                            // log_d("TLM_DATA:%s",tlm_data);
                            //  sprintf(tlm_data, "%i,%i,%i,%i", TLM_SEQ, (int)(VBat * 100), int(TempNTC * 100), gps.satellites.value());
                            telemetry_base91(tlm_data, tlm_result, tlm_sz);
                            cmn = String(tlm_result);
                        }
                    }
                }

                if (config.trk_rssi)
                {
                    cmn += " ?RSSI";
                }

                if (config.trk_gps) // TRACKER by GPS
                {
                    rawData = trk_gps_postion(cmn);
                    if (config.log & LOG_TRACKER)
                    {
                        double trkLat, trkLng, trkSpd, trkCrs;
                        xSemaphoreTake(gpsMutex, portMAX_DELAY);
                        trkLat = gps.location.lat();
                        trkLng = gps.location.lng();
                        trkSpd = gps.speed.kmph();
                        trkCrs = gps.course.deg();
                        xSemaphoreGive(gpsMutex);
                        logTracker(trkLat, trkLng, trkSpd, trkCrs);
                    }
                }
                else // TRACKER by FIX position
                {
                    rawData = trk_fix_position(cmn);
                    if (config.log & LOG_TRACKER)
                    {
                        logTracker(config.trk_lat, config.trk_lon, 0, 0);
                    }
                }

                log_d("TRACKER RAW: %s\n", rawData.c_str());
                log_d("TRACKER EVENT_TX_POSITION=%d\t INTERVAL=%d\n", EVENT_TX_POSITION, tx_interval);
                tx_counter = 0;
                EVENT_TX_POSITION = 0;
                last_heading = SB_HEADING;
#if defined OLED || defined ST7735_160x80
                if (config.trk_gps)
                {
                    // if (gps.location.isValid() && (gps.hdop.hdop() < 10.0))
                    sprintf(sts, "POSITION GPS\nSPD %dkPh/%d\nINTERVAL %ds", SB_SPEED, SB_HEADING, tx_interval);
                    // else
                    //     sprintf(sts, "POSITION GPS\nGPS INVALID\nINTERVAL %ds", tx_interval);
                }
                else
                {
                    sprintf(sts, "POSITION FIX\nINTERVAL %ds", tx_interval);
                }
                char name[12];
                if (strlen(config.trk_item) > 3)
                {
                    sprintf(name, "%s", config.trk_item);
                }
                else
                {
                    if (config.trk_ssid > 0)
                        sprintf(name, "%s-%d", config.trk_mycall, config.trk_ssid);
                    else
                        sprintf(name, "%s", config.trk_mycall);
                }
#endif
                uint8_t SendMode = 0;
                if (config.trk_loc2rf)
                    SendMode |= RF_CHANNEL;
                if (config.trk_loc2inet)
                    SendMode |= INET_CHANNEL;
                pkgTxPush(rawData.c_str(), rawData.length(), 0, SendMode);

                //                 if (config.trk_loc2rf)
                //                 { // TRACKER SEND TO RF
                //                     char *rawP = (char *)calloc(rawData.length(), sizeof(char));
                //                     memcpy(rawP, rawData.c_str(), rawData.length());
                //                     // rawData.toCharArray(rawP, rawData.length());
                //                     pkgTxPush(rawP, rawData.length(), 0);

#if defined OLED || defined ST7735_160x80
                if (config.oled_enable)
                    pushTxDisp(TXCH_RF, name, sts);
#endif
                //                     free(rawP);
                //                 }
                //                 if (config.trk_loc2inet)
                //                 { // TRACKER SEND TO APRS-IS
                //                     if (aprsClient.connected())
                //                     {
                //                         aprsClient.println(rawData); // Send packet to Inet
                // #if defined OLED || defined ST7735_160x80
                //                         // pushTxDisp(TXCH_TCP, "TX TRACKER", sts);
                // #endif
                //                     }
                //                 }
                rawData.clear();
                cmn.clear();
            }
        }

        // LOAD DATA incomming
        newIGatePkg = false;
        newDigiPkg = false;
        type = 0;
        // if (PacketBuffer.getCount() > 0)
        if (Ax25NewRxFrames())
        {
            if (Ax25ReadNextRxFrame(&buf, &size, &peak, &valley, &signalLevel, &fixed, &mV))
            {
                String tnc2 = "";
                // นำข้อมูลแพ็จเกจจาก TNC ออกจากคิว
                ax25_decode(buf, size, mV, &incomingPacket);
                status.rxCount++;
                if (packet2Raw(tnc2, incomingPacket) > 0)
                {
                    log_d("Peak:%d Valley:%d Signal:%d mV:%d", peak, valley, signalLevel, mV);
                    log_d("RX TNC2: %s", tnc2.c_str());
                    type = pkgType((const char *)incomingPacket.info);
                    newIGatePkg = true;
                    newDigiPkg = true;
                    if (config.ext_tnc_enable)
                    {
                        if (config.ext_tnc_channel > 0 && config.ext_tnc_channel < 5)
                        {
                            if (config.ext_tnc_mode == 1)
                            {
                                // KISS MODE
                                uint8_t pkg[500];
                                int sz = kiss_wrapper(pkg, buf, size);
                                if (config.ext_tnc_channel == 1)
                                {
                                    Serial0.write(pkg, sz);
                                }
                                else if (config.ext_tnc_channel == 2)
                                {
                                    Serial1.write(pkg, sz);
                                }
#ifdef ARDUINO_USB_MODE
                                else if (config.ext_tnc_channel == 4)
                                {
                                    Serial.write(pkg, sz);
                                }
#endif
#if SOC_UART_NUM > 2
                                else if (config.ext_tnc_channel == 3)
                                {
                                    Serial2.write(pkg, sz);
                                }
#endif
                            }
                            else if (config.ext_tnc_mode == 2)
                            {
                                // TNC2
                                if (config.ext_tnc_channel == 1)
                                {
                                    Serial0.println(tnc2);
                                }
                                else if (config.ext_tnc_channel == 2)
                                {
                                    Serial1.println(tnc2);
                                }
#ifdef ARDUINO_USB_MODE
                                else if (config.ext_tnc_channel == 4)
                                {
                                    Serial.println(tnc2);
                                }
#endif
#if SOC_UART_NUM > 2
                                else if (config.ext_tnc_channel == 3)
                                {
                                    Serial2.println(tnc2);
                                }
#endif
                            }
                        }
                    }
// #ifdef BLUETOOTH
//                     if (config.bt_master)
//                     { // Output TNC2RAW to BT Serial
//                       // SerialBT.println(tnc2);
//                         if (config.bt_mode == 1)
//                         {
//                             char *rawP = (char *)malloc(tnc2.length());
//                             memcpy(rawP, tnc2.c_str(), tnc2.length());
// #if defined(CONFIG_IDF_TARGET_ESP32)
//                             SerialBT.write((uint8_t *)rawP, tnc2.length());
// #else
//                             if (NuSerial.isConnected())
//                             {
//                                 NuSerial.write((uint8_t *)rawP, tnc2.length());
//                             }
// #endif
//                             free(rawP);
//                         }
//                         else if (config.bt_mode == 2)
//                         { // KISS
//                             uint8_t pkg[500];
//                             int sz = kiss_wrapper(pkg);
// #if defined(CONFIG_IDF_TARGET_ESP32)
//                             SerialBT.write(pkg, sz);
// #else
//                             if (NuSerial.isConnected())
//                             {
//                                 NuSerial.write(pkg, sz);
//                             }
// #endif
//                         }
//                     }
// #endif

#ifdef MQTT
                    if (config.en_mqtt && clientMQTT.connected() && (config.mqtt_topic_flag & MQTT_TOPIC_TNC))
                    {
                        log_d("Publish MQTT Topic: %s Payload: %s", config.mqtt_topic, tnc2.c_str());
                        clientMQTT.publish(config.mqtt_topic, tnc2.c_str());
                    }
#endif
                    // SerialBT.println(tnc2);
                    // uint16_t type = pkgType((char *)incomingPacket.info);
                    if (!(type & FILTER_THIRDPARTY))
                    {
                        char call[11];
                        if (incomingPacket.src.ssid > 0)
                            sprintf(call, "%s-%d", incomingPacket.src.call, incomingPacket.src.ssid);
                        else
                            sprintf(call, "%s", incomingPacket.src.call);

                        char *rawP = (char *)calloc(tnc2.length() + 1, sizeof(char));
                        if (rawP)
                        {
                            memset(rawP, 0, tnc2.length() + 1);
                            tnc2.toCharArray(rawP, tnc2.length(), 0);
                            // memcpy(rawP, tnc2.c_str(), tnc2.length());
                            int idx = pkgListUpdate(call, rawP, type, 0, incomingPacket.mVrms);

#if defined OLED || defined ST7735_160x80
                            if ((config.oled_enable) && (idx > -1))
                            {

                                if (config.rx_display && config.dispRF && (type & config.dispFilter))
                                {
                                    dispBuffer.push(tnc2.c_str());
                                    log_d("RF_putQueueDisp:[pkgList_idx=%d,Type=%d RAW:%s] %s\n", idx, type, call, tnc2.c_str());
                                }
                            }
#endif
                            handle_ws(rawP, tnc2.length(), incomingPacket.mVrms);
                            free(rawP);
                        }
                    }

                    if (config.msg_enable && (type & FILTER_MESSAGE))
                    {
                        handleIncomingAPRS(tnc2);
                    }
                    lastPkg = true;
                    // handle_ws(tnc2, incomingPacket.mVrms);
                    //   ESP_BT.println(tnc2);
                    status.allCount++;

                    tnc2.clear();
                }
            }
        }

        // IGate Process
        if (config.igate_en)
        {
            if (config.igate_sts_interval > 10)
            {
                if (millis() > igateSTSInterval)
                {
                    igateSTSInterval = millis() + (config.igate_sts_interval * 1000);
                    igate_status(config.igate_status);
                }
            }
            // IGATE Position
            if (config.igate_bcn)
            {
                if (millis() > iGatetickInterval)
                {

                    String rawData = "";
                    if (config.igate_gps)
                    { // IGATE Send GPS position
                        double igLat, igLng, igAlt, igSpd, igCrs;
                        bool igGpsValid;
                        xSemaphoreTake(gpsMutex, portMAX_DELAY);
                        igGpsValid = gps.location.isValid();
                        if (igGpsValid)
                        {
                            igLat = gps.location.lat();
                            igLng = gps.location.lng();
                            igAlt = gps.altitude.meters();
                            igSpd = gps.speed.kmph();
                            igCrs = gps.course.deg();
                        }
                        xSemaphoreGive(gpsMutex);
                        if (igGpsValid)
                        {
                            rawData = igate_position(igLat, igLng, igAlt, "");
                            if (config.log & LOG_IGATE)
                                logIGate(igLat, igLng, igSpd, igCrs);
                        }
                    }
                    else
                    { // IGATE Send fix position
                        rawData = igate_position(config.igate_lat, config.igate_lon, config.igate_alt, "");
                        if (config.log & LOG_TRACKER)
                        {
                            logIGate(config.igate_lat, config.igate_lon, 0, 0);
                        }
                    }
                    if (rawData != "")
                    {
                        iGatetickInterval = millis() + (config.igate_interval * 1000);
                        Sleep_Activate &= ~ACTIVATE_IGATE;
                        StandByTick = millis() + (5000);
                        if (config.igate_tlm_interval > 0)
                        {
                            igateTlmInvCount++;
                            if (igateTlmInvCount >= config.igate_tlm_interval)
                            {
                                igateTlmInvCount = 0;
                                if (config.igate_tlm_sensor[0] | config.igate_tlm_sensor[1] | config.igate_tlm_sensor[2] | config.igate_tlm_sensor[3] | config.igate_tlm_sensor[4])
                                {
                                    char tlm_result[100];
                                    char tlm_data[200];
                                    size_t tlm_sz;
                                    if ((IGATE_TLM_SEQ % 100) == 0)
                                    {
                                        char rawInfo[256];
                                        char name[10];
                                        sprintf(rawInfo, "PARM.");
                                        int i, c = 0;
                                        for (i = 0; i < 5; i++)
                                        {
                                            if (config.igate_tlm_sensor[i] == 0)
                                            {
                                                c++;
                                                continue;
                                            }
                                            else
                                            {
                                                if (i > 0)
                                                    strcat(rawInfo, ",");
                                                sprintf(name, "%s", config.igate_tlm_PARM[i]);
                                                strcat(rawInfo, name);
                                            }
                                        }
                                        for (int n = c + 8; n > 0; n--)
                                        {
                                            strcat(rawInfo, ",");
                                        }
                                        sendTelemetry_igate(rawInfo);
                                        memset(rawInfo, 0, sizeof(rawInfo));
                                        sprintf(rawInfo, "UNIT.");
                                        c = 0;
                                        for (i = 0; i < 5; i++)
                                        {
                                            if (config.igate_tlm_sensor[i] == 0)
                                            {
                                                c++;
                                                continue;
                                            }
                                            else
                                            {
                                                if (i > 0)
                                                    strcat(rawInfo, ",");
                                                sprintf(name, "%s", config.igate_tlm_UNIT[i]);
                                                strcat(rawInfo, name);
                                            }
                                        }
                                        for (int n = c + 8; n > 0; n--)
                                        {
                                            strcat(rawInfo, ",");
                                        }
                                        sendTelemetry_igate(rawInfo);
                                        memset(rawInfo, 0, sizeof(rawInfo));
                                        sprintf(rawInfo, "EQNS.");
                                        c = 0;
                                        for (i = 0; i < 5; i++)
                                        {
                                            if (config.igate_tlm_sensor[i] == 0)
                                            {
                                                c++;
                                                continue;
                                            }
                                            else
                                            {
                                                if (i > 0)
                                                    strcat(rawInfo, ",");
                                                if (fmod(config.igate_tlm_EQNS[i][0], 1) == 0)
                                                    sprintf(name, "%0.f", config.igate_tlm_EQNS[i][0]);
                                                else
                                                    sprintf(name, "%.3f", config.igate_tlm_EQNS[i][0]);
                                                strcat(rawInfo, name);
                                                if (fmod(config.igate_tlm_EQNS[i][1], 1) == 0)
                                                    sprintf(name, ",%0.f", config.igate_tlm_EQNS[i][1]);
                                                else
                                                    sprintf(name, ",%.3f", config.igate_tlm_EQNS[i][1]);
                                                strcat(rawInfo, name);
                                                if (fmod(config.igate_tlm_EQNS[i][2], 1) == 0)
                                                    sprintf(name, ",%0.f", config.igate_tlm_EQNS[i][2]);
                                                else
                                                    sprintf(name, ",%.3f", config.igate_tlm_EQNS[i][2]);
                                                strcat(rawInfo, name);
                                            }
                                        }
                                        for (int n = c; n > 0; n--)
                                        {
                                            strcat(rawInfo, ",");
                                            sprintf(name, "0");
                                            strcat(rawInfo, name);
                                            sprintf(name, ",1");
                                            strcat(rawInfo, name);
                                            sprintf(name, ",0");
                                            strcat(rawInfo, name);
                                        }
                                        // strcat(rawInfo, ",");
                                        sendTelemetry_igate(rawInfo);
                                    }

                                    if (++IGATE_TLM_SEQ > 8279)
                                        IGATE_TLM_SEQ = 0;
                                    memset(tlm_data, 0, 200);
                                    memset(tlm_result, 0, 100);
                                    int n = 0;
                                    sprintf(tlm_data, "%i", IGATE_TLM_SEQ);
                                    for (int s = 0; s < 5; s++)
                                    {
                                        if (config.igate_tlm_sensor[s] == 0)
                                        {
                                            continue;
                                            // strcat(tlm_data, "0");
                                        }
                                        else
                                        {
                                            strcat(tlm_data, ",");
                                            int sen_idx = config.igate_tlm_sensor[s] - 1;
                                            double data = 0;
                                            if (sen[sen_idx].visable)
                                                data = sen[sen_idx].sample;
                                            double precision = pow(10.0f, (double)config.igate_tlm_precision[s]);
                                            int val = (int)((data + config.igate_tlm_offset[s]) * precision);
                                            // log_d("s:%d Data:%.2f /tPresion:%.5f /tOffset:%.5f/t Val:%d",s,sen[sen_idx].sample,precision,config.trk_tlm_offset[s],val);
                                            if (val > 8280)
                                                val = 8280;
                                            if (val < 0)
                                                val = 0;
                                            char strVal[10];
                                            sprintf(strVal, "%i", val);
                                            strcat(tlm_data, strVal);
                                        }
                                    }
                                    // log_d("TLM_DATA:%s",tlm_data);
                                    //  sprintf(tlm_data, "%i,%i,%i,%i", TLM_SEQ, (int)(VBat * 100), int(TempNTC * 100), gps.satellites.value());
                                    telemetry_base91(tlm_data, tlm_result, tlm_sz);
                                    rawData += String(tlm_result);
                                }
                            }
                        }
                        if (strlen(config.igate_comment) > 0)
                        {
                            rawData += String(config.igate_comment);
                        }

                        log_d("IGATE_POSITION: %s", rawData.c_str());

                        if (config.igate_gps)
                            sprintf(sts, "POSITION GPS\nINTERVAL %ds", tx_interval);
                        else
                            sprintf(sts, "POSITION FIX\nINTERVAL %ds", tx_interval);

                        uint8_t SendMode = 0;
                        if (config.igate_loc2rf)
                            SendMode |= RF_CHANNEL;
                        if (config.igate_loc2inet)
                            SendMode |= INET_CHANNEL;
                        pkgTxPush(rawData.c_str(), rawData.length(), 0, SendMode);
//                         if (config.igate_loc2rf)
//                         { // IGATE SEND POSITION TO RF
//                             char *rawP = (char *)calloc(rawData.length(), sizeof(char));
//                             // rawData.toCharArray(rawP, rawData.length());
//                             memcpy(rawP, rawData.c_str(), rawData.length());
//                             pkgTxPush(rawP, rawData.length(), 0);
#if defined OLED || defined ST7735_160x80
                        if (config.oled_enable)
                            pushTxDisp(TXCH_RF, "TX IGATE", sts);
#endif
                        //                             free(rawP);
                        //                         }
                        //                         if (config.igate_loc2inet)
                        //                         { // IGATE SEND TO APRS-IS
                        //                             if (aprsClient.connected())
                        //                             {
                        //                                 status.txCount++;
                        //                                 aprsClient.println(rawData); // Send packet to Inet
                        // #if defined OLED || defined ST7735_160x80
                        //                                 pushTxDisp(TXCH_TCP, "TX IGATE", sts);
                        // #endif
                        //                             }
                        //                         }
                    }
                }
            }
            // IGATE send to inet
            if ((newIGatePkg && aprsClient.connected() == true))
            {
                newIGatePkg = false;
                // if (config.rf2inet && aprsClient.connected())
                if (config.rf2inet)
                {
                    int ret = 0;
                    // uint16_t type = pkgType((const char *)&incomingPacket.info[0]);
                    //  IGate Filter RF->INET
                    if ((type & config.rf2inetFilter))
                        ret = igateProcess(incomingPacket);
                    if (ret == 0)
                    {
                        status.dropCount++;
                        igateTLM.DROP++;
                    }
                    else
                    {
                        status.rf2inet++;
                        igateTLM.RF2INET++;
                        igateTLM.TX++;
                    }
                }
            }
            if (config.digi_auto)
            {
                DiGiInterval = millis() + (config.digi_interval * 1000);
            }
        }

        // Digi Repeater Process
        if (config.digi_en || (config.digi_auto && (aprsClient.connected() == false)))
        {
            if (config.digi_sts_interval > 10)
            {
                if (millis() > digiSTSInterval)
                {
                    digiSTSInterval = millis() + (config.digi_sts_interval * 1000);
                    digi_status(config.digi_status);
                }
            }
            // DIGI Position
            if (config.digi_bcn)
            {
                if (millis() > DiGiInterval)
                {

                    String rawData;
                    if (config.digi_gps)
                    { // DIGI Send GPS position
                        double digiLat, digiLng, digiAlt, digiSpd, digiCrs;
                        bool digiGpsValid;
                        xSemaphoreTake(gpsMutex, portMAX_DELAY);
                        digiGpsValid = gps.location.isValid();
                        if (digiGpsValid)
                        {
                            digiLat = gps.location.lat();
                            digiLng = gps.location.lng();
                            digiAlt = gps.altitude.meters();
                            digiSpd = gps.speed.kmph();
                            digiCrs = gps.course.deg();
                        }
                        xSemaphoreGive(gpsMutex);
                        if (digiGpsValid)
                        {
                            rawData = digi_position(digiLat, digiLng, digiAlt, "");
                            if (config.log & LOG_DIGI)
                                logDigi(digiLat, digiLng, digiSpd, digiCrs);
                        }
                    }
                    else
                    { // DIGI Send fix position
                        rawData = digi_position(config.digi_lat, config.digi_lon, config.digi_alt, "");
                        if (config.log & LOG_DIGI)
                        {
                            logDigi(config.digi_lat, config.digi_lon, 0, 0);
                        }
                    }
                    if (rawData != "")
                    {
                        DiGiInterval = millis() + (config.digi_interval * 1000);
                        Sleep_Activate &= ~ACTIVATE_DIGI;
                        StandByTick = millis() + (5000);
                        if (config.digi_tlm_interval > 0)
                        {
                            digiTlmInvCount++;
                            if (digiTlmInvCount >= config.digi_tlm_interval)
                            {
                                digiTlmInvCount = 0;

                                if (config.digi_tlm_sensor[0] | config.digi_tlm_sensor[1] | config.digi_tlm_sensor[2] | config.digi_tlm_sensor[3] | config.digi_tlm_sensor[4])
                                {
                                    char tlm_result[100];
                                    char tlm_data[200];
                                    size_t tlm_sz;
                                    if ((DIGI_TLM_SEQ % 100) == 0)
                                    {
                                        char rawInfo[256];
                                        char name[10];
                                        sprintf(rawInfo, "PARM.");
                                        int i, c = 0;
                                        for (i = 0; i < 5; i++)
                                        {
                                            if (config.digi_tlm_sensor[i] == 0)
                                            {
                                                c++;
                                                continue;
                                            }
                                            else
                                            {
                                                if (i > 0)
                                                    strcat(rawInfo, ",");
                                                sprintf(name, "%s", config.digi_tlm_PARM[i]);
                                                strcat(rawInfo, name);
                                            }
                                        }
                                        for (int n = c + 8; n > 0; n--)
                                        {
                                            strcat(rawInfo, ",");
                                        }
                                        sendTelemetry_digi(rawInfo);
                                        memset(rawInfo, 0, sizeof(rawInfo));
                                        sprintf(rawInfo, "UNIT.");
                                        c = 0;
                                        for (i = 0; i < 5; i++)
                                        {
                                            if (config.digi_tlm_sensor[i] == 0)
                                            {
                                                c++;
                                                continue;
                                            }
                                            else
                                            {
                                                if (i > 0)
                                                    strcat(rawInfo, ",");
                                                sprintf(name, "%s", config.digi_tlm_UNIT[i]);
                                                strcat(rawInfo, name);
                                            }
                                        }
                                        for (int n = c + 8; n > 0; n--)
                                        {
                                            strcat(rawInfo, ",");
                                        }
                                        sendTelemetry_digi(rawInfo);
                                        memset(rawInfo, 0, sizeof(rawInfo));
                                        sprintf(rawInfo, "EQNS.");
                                        c = 0;
                                        for (i = 0; i < 5; i++)
                                        {
                                            if (config.digi_tlm_sensor[i] == 0)
                                            {
                                                c++;
                                                continue;
                                            }
                                            else
                                            {
                                                if (i > 0)
                                                    strcat(rawInfo, ",");
                                                if (fmod(config.digi_tlm_EQNS[i][0], 1) == 0)
                                                    sprintf(name, "%0.f", config.digi_tlm_EQNS[i][0]);
                                                else
                                                    sprintf(name, "%.3f", config.digi_tlm_EQNS[i][0]);
                                                strcat(rawInfo, name);
                                                if (fmod(config.digi_tlm_EQNS[i][1], 1) == 0)
                                                    sprintf(name, ",%0.f", config.digi_tlm_EQNS[i][1]);
                                                else
                                                    sprintf(name, ",%.3f", config.digi_tlm_EQNS[i][1]);
                                                strcat(rawInfo, name);
                                                if (fmod(config.digi_tlm_EQNS[i][2], 1) == 0)
                                                    sprintf(name, ",%0.f", config.digi_tlm_EQNS[i][2]);
                                                else
                                                    sprintf(name, ",%.3f", config.digi_tlm_EQNS[i][2]);
                                                strcat(rawInfo, name);
                                            }
                                        }
                                        for (int n = c; n > 0; n--)
                                        {
                                            strcat(rawInfo, ",");
                                            sprintf(name, "0");
                                            strcat(rawInfo, name);
                                            sprintf(name, ",1");
                                            strcat(rawInfo, name);
                                            sprintf(name, ",0");
                                            strcat(rawInfo, name);
                                        }
                                        // strcat(rawInfo, ",");
                                        sendTelemetry_digi(rawInfo);
                                    }

                                    if (++DIGI_TLM_SEQ > 8279)
                                        DIGI_TLM_SEQ = 0;
                                    memset(tlm_data, 0, 200);
                                    memset(tlm_result, 0, 100);
                                    int n = 0;
                                    sprintf(tlm_data, "%i", DIGI_TLM_SEQ);
                                    for (int s = 0; s < 5; s++)
                                    {
                                        if (config.digi_tlm_sensor[s] == 0)
                                        {
                                            continue;
                                            // strcat(tlm_data, "0");
                                        }
                                        else
                                        {
                                            strcat(tlm_data, ",");
                                            int sen_idx = config.digi_tlm_sensor[s] - 1;
                                            double data = 0;
                                            if (sen[sen_idx].visable)
                                                data = sen[sen_idx].sample;
                                            double precision = pow(10.0f, (double)config.digi_tlm_precision[s]);
                                            int val = (int)((data + config.digi_tlm_offset[s]) * precision);
                                            // log_d("s:%d Data:%.2f /tPresion:%.5f /tOffset:%.5f/t Val:%d",s,sen[sen_idx].sample,precision,config.trk_tlm_offset[s],val);
                                            if (val > 8280)
                                                val = 8280;
                                            if (val < 0)
                                                val = 0;
                                            char strVal[10];
                                            sprintf(strVal, "%i", val);
                                            strcat(tlm_data, strVal);
                                        }
                                    }
                                    // log_d("TLM_DATA:%s",tlm_data);
                                    //  sprintf(tlm_data, "%i,%i,%i,%i", TLM_SEQ, (int)(VBat * 100), int(TempNTC * 100), gps.satellites.value());
                                    telemetry_base91(tlm_data, tlm_result, tlm_sz);
                                    rawData += String(tlm_result);
                                }
                            }
                        }
                        if (strlen(config.digi_comment) > 0)
                        {
                            rawData += String(config.digi_comment);
                        }

                        log_d("DIGI_POSITION: %s", rawData.c_str());

                        if (config.digi_gps)
                            sprintf(sts, "POSITION GPS\nINTERVAL %ds", tx_interval);
                        else
                            sprintf(sts, "POSITION FIX\nINTERVAL %ds", tx_interval);

                        uint8_t SendMode = 0;
                        if (config.digi_loc2rf)
                            SendMode |= RF_CHANNEL;
                        if (config.digi_loc2inet)
                            SendMode |= INET_CHANNEL;
                        pkgTxPush(rawData.c_str(), rawData.length(), 0, SendMode);
//                         if (config.digi_loc2rf)
//                         { // DIGI SEND POSITION TO RF
//                             char *rawP = (char *)calloc(rawData.length(), sizeof(char));
//                             // rawData.toCharArray(rawP, rawData.length());
//                             memcpy(rawP, rawData.c_str(), rawData.length());
//                             pkgTxPush(rawP, rawData.length(), 0);
#if defined OLED || defined ST7735_160x80
                        if (config.oled_enable)
                            pushTxDisp(TXCH_RF, "TX DIGI POS", sts);
#endif
                        //                             free(rawP);
                        //                         }
                        //                         if (config.digi_loc2inet)
                        //                         { // DIGI SEND TO APRS-IS
                        //                             if (aprsClient.connected())
                        //                             {
                        //                                 status.txCount++;
                        //                                 aprsClient.println(rawData); // Send packet to Inet
                        // #if defined OLED || defined ST7735_160x80
                        //                                 pushTxDisp(TXCH_TCP, "TX DIGI POS", sts);
                        // #endif
                        //                             }
                        //                         }
                    }
                    rawData.clear();
                }
            }

            // Repeater packet
            if (newDigiPkg)
            {
                newDigiPkg = false;
                // uint16_t type = pkgType((const char *)&incomingPacket.info[0]);
                Sleep_Activate &= ~ACTIVATE_DIGI;
                StandByTick = millis() + (config.pwr_stanby_delay * 1000);
                // Digi repeater filter
                if ((type & config.digiFilter))
                {
                    // Packet recheck
                    pkgTxDuplicate(incomingPacket); // Search duplicate in tx and drop packet for renew
                    int dlyFlag = digiProcess(incomingPacket);
                    if (dlyFlag > 0)
                    {
                        int digiDelay;
                        status.digiCount++;
                        if (dlyFlag == 1)
                        {
                            digiDelay = 0;
                        }
                        else
                        {
                            if (config.digi_delay == 0)
                            { // Auto mode
                              // if (digiCount > 20)
                              //   digiDelay = random(5000);
                              // else if (digiCount > 10)
                              //   digiDelay = random(3000);
                              // else if (digiCount > 0)
                              //   digiDelay = random(1500);
                              // else
                                digiDelay = random(100);
                            }
                            else
                            {
                                digiDelay = random(config.digi_delay);
                            }
                        }

                        String digiPkg;
                        packet2Raw(digiPkg, incomingPacket);
                        log_d("DIGI_REPEAT: %s", digiPkg.c_str());
                        log_d("DIGI delay=%d ms.", digiDelay);
                        pkgTxPush(digiPkg.c_str(), digiPkg.length(), digiDelay, RF_CHANNEL);
                        digiPkg.clear();
                        sprintf(sts, "--src call--\n%s\nDelay: %dms.", incomingPacket.src.call, digiDelay);
#if defined OLED || defined ST7735_160x80
                        if (config.oled_enable)
                            pushTxDisp(TXCH_DIGI, "DIGI REPEAT", sts);
#endif
                        // free(rawP);
                    }
                }
            }
        }

        // Weather
        if (config.wx_en)
        {
            if (millis() > WxInterval)
            {

                String rawData = "";
                if (config.wx_gps)
                { // Wx Send GPS position
                    double wxLat, wxLng, wxAlt, wxSpd, wxCrs;
                    bool wxGpsValid;
                    xSemaphoreTake(gpsMutex, portMAX_DELAY);
                    wxGpsValid = gps.location.isValid();
                    if (wxGpsValid)
                    {
                        wxLat = gps.location.lat();
                        wxLng = gps.location.lng();
                        wxAlt = gps.altitude.meters();
                        wxSpd = gps.speed.kmph();
                        wxCrs = gps.course.deg();
                    }
                    xSemaphoreGive(gpsMutex);
                    if (wxGpsValid)
                    {
                        rawData = wx_report(wxLat, wxLng, wxAlt, "");
                        if (config.log & LOG_WX)
                            logWeather(wxLat, wxLng, wxSpd, wxCrs);
                    }
                }
                else
                { // Wx Send fix position
                    rawData = wx_report(config.wx_lat, config.wx_lon, config.wx_alt, "");
                    if (config.log & LOG_WX)
                    {
                        logWeather(config.wx_lat, config.wx_lon, 0, 0);
                    }
                }
                if (rawData != "")
                {
                    WxInterval = millis() + (config.wx_interval * 1000);
                    Sleep_Activate &= ~ACTIVATE_WX;
                    StandByTick = millis() + (5000);
                    log_d("WX_REPORT: %s", rawData.c_str());
                    uint8_t SendMode = 0;
                    if (config.wx_2rf)
                        SendMode |= RF_CHANNEL;
                    if (config.wx_2inet)
                        SendMode |= INET_CHANNEL;
                    pkgTxPush(rawData.c_str(), rawData.length(), 0, SendMode);
//                     if (config.wx_2rf)
//                     { // WX SEND POSITION TO RF
//                         char *rawP = (char *)calloc(rawData.length(), sizeof(char));
//                         // rawData.toCharArray(rawP, rawData.length());
//                         memcpy(rawP, rawData.c_str(), rawData.length());
//                         pkgTxPush(rawP, rawData.length(), 0);
#ifdef OLED
                    sprintf(sts, "--src call--\n%s\nDelay: %dms.", config.wx_mycall, (config.wx_interval * 1000));
                    if (config.oled_enable)
                        pushTxDisp(TXCH_RF, "WX REPORT", sts);
#endif
                    //                         free(rawP);
                    //                     }
                    //                     if (config.wx_2inet)
                    //                     { // WX SEND TO APRS-IS
                    //                         if (aprsClient.connected())
                    //                         {
                    //                             status.txCount++;
                    //                             aprsClient.println(rawData); // Send packet to Inet
                    // #ifdef OLED
                    //                             // pushTxDisp(TXCH_TCP, "WX REPORT", sts);
                    // #endif
                    //                         }
                    //                     }
                }
                else
                {
                    WxInterval = millis() + (10 * 1000);
                }
#ifdef MQTT
                if (config.en_mqtt && (config.mqtt_topic_flag & MQTT_TOPIC_WX) && clientMQTT.connected())
                {
                    char payload[500];
                    char topic[100];
                    if (strlen(config.wx_object) < 3)
                        sprintf(topic, "/%s/WEATHER/sample", config.wx_mycall);
                    else
                        sprintf(topic, "/%s/WEATHER/sample", config.wx_object);
                    getWxJson(&payload[0], false);
                    log_d("Publish MQTT Topic: %s Payload: %s", config.mqtt_topic, payload);
                    clientMQTT.publish(config.mqtt_topic, payload);
                }
#endif
            }
#ifdef MQTT
            if (millis() > WxIntervalAvg)
            {
                WxIntervalAvg = millis() + (600 * 1000);
                if (config.en_mqtt && (config.mqtt_topic_flag & MQTT_TOPIC_WX) && clientMQTT.connected())
                {
                    char payload[500];
                    char topic[100];
                    if (strlen(config.wx_object) < 3)
                        sprintf(topic, "/%s/WEATHER/average", config.wx_mycall);
                    else
                        sprintf(topic, "/%s/WEATHER/average", config.wx_object);
                    getWxJson(&payload[0], true);
                    log_d("Publish MQTT Topic: %s Payload: %s", topic, payload);
                    clientMQTT.publish(topic, payload);
                }
            }
#endif
        }

        if (config.tlm0_en)
        {
            if (systemTLM.ParmTimeout < millis())
            {
                systemTLM.ParmTimeout = millis() + (config.tlm0_info_interval * 1000);
                char rawInfo[256];
                char name[10];
                sprintf(rawInfo, "PARM.");
                for (int i = 0; i < 13; i++)
                {
                    if (i > 0)
                        strcat(rawInfo, ",");
                    sprintf(name, "%s", config.tlm0_PARM[i]);
                    strcat(rawInfo, name);
                }
                sendTelemetry_0(rawInfo, true);
                memset(rawInfo, 0, sizeof(rawInfo));
                sprintf(rawInfo, "UNIT.");
                for (int i = 0; i < 13; i++)
                {
                    if (i > 0)
                        strcat(rawInfo, ",");
                    sprintf(name, "%s", config.tlm0_UNIT[i]);
                    strcat(rawInfo, name);
                }
                sendTelemetry_0(rawInfo, true);
                memset(rawInfo, 0, sizeof(rawInfo));
                sprintf(rawInfo, "EQNS.");
                for (int i = 0; i < 5; i++)
                {
                    if (i > 0)
                        strcat(rawInfo, ",");
                    if (fmod(config.tlm0_EQNS[i][0], 1) == 0)
                        sprintf(name, "%0.f", config.tlm0_EQNS[i][0]);
                    else
                        sprintf(name, "%.3f", config.tlm0_EQNS[i][0]);
                    strcat(rawInfo, name);
                    if (fmod(config.tlm0_EQNS[i][1], 1) == 0)
                        sprintf(name, ",%0.f", config.tlm0_EQNS[i][1]);
                    else
                        sprintf(name, ",%.3f", config.tlm0_EQNS[i][1]);
                    strcat(rawInfo, name);
                    if (fmod(config.tlm0_EQNS[i][2], 1) == 0)
                        sprintf(name, ",%0.f", config.tlm0_EQNS[i][2]);
                    else
                        sprintf(name, ",%.3f", config.tlm0_EQNS[i][2]);
                    strcat(rawInfo, name);
                }
                sendTelemetry_0(rawInfo, true);
                memset(rawInfo, 0, sizeof(rawInfo));
                sprintf(rawInfo, "BITS.");
                uint8_t b = 1;
                for (int i = 0; i < 8; i++)
                {
                    if (config.tlm0_BITS_Active & b)
                    {
                        strcat(rawInfo, "1");
                    }
                    else
                    {
                        strcat(rawInfo, "0");
                    }
                    b <<= 1;
                }
                strcat(rawInfo, ",");
                strcat(rawInfo, config.tlm0_comment);
                sendTelemetry_0(rawInfo, true);
            }

            if (systemTLM.TeleTimeout < millis())
            {
                systemTLM.TeleTimeout = millis() + (config.tlm0_data_interval * 1000);
                char rawTlm[100];
                if (systemTLM.Sequence > 999)
                    systemTLM.Sequence = 0;
                else
                    systemTLM.Sequence++;
                getTelemetry_0();
                sprintf(rawTlm, "T#%03d,%03d,%03d,%03d,%03d,%03d,", systemTLM.Sequence, systemTLM.A1, systemTLM.A2, systemTLM.A3, systemTLM.A4, systemTLM.A5);
                uint8_t b = 1;
                for (int i = 0; i < 8; i++)
                {
                    if (!((systemTLM.BITS & b) ^ (config.tlm0_BITS_Active & b)))
                    {
                        strcat(rawTlm, "1");
                    }
                    else
                    {
                        strcat(rawTlm, "0");
                    }
                    b <<= 1;
                }
                sendTelemetry_0(rawTlm, false);
            }
        }
    }
}
