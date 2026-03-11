#include "webservice.h"  // config, gps, webServiceBegin, handle_ws_gnss, WiFi, TimeLib, time.h, ESPAsyncWebServer
extern SemaphoreHandle_t gpsMutex;
#include <WiFiClient.h>

extern char nmea[100];
extern int nmea_idx;
extern unsigned long timerGPS, timerGPS_old;
extern bool firstGpsTime;
extern time_t startTime;
extern WiFiClient gnssClient;
extern unsigned long gnssTimeInterval;
extern AsyncWebSocket ws_gnss;

time_t getGpsTime();

void taskGPS(void *pvParameters)
{
    int c;
    log_d("GNSS Init");
    nmea_idx = 0;

    if (config.gnss_enable)
    {
        if ((config.gnss_channel > 0) && (config.gnss_channel < 4))
        {
            if (strstr("AT", config.gnss_at_command) != NULL)
            {
                if (config.gnss_channel == 1)
                {
                    Serial0.println(config.gnss_at_command);
                }
                else if (config.gnss_channel == 2)
                {
                    Serial1.println(config.gnss_at_command);
                }
                else if (config.gnss_channel == 3)
                {
                    // Serial2.println(config.gnss_at_command);
                }
            }
        }
    }
    for (;;)
    {
        timerGPS = micros() - timerGPS_old;
        vTaskDelay(10 / portTICK_PERIOD_MS);
        timerGPS_old = micros();

        if (config.gnss_enable)
        {
            if ((config.gnss_channel > 0) && (config.gnss_channel < 4))
            {
                do
                {
                    c = -1;
                    if (config.gnss_channel == 1)
                    {
                        if (Serial0.available()) c = Serial0.read();
                    }
                    else if (config.gnss_channel == 2)
                    {
                        if (Serial1.available()) c = Serial1.read();
                    }
#if SOC_UART_NUM > 2
                    else if (config.gnss_channel == 3)
                    {
                        if (Serial2.available()) c = Serial2.read();
                    }
#endif
                    if (c > -1)
                    {
                        xSemaphoreTake(gpsMutex, portMAX_DELAY);
                        gps.encode((char)c);
                        xSemaphoreGive(gpsMutex);
                        if (webServiceBegin == false)
                        {
                            if (nmea_idx > 99)
                            {
                                nmea_idx = 0;
                                memset(nmea, 0, sizeof(nmea));
                                // SerialGNSS->flush();
                            }
                            else
                            {
                                nmea[nmea_idx++] = (char)c;
                                if ((char)c == '\r' || (char)c == '\n')
                                {
                                    // nmea[nmea_idx] = 0;
                                    if (nmea_idx > 10)
                                    {
                                        // if (webServiceBegin == false)
                                        if (ws_gnss.enabled() && !ws_gnss.getClients().isEmpty())
                                        {
                                            // if (ws_gnss.availableForWriteAll())
                                            {
                                                handle_ws_gnss(nmea, nmea_idx);
                                            }
                                        }
                                        // log_d("[%d]:%s",nmea_idx,nmea);
                                    }
                                    nmea_idx = 0;
                                    memset(nmea, 0, sizeof(nmea));
                                    vTaskDelay(1 / portTICK_PERIOD_MS);
                                    break;
                                }
                            }
                        }
                        //}
                    }
                    else
                    {
                        break;
                    }
                } while (1);
            }
            else if (config.gnss_channel == 4)
            { // TCP
                if (WiFi.isConnected())
                {
                    if (!gnssClient.connected())
                    {
                        IPAddress ip;
                        ip.fromString(config.gnss_tcp_host);
                        gnssClient.connect(ip, config.gnss_tcp_port, 5000);
                        log_d("GNSS TCP ReConnect to %s:%d", config.gnss_tcp_host, config.gnss_tcp_port);
                        delay(5000);
                    }
                    else
                    {
                        while (gnssClient.available())
                        {
                            c = (char)gnssClient.read();
                            // Serial.print(c);
                            xSemaphoreTake(gpsMutex, portMAX_DELAY);
                            gps.encode(c);
                            xSemaphoreGive(gpsMutex);
                            if (webServiceBegin == false)
                            {
                                if (nmea_idx > 99)
                                {
                                    nmea_idx = 0;
                                    memset(nmea, 0, sizeof(nmea));
                                }
                                else
                                {
                                    nmea[nmea_idx++] = c;
                                    if (c == '\r' || c == '\n')
                                    {
                                        // nmea[nmea_idx] = 0;
                                        if (nmea_idx > 10)
                                        {
                                            // if (webServiceBegin == false)
                                            if (ws_gnss.enabled() && !ws_gnss.getClients().isEmpty())
                                            {
                                                handle_ws_gnss(nmea, nmea_idx);
                                            }
                                            // log_d("%s",nmea);
                                        }
                                        nmea_idx = 0;
                                        memset(nmea, 0, sizeof(nmea));
                                    }
                                }
                            }
                        }
                    }
                }
            }

            xSemaphoreTake(gpsMutex, portMAX_DELAY);
            if (gps.time.isValid())
            {
                if (gps.time.isUpdated())
                {
                    if (gnssTimeInterval > millis())
                    {
                        gnssTimeInterval = millis() + 10000;
                        time_t nowTime;
                        time_t timeGps = getGpsTime(); // Local gps time
                        time(&nowTime);
                        int tdiff = abs(timeGps - nowTime);
                        if (timeGps > 1700000000 && tdiff > 2) // && timeGps < 2347462800)
                        {
                            setTime(timeGps);
                            time_t rtc = timeGps - (config.timeZone * SECS_PER_HOUR);
                            timeval tv = {rtc, 0};
                            timezone tz = {static_cast<int>(config.timeZone * SECS_PER_HOUR), 0};
                            settimeofday(&tv, &tz);
                            log_d("\nSET GPS Timestamp = %u Year=%d\n", timeGps, year());
                            // firstGpsTime = false;
                            firstGpsTime = false;
                            if (startTime == 0)
                                startTime = timeGps;
                        }
                        // else
                        // {
                        //     startTime = 0;
                        // }
                    }
                }
            }
            xSemaphoreGive(gpsMutex);
        }
    }
}
