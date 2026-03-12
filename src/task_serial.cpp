#include "webservice.h"    // config, aprsClient, status
#include "pkg_list.h"      // pkgType, pkgListUpdate
#include <KISS.h>          // kiss_serial
#include "handleATCommand.h"  // handleATCommand

extern char nmea[100];
extern int nmea_idx;
extern unsigned long timerSerial, timerSerial_old;

void taskSerial(void *pvParameters)
{
    String raw;
    int c;
    char rawP[500];
    char call[11];
    // Per-UART line accumulators for AT command input. Using accumulators instead
    // of readStringUntil() ensures partial lines from human-speed typing are not
    // dispatched prematurely and do not block the task while waiting for more data.
    String atBuf0, atBuf1, atBuf2, atBuf3;
    log_d("Serial task Init");
    nmea_idx = 0;
    if (config.ext_tnc_enable)
    {
        if (config.ext_tnc_channel == 1)
        {

#if ARDUINO_USB_MODE
            Serial.setTimeout(10);
#endif
        }
        else if (config.ext_tnc_channel == 2)
        {
            Serial1.setTimeout(10);
        }
        else if (config.ext_tnc_channel == 3)
        {
            Serial2.setTimeout(10);
        }
    }
    if (config.wx_en)
    {
        //         if (config.wx_channel == 1)
        //         {
        // #if ARDUINO_USB_CDC_ON_BOOT
        //             Serial0.setTimeout(10);
        // #else
        //             Serial.setTimeout(10);
        // #endif
        //         }
        //         else if (config.wx_channel == 2)
        //         {
        //             Serial1.setTimeout(10);
        //         }
        //         else if (config.wx_channel == 3)
        //         {
        //             // Serial2.setTimeout(10);
        //         }
    }
    for (;;)
    {
        timerSerial = micros() - timerSerial_old;
        vTaskDelay(10 / portTICK_PERIOD_MS);
        timerSerial_old = micros();

        if (config.wx_en)
        {
            //             if (config.wx_channel > 0 && config.wx_channel < 4)
            //             {
            //                 String wx = "";
            //                 if (config.wx_channel == 1)
            //                 {
            // #if ARDUINO_USB_CDC_ON_BOOT
            //                     wx = Serial.readString();
            // #else
            //                     wx = Serial.readString();
            // #endif
            //                 }
            //                 else if (config.wx_channel == 2)
            //                 {
            //                     wx = Serial1.readString();
            //                 }
            //                 else if (config.wx_channel == 3)
            //                 {
            //                     // wx = Serial2.readString();
            //                 }
            //                 // if (wx!="")
            //                 //{
            //                 // while (SerialWX->available())
            //                 //{
            //                 // String wx = SerialWX->readString();
            //                 if (wx != "" && wx.indexOf("DATA:") >= 0)
            //                 {
            //                     log_d("WX Raw >> %d", wx.c_str());
            //                     getCSV2Wx(wx);
            //                 }
            //                 //}
            //                 //}
            //             }
            // else if(config.wx_channel == 4){
            //     bool result=getM702Modbus(modbus);
            // }
        }

        if (config.ext_tnc_enable && (config.ext_tnc_mode > 0 && config.ext_tnc_mode < 5))
        {
            if (config.ext_tnc_mode == 1)
            { // KISS
                // KISS MODE
                do
                {
                    c = -1;
                    if (config.ext_tnc_channel == 1)
                    {
                        if (Serial0.available()) c = Serial0.read();
                    }
                    else if (config.ext_tnc_channel == 2)
                    {
                        if (Serial1.available()) c = Serial1.read();
                    }
#if SOC_UART_NUM > 2
                    else if (config.ext_tnc_channel == 3)
                    {
                        if (Serial2.available()) c = Serial2.read();
                    }
#endif
                    else if (config.ext_tnc_channel == 4)
                    {
                        if (Serial.available()) c = Serial.read();
                    }

                    if (c > -1)
                        kiss_serial((uint8_t)c);
                    else
                        break;
                } while (c > -1);
            }
            else if (config.ext_tnc_mode == 2)
            { // TNC2RAW
                raw.clear();
                if (config.ext_tnc_channel == 1)
                {
                    if (Serial0.available())
                        raw = Serial0.readStringUntil(0x0D);
                }
                else if (config.ext_tnc_channel == 2)
                {
                    if (Serial1.available())
                        raw = Serial1.readStringUntil(0x0D);
                }
#if SOC_UART_NUM > 2
                else if (config.ext_tnc_channel == 3)
                {
                    if (Serial2.available())
                        raw = Serial2.readStringUntil(0x0D);
                }
#endif
                else if (config.ext_tnc_channel == 4)
                {
                    if (Serial.available())
                        raw = Serial.readStringUntil(0x0D);
                }

                log_d("Ext TNC2RAW RX:%s", raw.c_str());
                String src_call = raw.substring(0, raw.indexOf('>'));
                if ((src_call != "") && (src_call.length() < 10) && (raw.length() < sizeof(rawP)))
                {
                    memset(call, 0, sizeof(call));
                    strlcpy(call, src_call.c_str(), sizeof(call));
                    strlcpy(rawP, raw.c_str(), sizeof(rawP));
                    uint16_t type = pkgType((const char *)rawP);
                    pkgListUpdate(call, rawP, type, 1, -1);
                    if (config.rf2inet && aprsClient.connected())
                    {
                        // RF->INET
                        aprsClient.write(&rawP[0], strlen(rawP)); // Send binary frame packet to APRS-IS (aprsc)
                        aprsClient.write("\r\n");                 // Send CR LF the end frame packet
                        status.rf2inet++;
                        // igateTLM.RF2INET++;
                        // igateTLM.RX++;
                    }
                }
            }
            else if (config.ext_tnc_mode == 3)
            { // YAESU FTM-350,FTM-400
                String info = "";
                if (config.ext_tnc_channel == 1)
                {
                    if (Serial0.available())
                        info = Serial0.readStringUntil(0x0D);
                }
                else if (config.ext_tnc_channel == 2)
                {
                    if (Serial1.available())
                        info = Serial1.readStringUntil(0x0D);
                }
#if SOC_UART_NUM > 2
                else if (config.ext_tnc_channel == 3)
                {
                    if (Serial2.available())
                        info = Serial2.readStringUntil(0x0D);
                }
#endif
                else if (config.ext_tnc_channel == 4)
                {
                    if (Serial.available())
                        info = Serial.readStringUntil(0x0D);
                }

                //  log_d("Ext Yaesu Packet >> %s",info.c_str());
                int ed = info.indexOf(" [");
                if (info != "" && ed > 10)
                {
                    raw.clear();
                    raw = info.substring(0, ed);
                    int st = info.indexOf(">:");
                    if (st > ed)
                    {
                        int idx = 0;
                        st += 2;
                        for (int i = 0; i < 5; i++)
                        {
                            if (info.charAt(st + i) == 0x0A || info.charAt(st + i) == 0x0D)
                            {
                                idx++;
                            }
                            else
                            {
                                break;
                            }
                        }
                        st += idx;
                        ed = info.indexOf(0x0D, st + 1);
                        if (ed > info.length())
                            ed = info.length();
                        if (ed > st)
                        {
                            raw += ":" + info.substring(st, ed);

                            String src_call = raw.substring(0, raw.indexOf('>'));
                            if ((src_call != "") && (src_call.length() < 11) && (raw.length() < sizeof(rawP)))
                            {
                                memset(call, 0, sizeof(call));
                                strlcpy(call, src_call.c_str(), sizeof(call));
                                memset(rawP, 0, sizeof(rawP));
                                strlcpy(rawP, raw.c_str(), sizeof(rawP));
                                log_d("Yaesu Packet: CallSign:%s RAW:%s", call, rawP);
                                // String hstr="";
                                // for(int i=0;i<raw.length();i++){
                                //     hstr+=" "+String(rawP[i],HEX);
                                // }
                                // log_d("HEX: %s",hstr.c_str());
                                uint16_t type = pkgType((const char *)rawP);
                                pkgListUpdate(call, rawP, type, 1, -1);
                                if (config.rf2inet && aprsClient.connected())
                                {
                                    // RF->INET
                                    aprsClient.write(&rawP[0], strlen(rawP)); // Send binary frame packet to APRS-IS (aprsc)
                                    aprsClient.write("\r\n");                 // Send CR LF the end frame packet
                                    status.rf2inet++;
                                    // igateTLM.RF2INET++;
                                    // igateTLM.RX++;
                                }
                            }
                        }
                    }
                }
            }
            //}
        }

        if (config.at_cmd_uart > 0)
        {
            // Read available bytes one at a time into the accumulator.
            // Dispatch to handleATCommand only when a full line (\n) is received.
            // This avoids readStringUntil() blocking the task while waiting for the
            // next character from a human typist, and prevents partial-line dispatch
            // if the inter-character gap exceeds the stream timeout.
            // AT_CMD_MAX_LEN caps accumulator growth; lines exceeding this are
            // silently discarded to prevent heap exhaustion from runaway input.
            static constexpr size_t AT_CMD_MAX_LEN = 256;

            auto processATChar = [](String &buf, char ch) -> String {
                if (ch == '\n' || ch == '\r')
                {
                    String cmd = buf;
                    buf.clear();
                    cmd.trim();
                    if (cmd.length() == 0) return "";
                    String ret = handleATCommand(cmd);
                    log_d("AT-Command response: %s", ret.c_str());
                    return ret;
                }
                else if (ch != '\r' && ch != '\n')
                {
                    if (buf.length() >= AT_CMD_MAX_LEN)
                    {
                        log_w("AT accumulator overflow, discarding line");
                        buf.clear();
                    }
                    else
                    {
                        buf += ch;
                    }
                }
                return "";
            };

            if (config.at_cmd_uart == 1)
            { // UART0
                while (Serial0.available()) {
                    String ret = processATChar(atBuf0, (char)Serial0.read());
                    if (ret != "") Serial0.println(ret);
                }
            }
            else if (config.at_cmd_uart == 2)
            { // UART1
                while (Serial1.available()) {
                    String ret = processATChar(atBuf1, (char)Serial1.read());
                    if (ret != "") Serial1.println(ret);
                }
            }
#if SOC_UART_NUM > 2
            else if (config.at_cmd_uart == 3)
            { // UART2
                while (Serial2.available()) {
                    String ret = processATChar(atBuf2, (char)Serial2.read());
                    if (ret != "") Serial2.println(ret);
                }
            }
#endif
            else if (config.at_cmd_uart == 4)
            { // USB-CDC
                while (Serial.available()) {
                    String ret = processATChar(atBuf3, (char)Serial.read());
                    if (ret != "") Serial.println(ret);
                }
            }
        }
    }
}
