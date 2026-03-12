/*
 Name:		ESP32APRS_Audio
 Created:	13-10-2023 14:27:23
 Author:	HS5TQA/Atten
 Github:	https://github.com/nakhonthai
 Facebook:	https://www.facebook.com/atten
 Support IS: host:aprs.dprns.com port:14580 or aprs.hs5tqa.ampr.org:14580
 Support IS monitor: http://aprs.dprns.com:14501 or http://aprs.hs5tqa.ampr.org:14501
*/
#include <Arduino.h>
#include "webservice.h"
#include "pkg_list.h"
#include "base64.hpp"
#include "wireguard_vpn.h"
#include <LibAPRSesp.h>
#include <parse_aprs.h>
#include "jquery_min_js.h"
#include <ESPCPUTemp.h>
#include "esp_wifi.h"

#ifdef PPPOS
#include <PPP.h>
#endif

#ifdef SH1106
#include <Adafruit_SH1106.h>
#else
#include "Adafruit_SSD1306.h"
#endif // SH1106

#define SCREEN_ADDRESS 0x3C

AsyncWebServer async_server(80);
AsyncWebServer async_websocket(81);
char csrfToken[33] = {};
AsyncWebSocket ws("/ws");
AsyncWebSocket ws_gnss("/ws_gnss");

#ifdef MQTT
#include <PubSubClient.h>
extern PubSubClient clientMQTT;
#endif

#ifdef PPPOS
extern pppType pppStatus;
#endif

// Create an Event Source on /events
AsyncEventSource lastheard_events("/eventHeard");
AsyncEventSource message_events("/eventMsg");

String webString;

extern unsigned long waitISRetry;
extern int8_t adcEn;
extern int8_t dacEn;
extern unsigned long upTimeStamp;
extern double VBat;
extern bool VBat_Flag;
extern TaskHandle_t taskNetworkHandle;
extern TaskHandle_t taskAPRSHandle;
extern TaskHandle_t taskAPRSPollHandle;
extern TaskHandle_t taskSerialHandle;
extern TaskHandle_t taskGPSHandle;
extern TaskHandle_t taskSensorHandle;
#ifdef OLED
#ifdef SH1106
extern Adafruit_SH1106 display;
#else
extern Adafruit_SSD1306 display;
#endif
#endif // OLED

bool defaultSetting = false;

void serviceHandle()
{
	// server.handleClient();
}

void notFound(AsyncWebServerRequest *request)
{
	request->send(404, "text/plain", "Not found");
}

// Escape special HTML characters to prevent XSS when config values are
// embedded in HTML attribute values or page content.
String htmlEncode(const char *s)
{
	String out;
	for (; *s; s++) {
		switch (*s) {
			case '&':  out += "&amp;";  break;
			case '<':  out += "&lt;";   break;
			case '>':  out += "&gt;";   break;
			case '"':  out += "&quot;"; break;
			case '\'': out += "&#39;";  break;
			default:   out += *s;       break;
		}
	}
	return out;
}

void handle_logout(AsyncWebServerRequest *request)
{
	webString = "Log out";
	request->send(200, "text/html", webString);
}

void setMainPage(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	webString = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
	webString += "<meta name=\"robots\" content=\"index\" />\n";
	webString += "<meta name=\"robots\" content=\"follow\" />\n";
	webString += "<meta name=\"language\" content=\"English\" />\n";
	webString += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n";
	webString += "<meta name=\"GENERATOR\" content=\"configure 20230924\" />\n";
	webString += "<meta name=\"Author\" content=\"Mr.Somkiat Nakhonthai (HS5TQA)\" />\n";
	webString += "<meta name=\"Description\" content=\"Web Embedded Configuration\" />\n";
	webString += "<meta name=\"KeyWords\" content=\"ESP32,ESP32C3,AFSK,APRS\" />\n";
	webString += "<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\" />\n";
	webString += "<meta http-equiv=\"pragma\" content=\"no-cache\" />\n";
	webString += "<link rel=\"shortcut icon\" href=\"http://aprs.dprns.com/favicon.ico\" type=\"image/x-icon\" />\n";
	webString += "<meta http-equiv=\"Expires\" content=\"0\" />\n";
	if(strlen(config.host_name) > 0)
		webString += "<title>" + htmlEncode(config.host_name) + "</title>\n";
	else
		webString += "<title>ESP32APRS_Audio</title>\n";
	webString += "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />\n";
	webString += "<script src=\"/jquery-3.7.1.js\"></script>\n";
	webString += "<script type=\"text/javascript\">\n";
	webString += "function selectTab(evt, tabName) {\n";
	webString += "var i, tabcontent, tablinks;\n";
	webString += "tablinks = document.getElementsByClassName(\"nav-tabs\");\n";
	webString += "for (i = 0; i < tablinks.length; i++) {\n";
	webString += "tablinks[i].className = tablinks[i].className.replace(\" active\", \"\");\n";
	webString += "}\n";
	webString += "\n";
	webString += "//document.getElementById(tabName).style.display = \"block\";\n";
	webString += "if (tabName == 'DashBoard') {\n";
	webString += "$(\"#contentmain\").load(\"/dashboard\");\n";
	webString += "} else if (tabName == 'Radio') {\n";
	webString += "$(\"#contentmain\").load(\"/radio\");\n";
	webString += "} else if (tabName == 'IGATE') {\n";
	webString += "$(\"#contentmain\").load(\"/igate\");\n";
	webString += "} else if (tabName == 'DIGI') {\n";
	webString += "$(\"#contentmain\").load(\"/digi\");\n";
	webString += "} else if (tabName == 'TRACKER') {\n";
	webString += "$(\"#contentmain\").load(\"/tracker\");\n";
	webString += "} else if (tabName == 'WX') {\n";
	webString += "$(\"#contentmain\").load(\"/wx\");\n";
	webString += "} else if (tabName == 'TLM') {\n";
	webString += "$(\"#contentmain\").load(\"/tlm\");\n";
	webString += "} else if (tabName == 'SENSOR') {\n";
	webString += "$(\"#contentmain\").load(\"/sensor\");\n";
	webString += "} else if (tabName == 'VPN') {\n";
	webString += "$(\"#contentmain\").load(\"/vpn\");\n";
#ifdef MQTT
	webString += "} else if (tabName == 'MQTT') {\n";
	webString += "$(\"#contentmain\").load(\"/mqtt\");\n";
#endif
	webString += "} else if (tabName == 'MSG') {\n";
	webString += "$(\"#contentmain\").load(\"/msg\");\n";
	webString += "} else if (tabName == 'WiFi') {\n";
	webString += "$(\"#contentmain\").load(\"/wireless\");\n";
	webString += "} else if (tabName == 'MOD') {\n";
	webString += "$(\"#contentmain\").load(\"/mod\");\n";
	webString += "} else if (tabName == 'System') {\n";
	webString += "$(\"#contentmain\").load(\"/system\");\n";
	webString += "} else if (tabName == 'File') {\n";
	webString += "$(\"#contentmain\").load(\"/storage\");\n";
	webString += "} else if (tabName == 'About') {\n";
	webString += "$(\"#contentmain\").load(\"/about\");\n";
	webString += "}\n";
	webString += "\n";
	webString += "if (evt != null) evt.currentTarget.className += \" active\";\n";
	webString += "}\n";
	webString += "if (!!window.EventSource) {";
	webString += "var source = new EventSource('/eventHeard');";

	webString += "source.addEventListener('open', function(e) {";
	webString += "console.log(\"Events Connected\");";
	webString += "}, false);";
	webString += "source.addEventListener('error', function(e) {";
	webString += "if (e.target.readyState != EventSource.OPEN) {";
	webString += "console.log(\"Events Disconnected\");";
	webString += "}\n}, false);";
	webString += "source.addEventListener('lastHeard', function(e) {";
	// webString += "console.log(\"lastHeard\", e.data);";
	webString += "var lh=document.getElementById(\"lastHeard\");";
	webString += "if(lh != null) {lh.innerHTML = e.data;}";
	webString += "}, false);\n}";
	webString += "if (!!window.EventSource) {";
		webString += "var source = new EventSource('/eventMsg');";

		webString += "source.addEventListener('open', function(e) {";
		webString += "console.log(\"Events MSG Connected\");";
		webString += "}, false);";
		webString += "source.addEventListener('error', function(e) {";
		webString += "if (e.target.readyState != EventSource.OPEN) {";
		webString += "console.log(\"Events MSG Disconnected\");";
		webString += "}\n}, false);";
		webString += "source.addEventListener('chatMsg', function(e) {";
		// webString += "console.log(\"lastHeard\", e.data);";
		webString += "var lh=document.getElementById(\"chatMsg\");";
		webString += "if(lh != null) {lh.innerHTML = e.data;}";
		webString += "}, false);\n}";
	webString += "</script>\n";
	webString += "</head>\n";
	webString += "\n";
	webString += "<body onload=\"selectTab(event, 'DashBoard')\">\n";
	webString += "\n";
	webString += "<div class=\"container\">\n";
	webString += "<div class=\"header\">\n";
	// webString += "<div style=\"font-size: 8px; text-align: right; padding-right: 8px;\">ESP32IGate Firmware V" + String(VERSION) + "</div>\n";
	// webString += "<div style=\"font-size: 8px; text-align: right; padding-right: 8px;\"><a href=\"/logout\">[LOG OUT]</a></div>\n";
	if(strlen(config.host_name) > 0)
		webString += "<h1>" + htmlEncode(config.host_name) + "</h1>\n";
	else
		webString += "<h1>ESP32APRS_Audio</h1>\n";
	webString += "<div style=\"font-size: 8px; text-align: right; padding-right: 8px;\"><a href=\"/logout\">[LOG OUT]</a></div>\n";
	webString += "<div class=\"row\">\n";
	webString += "<ul class=\"nav nav-tabs\" style=\"margin: 5px;\">\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'DashBoard')\">DashBoard</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'Radio')\" id=\"btnRadio\">Radio</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'IGATE')\">IGATE</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'DIGI')\">DIGI</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'TRACKER')\">TRACKER</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'WX')\">WX</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'TLM')\">TLM</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'SENSOR')\">SENSOR</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'VPN')\">VPN</button>\n";
#ifdef MQTT
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'MQTT')\">MQTT</button>\n";
#endif
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'MSG')\">MSG</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'WiFi')\">WiFi</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'MOD')\">MOD</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'System')\">System</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'File')\">File</button>\n";
	webString += "<button class=\"nav-tabs\" onclick=\"selectTab(event, 'About')\">About</button>\n";
	webString += "</ul>\n";
	webString += "</div>\n";
	webString += "</div>\n";
	webString += "\n";

	webString += "<div class=\"contentwide\" id=\"contentmain\"  style=\"font-size: 2pt;\">\n";
	webString += "\n";
	webString += "</div>\n";
	webString += "<br />\n";
	webString += "<div class=\"footer\">\n";
	webString += "ESP32APRS_Audio Web Configuration<br />Copy right ©2023.\n";
	webString += "<br />\n";
	webString += "</div>\n";
	webString += "</div>\n";
	webString += "<!-- <script type=\"text/javascript\" src=\"/nice-select.min.js\"></script> -->\n";
	webString += "<script type=\"text/javascript\">\n";
	webString += "var selectize = document.querySelectorAll('select')\n";
	webString += "var options = { searchable: true };\n";
	webString += "selectize.forEach(function (select) {\n";
	webString += "if (select.length > 30 && null === select.onchange && !select.name.includes(\"ExtendedId\")) {\n";
	webString += "select.classList.add(\"small\", \"selectize\");\n";
	webString += "tabletd = select.closest('td');\n";
	webString += "tabletd.style.cssText = 'overflow-x:unset';\n";
	webString += "NiceSelect.bind(select, options);\n";
	webString += "}\n";
	webString += "});\n";
	webString += "</script>\n";
	webString += "</body>\n";
	webString += "</html>";
	request->send(200, "text/html", webString); // send to someones browser when asked
	lastHeardTimeout=0;
	lastHeard_Flag = true;
}

////////////////////////////////////////////////////////////
// handler for web server request: http://IpAddress/      //
////////////////////////////////////////////////////////////

// Forward declarations for handlers extracted to separate modules
void handle_radio(AsyncWebServerRequest *request);
void handle_vpn(AsyncWebServerRequest *request);
void handle_mqtt(AsyncWebServerRequest *request);
void handle_msg(AsyncWebServerRequest *request);
void handle_mod(AsyncWebServerRequest *request);
void handle_system(AsyncWebServerRequest *request);
void handle_igate(AsyncWebServerRequest *request);
void handle_digi(AsyncWebServerRequest *request);
void handle_wx(AsyncWebServerRequest *request);
void handle_tlm(AsyncWebServerRequest *request);
void handle_sensor(AsyncWebServerRequest *request);
void handle_tracker(AsyncWebServerRequest *request);
void handle_wireless(AsyncWebServerRequest *request);

void handle_css(AsyncWebServerRequest *request)
{
	const char *css = ".container{width:820px;text-align:left;margin:auto;border-radius:10px 10px 10px 10px;-moz-border-radius:10px 10px 10px 10px;-webkit-border-radius:10px 10px 10px 10px;-khtml-border-radius:10px 10px 10px 10px;-ms-border-radius:10px 10px 10px 10px;box-shadow:3px 3px 3px #707070;background:#fff;border-color: #2194ec;padding: 0px;border-width: 5px;border-style:solid;}body,font{font:12px verdana,arial,sans-serif;color:#fff}.header{background:#2194ec;text-decoration:none;color:#fff;font-family:verdana,arial,sans-serif;text-align:left;padding:5px 0;border-radius:10px 10px 0 0;-moz-border-radius:10px 10px 0 0;-webkit-border-radius:10px 10px 0 0;-khtml-border-radius:10px 10px 0 0;-ms-border-radius:10px 10px 0 0}.content{margin:0 0 0 166px;padding:1px 5px 5px;color:#000;background:#fff;text-align:center;font-size: 8pt;}.contentwide{padding:50px 5px 5px;color:#000;background:#fff;text-align:center}.contentwide h2{color:#000;font:1em verdana,arial,sans-serif;text-align:center;font-weight:700;padding:0;margin:0;font-size: 12pt;}.footer{background:#2194ec;text-decoration:none;color:#fff;font-family:verdana,arial,sans-serif;font-size:9px;text-align:center;padding:10px 0;border-radius:0 0 10px 10px;-moz-border-radius:0 0 10px 10px;-webkit-border-radius:0 0 10px 10px;-khtml-border-radius:0 0 10px 10px;-ms-border-radius:0 0 10px 10px;clear:both}#tail{height:450px;width:805px;overflow-y:scroll;overflow-x:scroll;color:#0f0;background:#000}table{vertical-align:middle;text-align:center;empty-cells:show;padding-left:3;padding-right:3;padding-top:3;padding-bottom:3;border-collapse:collapse;border-color:#0f07f2;border-style:solid;border-spacing:0px;border-width:3px;text-decoration:none;color:#fff;background:#000;font-family:verdana,arial,sans-serif;font-size : 12px;width:100%;white-space:nowrap}table th{font-size: 10pt;font-family:lucidia console,Monaco,monospace;text-shadow:1px 1px #0e038c;text-decoration:none;background:#0525f7;border:1px solid silver}table tr:nth-child(even){background:#f7f7f7}table tr:nth-child(odd){background:#eeeeee}table td{color:#000;font-family:lucidia console,Monaco,monospace;text-decoration:none;border:1px solid #010369}body{background:#edf0f5;color:#000}a{text-decoration:none}a:link,a:visited{text-decoration:none;color:#0000e0;font-weight:400}th:last-child a.tooltip:hover span{left:auto;right:0}ul{padding:5px;margin:10px 0;list-style:none;float:left}ul li{float:left;display:inline;margin:0 10px}ul li a{text-decoration:none;float:left;color:#999;cursor:pointer;font:900 14px/22px arial,Helvetica,sans-serif}ul li a span{margin:0 10px 0 -10px;padding:1px 8px 5px 18px;position:relative;float:left}h1{text-shadow:2px 2px #303030;text-align:center}.toggle{position:absolute;margin-left:-9999px;visibility:hidden}.toggle+label{display:block;position:relative;cursor:pointer;outline:none}input.toggle-round-flat+label{padding:1px;width:33px;height:18px;background-color:#ddd;border-radius:10px;transition:background .4s}input.toggle-round-flat+label:before,input.toggle-round-flat+label:after{display:block;position:absolute;}input.toggle-round-flat+label:before{top:1px;left:1px;bottom:1px;right:1px;background-color:#fff;border-radius:10px;transition:background .4s}input.toggle-round-flat+label:after{top:2px;left:2px;bottom:2px;width:16px;background-color:#ddd;border-radius:12px;transition:margin .4s,background .4s}input.toggle-round-flat:checked+label{background-color:#dd4b39}input.toggle-round-flat:checked+label:after{margin-left:14px;background-color:#dd4b39}@-moz-document url-prefix(){select,input{margin:0;padding:0;border-width:1px;font:12px verdana,arial,sans-serif}input[type=button],button,input[type=submit]{padding:0 3px;border-radius:3px 3px 3px 3px;-moz-border-radius:3px 3px 3px 3px}}.nice-select.small,.nice-select-dropdown li.option{height:24px!important;min-height:24px!important;line-height:24px!important}.nice-select.small ul li:nth-of-type(2){clear:both}.nav{margin-bottom:0;padding-left:10;list-style:none}.nav>li{position:relative;display:block}.nav>li>a{position:relative;display:block;padding:5px 10px}.nav>li>a:hover,.nav>li>a:focus{text-decoration:none;background-color:#eee}.nav>li.disabled>a{color:#999}.nav>li.disabled>a:hover,.nav>li.disabled>a:focus{color:#999;text-decoration:none;background-color:initial;cursor:not-allowed}.nav .open>a,.nav .open>a:hover,.nav .open>a:focus{background-color:#eee;border-color:#428bca}.nav .nav-divider{height:1px;margin:9px 0;overflow:hidden;background-color:#e5e5e5}.nav>li>a>img{max-width:none}.nav-tabs{border-bottom:1px solid #ddd}.nav-tabs>li{float:left;margin-bottom:-1px}.nav-tabs>li>a{margin-right:0;line-height:1.42857143;border:1px solid #ddd;border-radius:10px 10px 0 0}.nav-tabs>li>a:hover{border-color:#eee #eee #ddd}.nav-tabs>button{margin-right:0;line-height:1.42857143;border:2px solid #ddd;border-radius:10px 10px 0 0}.nav-tabs>button:hover{background-color:#25bbfc;border-color:#428bca;color:#eaf2f9;border-bottom-color:transparent;}.nav-tabs>button.active,.nav-tabs>button.active:hover,.nav-tabs>button.active:focus{color:#f7fdfd;background-color:#1aae0d;border:1px solid #ddd;border-bottom-color:transparent;cursor:default}.nav-tabs>li.active>a,.nav-tabs>li.active>a:hover,.nav-tabs>li.active>a:focus{color:#428bca;background-color:#e5e5e5;border:1px solid #ddd;border-bottom-color:transparent;cursor:default}.nav-tabs.nav-justified{width:100%;border-bottom:0}.nav-tabs.nav-justified>li{float:none}.nav-tabs.nav-justified>li>a{text-align:center;margin-bottom:5px}.nav-tabs.nav-justified>.dropdown .dropdown-menu{top:auto;left:auto}.nav-status{float:left;margin:0;padding:3px;width:160px;font-weight:400;min-height:600}#bar,#prgbar {background-color: #f1f1f1;border-radius: 14px}#bar {background-color: #3498db;width: 0%;height: 14px}.switch{position:relative;display:inline-block;width:34px;height:16px}.switch input{opacity:0;width:0;height:0}.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#f55959;-webkit-transition:.4s;transition:.4s}.slider:before{position:absolute;content:\"\";height:12px;width:12px;left:2px;bottom:2px;background-color:#fff;-webkit-transition:.4s;transition:.4s}input:checked+.slider{background-color:#5ca30a}input:focus+.slider{box-shadow:0 0 1px #5ca30a}input:checked+.slider:before{-webkit-transform:translateX(16px);-ms-transform:translateX(16px);transform:translateX(16px)}.slider.round{border-radius:34px}.slider.round:before{border-radius:50%}.button{border:1px solid #06c;background-color:#09c;color:#fff;padding:5px 10px;border-radius: 3px}.button:hover{border:1px solid #09c;background-color:#0ac;color:#fff}.button:disabled,button[disabled]{border:1px solid #999;background-color:#ccc;color:#666}\n";
	request->send_P(200, "text/css", css);
}

void handle_jquery(AsyncWebServerRequest *request)
{
	#if defined(CONFIG_IDF_TARGET_ESP32)
	adcEn=-1;
	dacEn=-1;
	delay(100);
	#endif
	AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("application/javascript")), (const uint8_t *)jquery_3_7_1_min_js_gz, jquery_3_7_1_min_js_gz_len);
	response->addHeader(String(F("Content-Encoding")), String(F("gzip")));
	response->setContentLength(jquery_3_7_1_min_js_gz_len);
	request->send(response);
	#if defined(CONFIG_IDF_TARGET_ESP32)
	delay(200);
	adcEn=1;
	dacEn=0;
	#endif
}

void handle_dashboard(AsyncWebServerRequest *request)
{
	// if (!request->authenticate(config.http_username, config.http_password))
	// {
	// 	return request->requestAuthentication();
	// }
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);
	webString = "<script type=\"text/javascript\">\n";
	webString += "function reloadSysInfo() {\n";
	webString += "$(\"#sysInfo\").load(\"/sysinfo\", function () { setTimeout(reloadSysInfo, 60000) });\n";
	webString += "}\n";
	webString += "setTimeout(reloadSysInfo(), 100);\n";
	webString += "function reloadSidebarInfo() {\n";
	webString += "$(\"#sidebarInfo\").load(\"/sidebarInfo\", function () { setTimeout(reloadSidebarInfo, 10000) });\n";
	webString += "}\n";
	webString += "setTimeout(reloadSidebarInfo, 1000);\n";
	webString += "$(window).trigger('resize');\n";

	webString += "</script>\n";

	webString += "<div id=\"sysInfo\">\n";
	webString += "</div>\n";

	webString += "<br />\n";
	webString += "<div class=\"nav-status\">\n";
	webString += "<div id=\"sidebarInfo\">\n";
	webString += "</div>\n";
	webString += "<br />\n";

	webString += "<table>\n";
	webString += "<tr>\n";
	webString += "<th colspan=\"2\">Radio Info</th>\n";
	webString += "</tr>\n";
	if(config.rf_en){
		webString += "<tr>\n";
		webString += "<td>Freq.TX</td>\n";
		webString += "<td style=\"background: #ffffff;\">" + String(config.freq_tx, 4) + " MHz</td>\n";
		webString += "</tr>\n";
		webString += "<tr>\n";
		webString += "<td>Freq.RX</td>\n";
		webString += "<td style=\"background: #ffffff;\">" + String(config.freq_rx, 4) + " MHz</td>\n";
		webString += "</tr>\n";	
		webString += "<tr>\n";
		webString += "<td>TX PWR</td>\n";
		if (config.rf_power)
			webString += "<td>HIGH</td>\n";
		else
			webString += "<td>LOW</td>\n";
		webString += "</tr>\n";		
	}
	webString += "<tr>\n";
	webString += "<td>MODEM</td>\n";
	webString += "<td style=\"background: #ffffff;\">" + String(MODEM_TYPE[config.modem_type]) + "</td>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "<td>FX.25</td>\n";
	webString += "<td style=\"background: #ffffff;\">" + String(FX25_MODE[config.fx25_mode]) + "</td>\n";
	webString += "</tr>\n";
	webString += "</table>\n";
	webString += "\n";
	if (config.igate_en)
	{
		webString += "<br />\n";
		webString += "<table>\n";
		webString += "<tr>\n";
		webString += "<th colspan=\"2\">APRS-IS SERVER</th>\n";
		webString += "</tr>\n";
		webString += "<tr>\n";
		webString += "<td>HOST</td>\n";
		webString += "<td style=\"background: #ffffff;\">" + htmlEncode(config.aprs_host) + "</td>\n";
		webString += "</tr>\n";
		webString += "<tr>\n";
		webString += "<td>PORT</td>\n";
		webString += "<td style=\"background: #ffffff;\">" + String(config.aprs_port) + "</td>\n";
		webString += "</tr>\n";
		webString += "</table>\n";
	}
	webString += "<br />\n";
	webString += "<table>\n";
	webString += "<tr>\n";
	webString += "<th colspan=\"2\">WiFi</th>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "<td>MODE</td>\n";
	String strWiFiMode = "OFF";
	if (config.wifi_mode == WIFI_STA_FIX)
	{
		strWiFiMode = "STA";
	}
	else if (config.wifi_mode == WIFI_AP_FIX)
	{
		strWiFiMode = "AP";
	}
	else if (config.wifi_mode == WIFI_AP_STA_FIX)
	{
		strWiFiMode = "AP+STA";
	}
	webString += "<td style=\"background: #ffffff;\">" + strWiFiMode + "</td>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "<td>SSID</td>\n";
	webString += "<td style=\"background: #ffffff;\">" + String(WiFi.SSID()) + "</td>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "<td>RSSI</td>\n";
	if (WiFi.isConnected())
		webString += "<td style=\"background: #ffffff;\">" + String(WiFi.RSSI()) + " dBm</td>\n";
	else
		webString += "<td style=\"background:#606060; color:#b0b0b0;\" aria-disabled=\"true\">Disconnect</td>\n";
	webString += "</tr>\n";
	webString += "</table>\n";
	webString += "<br />\n";
#ifdef BLUETOOTH
	webString += "<table>\n";
	webString += "<tr>\n";

	webString += "<th colspan=\"2\">Bluetooth</th>\n";
	webString += "</tr>\n";
	webString += "<td>Master</td>\n";
	if (config.bt_master)
		webString += "<td style=\"background:#0b0; color:#030; width:50%;\">Enabled</td>\n";
	else
		webString += "<td style=\"background:#606060; color:#b0b0b0;\" aria-disabled=\"true\">Disabled</td>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "<tr>\n";
	webString += "<td>NAME</td>\n";
	webString += "<td style=\"background: #ffffff;\">" + htmlEncode(config.bt_name) + "</td>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "<tr>\n";
	webString += "<td>MODE</td>\n";
	String btMode = "";
	if (config.bt_mode == 1)
	{
		btMode = "TNC2";
	}
	else if (config.bt_mode == 2)
	{
		btMode = "KISS";
	}
	else
	{
		btMode = "NONE";
	}
	webString += "<td style=\"background: #ffffff;\">" + btMode + "</td>\n";
	webString += "</tr>\n";
	webString += "<tr>\n";
	webString += "</table>\n";
#endif
	webString += "</div>\n";

	webString += "</div>\n";
	webString += "\n";
	webString += "<div class=\"content\">\n";
	webString += "<div id=\"lastHeard\">\n";
	webString += event_lastHeard(true);
	webString += "</div>\n";

	request->send(200, "text/html", webString); // send to someones browser when asked
	lastHeardTimeout=0;
	lastHeard_Flag = true;
}

void handle_sidebar(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	String html = "<table style=\"background:white;border-collapse: unset;\">\n";
	html += "<tr>\n";
	html += "<th colspan=\"2\">Modes Enabled</th>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	if (config.igate_en)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">IGATE</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\">IGATE</th>\n";

	if (config.digi_en)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">DIGI</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\">DIGI</th>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	if (config.wx_en)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">WX</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\">WX</th>\n";
	if (config.trk_en)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">TRACKER</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\">TRACKER</th>\n";
	html += "</tr>\n";
	html += "</table>\n";
	html += "<br />\n";
	html += "<table style=\"background:white;border-collapse: unset;\">\n";
	html += "<tr>\n";
	html += "<th colspan=\"2\">Network Status</th>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	if (aprsClient.connected() == true)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">APRS-IS</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\" aria-disabled=\"true\">APRS-IS</th>\n";
	if (wireguard_active() == true)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">VPN</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\" aria-disabled=\"true\">VPN</th>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	//html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\" aria-disabled=\"true\">4G LTE</th>\n";
	#ifdef PPPOS
	if (PPP.connected())
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">PPPoS</th>\n";
	else
	#endif
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\" aria-disabled=\"true\">PPPoS</th>\n";
	if(config.fx25_mode>0)
		html += "<th style=\"background:#0b0; color:#030; width:50%;border-radius: 10px;border: 2px solid white;\">FX.25</th>\n";
	else
		html += "<th style=\"background:#606060; color:#b0b0b0;border-radius: 10px;border: 2px solid white;\" aria-disabled=\"true\">FX.25</th>\n";
	html += "</tr>\n";
	html += "</table>\n";
	html += "<br />\n";
	html += "<table>\n";
	html += "<tr>\n";
	html += "<th colspan=\"2\">STATISTICS</th>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	xSemaphoreTake(statusMutex, portMAX_DELAY);
	html += "<td style=\"width: 60px;text-align: right;\">RADIO RX:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.rxCount) + "</td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td style=\"width: 60px;text-align: right;\">PACKET RX:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.allCount) + "</td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td style=\"width: 60px;text-align: right;\">PACKET TX:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.txCount) + "</td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td style=\"width: 60px;text-align: right;\">RF2INET:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.rf2inet) + "</td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td style=\"width: 60px;text-align: right;\">INET2RF:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.inet2rf) + "</td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td style=\"width: 60px;text-align: right;\">DIGI:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.digiCount) + "</td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td style=\"width: 60px;text-align: right;\">DROP/ERR:</td>\n";
	html += "<td style=\"background: #ffffff;\">" + String(status.dropCount) + "/" + String(status.errorCount) + "</td>\n";
	html += "</tr>\n";
	xSemaphoreGive(statusMutex);
	html += "</table>\n";
	html += "<br />\n";
	if (config.gnss_enable)
	{
		html += "<table>\n";
		html += "<tr>\n";
		html += "<th colspan=\"2\">GPS Info <a href=\"/gnss\" target=\"_gnss\" style=\"color: yellow;font-size:8pt\">[View]</a></th>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td>LAT:</td>\n";
		html += "<td style=\"background: #ffffff;text-align: left;\">" + String(gps.location.lat(), 5) + "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td>LON:</td>\n";
		html += "<td style=\"background: #ffffff;text-align: left;\">" + String(gps.location.lng(), 5) + "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td>ALT:</td>\n";
		html += "<td style=\"background: #ffffff;text-align: left;\">" + String(gps.altitude.meters(), 1) + "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td>SAT:</td>\n";
		html += "<td style=\"background: #ffffff;text-align: left;\">" + String(gps.satellites.value()) + "</td>\n";
		html += "</tr>\n";
		html += "</table>\n";
	}
	html += "<script>\n";
	html += "$(window).trigger('resize');\n";
	html += "</script>\n";
	// request->send(200, "text/html", html); // send to someones browser when asked
	// delay(100);
	request->send(200, "text/html", html);
	html.clear();
}

void handle_symbol(AsyncWebServerRequest *request)
{
	int i;
	int sel = -1;
	for (i = 0; i < request->args(); i++)
	{
		if (request->argName(i) == "sel")
		{
			if (request->arg(i) != "")
			{
				if (isValidNumber(request->arg(i)))
				{
					sel = request->arg(i).toInt();
				}
			}
		}
	}

	char *web = (char *)calloc(22000, sizeof(char));
	if (web)
	{
		memset(web, 0, 22000);
		strcat(web, "<table border=\"1\" align=\"center\">\n");
		strcat(web, "<tr><th colspan=\"16\">Table '/'</th></tr>\n");
		strcat(web, "<tr>\n");
		char lnk[200];
		for (i = 33; i < 129; i++)
		{
			memset(lnk,0,sizeof(lnk));
			//<td><img onclick="window.opener.setValue(113,2);" src="http://aprs.dprns.com/symbols/icons/113-2.png"></td>			
			if (sel == -1)
				sprintf(lnk, "<td><img onclick=\"window.opener.setValue(%d,1);\" src=\"http://aprs.dprns.com/symbols/icons/%d-1.png\"></td>", i, i);
			else
				sprintf(lnk, "<td><img onclick=\"window.opener.setValue(%d,%d,1);\" src=\"http://aprs.dprns.com/symbols/icons/%d-1.png\"></td>", sel, i, i);
			strcat(web, lnk);

			if (((i % 16) == 0) && (i < 126))
				strcat(web, "</tr>\n<tr>\n");
		}
		strcat(web, "</tr>");
		strcat(web, "</table>\n<br />");
		strcat(web, "<table border=\"1\" align=\"center\">\n");
		strcat(web, "<tr><th colspan=\"16\">Table '\\'</th></tr>\n");
		strcat(web, "<tr>\n");
		for (i = 33; i < 129; i++)
		{
			memset(lnk,0,sizeof(lnk));
			if (sel == -1)
				sprintf(lnk, "<td><img onclick=\"window.opener.setValue(%d,2);\" src=\"http://aprs.dprns.com/symbols/icons/%d-2.png\"></td>", i, i);
			else
				sprintf(lnk, "<td><img onclick=\"window.opener.setValue(%d,%d,2);\" src=\"http://aprs.dprns.com/symbols/icons/%d-2.png\"></td>", sel, i, i);
			strcat(web, lnk);
			if (((i % 16) == 0) && (i < 126))
				strcat(web, "</tr>\n<tr>\n");
		}
		strcat(web, "</tr>");
		strcat(web, "</table>\n");
		request->send_P(200, "text/html", web);
		free(web);
	}
}

void handle_sysinfo(AsyncWebServerRequest *request)
{
	String html = "<table style=\"table-layout: fixed;border-collapse: unset;border-radius: 10px;border-color: #ee800a;border-style: ridge;border-spacing: 1px;border-width: 4px;background: #ee800a;\">\n";
	html += "<tr>\n";
	html += "<th><span><b>Up Time</b></span></th>\n";
	html += "<th><span>RAM(KByte)</span></th>\n";
	#ifdef BOARD_HAS_PSRAM
	html += "<th><span>PSRAM(KByte)</span></th>\n";
	#endif
	html += "<th><span>SPIFFS(KByte)</span></th>\n";
	if(VBat_Flag)
		html += "<th><span>VBat(V)</span></th>\n";
	html += "<th><span>CPU(Mhz)</span></th>\n";
	html += "<th><span>CPU.Temp(°C)</span></th>\n";

	html += "</tr>\n";
	html += "<tr>\n";
	//time_t tn = time(NULL) - systemUptime;
	// String uptime = String(day(tn) - 1, DEC) + "D " + String(hour(tn), DEC) + ":" + String(minute(tn), DEC) + ":" + String(second(tn), DEC);
	//String uptime = String(day(tn) - 1, DEC) + "D " + String(hour(tn), DEC) + ":" + String(minute(tn), DEC);
	char strTime[20];
	convertSecondsToDHMS(strTime,(millis()/1000)-upTimeStamp);
	html += "<td><b>" + String(strTime) + "</b></td>\n";
	html += "<td><b>" + String((float)ESP.getFreeHeap() / 1000, 1) + "/" + String((float)ESP.getHeapSize() / 1000, 1) + "</b></td>\n";
	#ifdef BOARD_HAS_PSRAM
	html += "<td><b>" + String((float)ESP.getFreePsram() / 1000, 1) + "/" + String((float)ESP.getPsramSize() / 1000, 1) + "</b></td>\n";
	#endif
	unsigned long cardTotal = LITTLEFS.totalBytes();
	unsigned long cardUsed = LITTLEFS.usedBytes();
	html += "<td><b>" + String((double)cardUsed / 1024, 1) + "/" + String((double)cardTotal / 1024, 1) + "</b></td>\n";
	if(VBat_Flag)
		html += "<td><b>" + String(VBat, 2) + "</b></td>\n";
	html += "<td><b>" + String(ESP.getCpuFreqMHz()) + "</b></td>\n";
	ESPCPUTemp tempSensor;
	if (tempSensor.begin()) {
		html += "<td><b>" + String(tempSensor.getTemp(), 1) + "</b></td>\n";
	}else{
		html += "<td><b>N/A</b></td>\n";
	}
	// html += "<td style=\"background: #f00\"><b>" + String(ESP.getCycleCount()) + "</b></td>\n";
	html += "</tr>\n";
	html += "<tr><td colspan=\"7\"><b>Stack HWM (words)</b></td></tr>\n";
	html += "<tr>";
	html += "<td>Net:"  + String(taskNetworkHandle  ? uxTaskGetStackHighWaterMark(taskNetworkHandle)  : 0) + "</td>";
	html += "<td>APRS:" + String(taskAPRSHandle     ? uxTaskGetStackHighWaterMark(taskAPRSHandle)     : 0) + "</td>";
	html += "<td>Poll:" + String(taskAPRSPollHandle ? uxTaskGetStackHighWaterMark(taskAPRSPollHandle) : 0) + "</td>";
	html += "<td>GPS:"  + String(taskGPSHandle      ? uxTaskGetStackHighWaterMark(taskGPSHandle)      : 0) + "</td>";
	html += "<td>Ser:"  + String(taskSerialHandle   ? uxTaskGetStackHighWaterMark(taskSerialHandle)   : 0) + "</td>";
	html += "<td>Sns:"  + String(taskSensorHandle   ? uxTaskGetStackHighWaterMark(taskSensorHandle)   : 0) + "</td>";
	html += "</tr>\n";
	html += "</table>\n";
	request->send(200, "text/html", html); // send to someones browser when asked
	html.clear();
}

// void handle_lastHeard(AsyncWebServerRequest *request)
// {
// 	struct pbuf_t aprs;
// 	ParseAPRS aprsParse;
// 	struct tm tmstruct;
// 	String html = "";
// 	sort(pkgList, PKGLISTSIZE);

// 	html = "<table>\n";
// 	html += "<th colspan=\"7\" style=\"background-color: #070ac2;\">LAST HEARD <a href=\"/tnc2\" target=\"_tnc2\" style=\"color: yellow;font-size:8pt\">[RAW]</a></th>\n";
// 	html += "<tr>\n";
// 	html += "<th style=\"min-width:10ch\"><span><b>Time (";
// 	if (config.timeZone >= 0)
// 		html += "+";
// 	// else
// 	//	html += "-";

// 	if (config.timeZone == (int)config.timeZone)
// 		html += String((int)config.timeZone) + ")</b></span></th>\n";
// 	else
// 		html += String(config.timeZone, 1) + ")</b></span></th>\n";
// 	html += "<th style=\"min-width:16px\">ICON</th>\n";
// 	html += "<th style=\"min-width:10ch\">Callsign</th>\n";
// 	html += "<th>VIA LAST PATH</th>\n";
// 	html += "<th style=\"min-width:5ch\">DX</th>\n";
// 	html += "<th style=\"min-width:5ch\">PACKET</th>\n";
// 	html += "<th style=\"min-width:5ch\">AUDIO</th>\n";
// 	html += "</tr>\n";

// 	for (int i = 0; i < PKGLISTSIZE; i++)
// 	{
// 		if (i >= PKGLISTSIZE)
// 			break;
// 		pkgListType pkg = getPkgList(i);
// 		if (pkg.time > 0)
// 		{
// 			String line = String(pkg.raw);
// 			int packet = pkg.pkg;
// 			int start_val = line.indexOf(">", 0); // หาตำแหน่งแรกของ >
// 			if (start_val > 3)
// 			{
// 				String src_call = line.substring(0, start_val);
// 				memset(&aprs, 0, sizeof(pbuf_t));
// 				aprs.buf_len = 300;
// 				aprs.packet_len = line.length();
// 				line.toCharArray(&aprs.data[0], aprs.packet_len);
// 				int start_info = line.indexOf(":", 0);
// 				int end_ssid = line.indexOf(",", 0);
// 				int start_dst = line.indexOf(">", 2);
// 				int start_dstssid = line.indexOf("-", start_dst);
// 				String path = "";

// 				if ((end_ssid > start_dst) && (end_ssid < start_info))
// 				{
// 					path = line.substring(end_ssid + 1, start_info);
// 				}
// 				if (end_ssid < 5)
// 					end_ssid = start_info;
// 				if ((start_dstssid > start_dst) && (start_dstssid < start_dst + 10))
// 				{
// 					aprs.dstcall_end_or_ssid = &aprs.data[start_dstssid];
// 				}
// 				else
// 				{
// 					aprs.dstcall_end_or_ssid = &aprs.data[end_ssid];
// 				}
// 				aprs.info_start = &aprs.data[start_info + 1];
// 				aprs.dstname = &aprs.data[start_dst + 1];
// 				aprs.dstname_len = end_ssid - start_dst;
// 				aprs.dstcall_end = &aprs.data[end_ssid];
// 				aprs.srccall_end = &aprs.data[start_dst];

// 				// Serial.println(aprs.info_start);
// 				if (aprsParse.parse_aprs(&aprs))
// 				{
// 					pkg.calsign[10] = 0;
// 					// time_t tm = pkg.time;
// 					localtime_r(&pkg.time, &tmstruct);
// 					char strTime[10];
// 					sprintf(strTime, "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
// 					// String str = String(tmstruct.tm_hour, DEC) + ":" + String(tmstruct.tm_min, DEC) + ":" + String(tmstruct.tm_sec, DEC);

// 					html += "<tr><td>" + String(strTime) + "</td>";
// 					String fileImg = "";
// 					uint8_t sym = (uint8_t)aprs.symbol[1];
// 					if (sym > 31 && sym < 127)
// 					{
// 						if (aprs.symbol[0] > 64 && aprs.symbol[0] < 91) // table A-Z
// 						{
// 							html += "<td><b>" + String(aprs.symbol[0]) + "</b></td>";
// 						}
// 						else
// 						{
// 							fileImg = String(sym, DEC);
// 							if (aprs.symbol[0] == 92)
// 							{
// 								fileImg += "-2.png";
// 							}
// 							else if (aprs.symbol[0] == 47)
// 							{
// 								fileImg += "-1.png";
// 							}
// 							else
// 							{
// 								fileImg = "dot.png";
// 							}
// 							html += "<td><img src=\"http://aprs.dprns.com/symbols/icons/" + fileImg + "\"></td>";
// 						}
// 					}
// 					else
// 					{
// 						html += "<td><img src=\"http://aprs.dprns.com/symbols/icons/dot.png\"></td>";
// 					}
// 					html += "<td>" + src_call;
// 					if (aprs.srcname_len > 0 && aprs.srcname_len < 10) // Get Item/Object
// 					{
// 						char itemname[10];
// 						memset(&itemname, 0, sizeof(itemname));
// 						memcpy(&itemname, aprs.srcname, aprs.srcname_len);
// 						html += "(" + String(itemname) + ")";
// 					}
// 					html += +"</td>";
// 					if (path == "")
// 					{
// 						html += "<td style=\"text-align: left;\">RF: DIRECT</td>";
// 					}
// 					else
// 					{
// 						String LPath = path.substring(path.lastIndexOf(',') + 1);
// 						// if(path.indexOf("qAR")>=0 || path.indexOf("qAS")>=0 || path.indexOf("qAC")>=0){ //Via from Internet Server
// 						if (path.indexOf("qA") >= 0 || path.indexOf("TCPIP") >= 0)
// 						{
// 							html += "<td style=\"text-align: left;\">INET: " + LPath + "</td>";
// 						}
// 						else
// 						{
// 							if (path.indexOf("*") > 0)
// 							{
// 								html += "<td style=\"text-align: left;\">DIGI: " + path + "</td>";
// 							}
// 							else
// 							{
// 								html += "<td style=\"text-align: left;\">RF: " + path + "</td>";
// 							}
// 						}
// 					}
// 					// html += "<td>" + path + "</td>";
// 					if (aprs.flags & F_HASPOS)
// 					{
// 						double lat, lon;
// 						if (gps.location.isValid())
// 						{
// 							lat = gps.location.lat();
// 							lon = gps.location.lng();
// 						}
// 						else
// 						{
// 							lat = config.igate_lat;
// 							lon = config.igate_lon;
// 						}
// 						double dtmp = aprsParse.direction(lon, lat, aprs.lng, aprs.lat);
// 						double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
// 						html += "<td>" + String(dist, 1) + "km/" + String(dtmp, 0) + "°</td>";
// 					}
// 					else
// 					{
// 						html += "<td>-</td>\n";
// 					}
// 					html += "<td>" + String(packet) + "</td>\n";
// 					if (pkg.audio_level == 0)
// 					{
// 						html += "<td>-</td></tr>\n";
// 					}
// 					else
// 					{
// 						double Vrms = (double)pkg.audio_level / 1000;
// 						double audBV = 20.0F * log10(Vrms);
// 						if (audBV < -20.0F)
// 						{
// 							html += "<td style=\"color: #0000f0;\">";
// 						}
// 						else if (audBV > -5.0F)
// 						{
// 							html += "<td style=\"color: #f00000;\">";
// 						}
// 						else
// 						{
// 							html += "<td style=\"color: #008000;\">";
// 						}
// 						html += String(audBV, 1) + "dBV</td></tr>\n";
// 					}
// 				}
// 			}
// 		}
// 	}
// 	html += "</table>\n";
// 	if ((ESP.getFreeHeap() / 1000) > 120)
// 	{
// 		request->send(200, "text/html", html); // send to someones browser when asked
// 	}
// 	else
// 	{
// 		size_t len = html.length();
// 		char *info = (char *)calloc(len, sizeof(char));
// 		if (info)
// 		{

// 			html.toCharArray(info, len, 0);
// 			html.clear();
// 			AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)info, len);

// 			response->addHeader("Sensor", "content");
// 			request->send(response);
// 			free(info);
// 		}
// 		else
// 		{
// 			log_d("Can't define calloc info size %d", len);
// 		}
// 	}
// 	// request->send(200, "text/html", html); // send to someones browser when asked
// 	// delay(100);
// 	// html.clear();
// }

String event_lastHeard(bool gethtml)
{
	// log_d("Event count: %d",lastheard_events.count());
	//if (lastheard_events.count() == 0)
	//	return;

	struct pbuf_t aprs;
	ParseAPRS aprsParse;
	struct tm tmstruct,tmNow;

	// adcEn=-1;
	// dacEn=-1;
	// delay(20);

	String html = "";
	String line = "";
	//sort(pkgList, PKGLISTSIZE);
	time_t timeNow = time(NULL);

	// log_d("Create html last heard");
	localtime_r(&timeNow, &tmNow);

	html = "<table>\n";
	html += "<th colspan=\"7\" style=\"background-color: #070ac2;\">LAST HEARD <a href=\"/tnc2\" target=\"_tnc2\" style=\"color: yellow;font-size:8pt\">[RAW]</a></th>\n";
	html += "<tr>\n";
	html += "<th style=\"min-width:10ch\"><span><b>Time (";
	if (config.timeZone >= 0)
		html += "+";
	// else
	//	html += "-";

	if (config.timeZone == (int)config.timeZone)
		html += String((int)config.timeZone) + ")</b></span></th>\n";
	else
		html += String(config.timeZone, 1) + ")</b></span></th>\n";
	html += "<th style=\"min-width:16px\">ICON</th>\n";
	html += "<th style=\"min-width:10ch\">Callsign</th>\n";
	html += "<th>VIA LAST PATH</th>\n";
	html += "<th style=\"min-width:5ch\">DX</th>\n";
	html += "<th style=\"min-width:5ch\">PACKET</th>\n";
	html += "<th style=\"min-width:5ch\">AUDIO</th>\n";
	html += "</tr>\n";

	for (int i = 0; i < PKGLISTSIZE; i++)
	{
		pkgListType pkg;
		xSemaphoreTakeRecursive(pkgListMutex, portMAX_DELAY);
		memset(&pkg, 0, sizeof(pkgListType));
		memcpy(&pkg, &pkgList[i], sizeof(pkgListType));
		line = pkg.raw ? String(pkg.raw) : String();
		pkg.raw = nullptr;
		xSemaphoreGiveRecursive(pkgListMutex);
		if (pkg.time > 0)
		{
			// log_d("IDX=%d RAW:%s",i,line.c_str());
			int packet = pkg.pkg;
			int start_val = line.indexOf(">", 0); // หาตำแหน่งแรกของ >
			if (start_val > 3)
			{
				String src_call = line.substring(0, start_val);
				memset(&aprs, 0, sizeof(pbuf_t));
				aprs.buf_len = 300;
				aprs.packet_len = line.length();
				line.toCharArray(&aprs.data[0], aprs.packet_len);
				int start_info = line.indexOf(":", 0);
				int end_ssid = line.indexOf(",", 0);
				int start_dst = line.indexOf(">", 2);
				int start_dstssid = line.indexOf("-", start_dst);
				String path = "";

				if ((end_ssid > start_dst) && (end_ssid < start_info))
				{
					path = line.substring(end_ssid + 1, start_info);
				}
				if (end_ssid < 5)
					end_ssid = start_info;
				if ((start_dstssid > start_dst) && (start_dstssid < start_dst + 10))
				{
					aprs.dstcall_end_or_ssid = &aprs.data[start_dstssid];
				}
				else
				{
					aprs.dstcall_end_or_ssid = &aprs.data[end_ssid];
				}
				aprs.info_start = &aprs.data[start_info + 1];
				aprs.dstname = &aprs.data[start_dst + 1];
				aprs.dstname_len = end_ssid - start_dst;
				aprs.dstcall_end = &aprs.data[end_ssid];
				aprs.srccall_end = &aprs.data[start_dst];

				// Serial.println(aprs.info_start);
				if (aprsParse.parse_aprs(&aprs))
				{
					pkg.calsign[10] = 0;
					// time_t tm = pkg.time;
					localtime_r(&pkg.time, &tmstruct);
					char strTime[10];
					if(tmNow.tm_mday==tmstruct.tm_mday)
						sprintf(strTime, "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
					else	
						sprintf(strTime, "%02dD %02d:%02d", tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min);
					// String str = String(tmstruct.tm_hour, DEC) + ":" + String(tmstruct.tm_min, DEC) + ":" + String(tmstruct.tm_sec, DEC);

					html += "<tr><td>" + String(strTime) + "</td>";
					String fileImg = "";
					uint8_t sym = (uint8_t)aprs.symbol[1];
					if (sym > 31 && sym < 127)
					{
						if (aprs.symbol[0] > 64 && aprs.symbol[0] < 91) // table A-Z
						{
							html += "<td><b>" + String(aprs.symbol[0]) + "</b></td>";
						}
						else
						{
							fileImg = String(sym, DEC);
							if (aprs.symbol[0] == 92)
							{
								fileImg += "-2.png";
							}
							else if (aprs.symbol[0] == 47)
							{
								fileImg += "-1.png";
							}
							else
							{
								fileImg = "dot.png";
							}
							html += "<td><img src=\"http://aprs.dprns.com/symbols/icons/" + fileImg + "\"></td>";
						}
						fileImg.clear();
					}
					else
					{
						html += "<td><img src=\"http://aprs.dprns.com/symbols/icons/dot.png\"></td>";
					}
					html += "<td>" + src_call;
					if (aprs.srcname_len > 0 && aprs.srcname_len < 10) // Get Item/Object
					{
						char itemname[10];
						memset(&itemname, 0, 10);
						memcpy(&itemname, aprs.srcname, aprs.srcname_len);
						html += "(" + String(itemname) + ")";
					}
					html += +"</td>";
					if (path == "")
					{
						html += "<td style=\"text-align: left;\">RF: DIRECT</td>";
					}
					else
					{
						String LPath = path.substring(path.lastIndexOf(',') + 1);
						// if(path.indexOf("qAR")>=0 || path.indexOf("qAS")>=0 || path.indexOf("qAC")>=0){ //Via from Internet Server
						if (path.indexOf("qA") >= 0 || path.indexOf("TCPIP") >= 0)
						{
							html += "<td style=\"text-align: left;\">INET: " + LPath + "</td>";
						}
						else
						{
							if (path.indexOf("*") > 0)
								html += "<td style=\"text-align: left;\">DIGI: " + path + "</td>";
							else
								html += "<td style=\"text-align: left;\">RF: " + path + "</td>";
						}
						LPath.clear();
					}
					// html += "<td>" + path + "</td>";
					if (aprs.flags & F_HASPOS)
					{
						double lat, lon;
						if (gps.location.isValid())
						{
							lat = gps.location.lat();
							lon = gps.location.lng();
						}
						else
						{
							lat = config.igate_lat;
							lon = config.igate_lon;
						}
						double dtmp = aprsParse.direction(lon, lat, aprs.lng, aprs.lat);
						double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
						html += "<td>" + String(dist, 1) + "km/" + String(dtmp, 0) + "°</td>";
					}
					else
					{
						html += "<td>-</td>\n";
					}
					html += "<td>" + String(packet) + "</td>\n";
					if (pkg.audio_level == 0)
					{
						html += "<td>-</td></tr>\n";
					}
					else
					{
						double Vrms = (double)pkg.audio_level / 1000;
						double audBV = 20.0F * log10(Vrms);
						if (audBV < -20.0F)
						{
							html += "<td style=\"color: #0000f0;\">";
						}
						else if (audBV > -5.0F)
						{
							html += "<td style=\"color: #f00000;\">";
						}
						else
						{
							html += "<td style=\"color: #008000;\">";
						}
						html += String(audBV, 1) + "dBV</td></tr>\n";
					}
				}
				path.clear();
				src_call.clear();
			}
			line.clear();
		}
	}
	html += "</table>\n";
	// log_d("HTML Length=%d Byte",html.length());
	if(gethtml) return html;
	size_t len = html.length();
	char *info = (char *)calloc(len+1, sizeof(char));
	if (info)
	{
		memset(info,0,len+1);
		html.toCharArray(info, len, 0);
		html.clear();
		lastheard_events.send(info, "lastHeard", millis()/1000, 3000);
		free(info);
	}
	// lastheard_events.send(html.c_str(), "lastHeard", millis());
	// adcEn=1;
	// dacEn=0;
	return "";
}

String event_chatMessage(bool gethtml)
{
	// log_d("Event count: %d",lastheard_events.count());
	// if (message_events.count() == 0)
	//	return "NO";

	struct tm tmstruct, tmNow;

	String html = "";

	time_t timen = time(NULL);
	localtime_r(&timen, &tmNow);

	html += "<tr>\n";
	html += "<th style=\"width:60pt\"><span><b>Time (";
	if (config.timeZone >= 0)
		html += "+";

	if (config.timeZone == (int)config.timeZone)
		html += String((int)config.timeZone) + ")</b></span></th>\n";
	else
		html += String(config.timeZone, 1) + ")</b></span></th>\n";
	// html += "<th style=\"min-width:16px\">ICON</th>\n";

	html += "<th style=\"width:70pt\">Callsign</th>\n";
	html += "<th>Message</th>\n";
	html += "<th style=\"width:10pt\">ACK</th>\n";
	html += "<th style=\"width:20pt\">msgID</th>\n";
	html += "</tr>\n";

	pkgMsgSort(msgQueue);
	for (int i = 0; i < PKGLISTSIZE; i++)
	{
		if (i >= PKGLISTSIZE)
			break;
		msgType pkg = getMsgList(i);
		if (pkg.time > 0)
		{
			String line = String(pkg.text);

			pkg.callsign[10] = 0;
			// time_t tm = pkg.time;
			localtime_r(&pkg.time, &tmstruct);
			char strTime[10];
			// sprintf(strTime, "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
			if (tmNow.tm_mday == tmstruct.tm_mday)
				sprintf(strTime, "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
			else
				sprintf(strTime, "%dd %02d:%02d", tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min);
			// String str = String(tmstruct.tm_hour, DEC) + ":" + String(tmstruct.tm_min, DEC) + ":" + String(tmstruct.tm_sec, DEC);

			if (pkg.ack > 0)
			{
				html += "<tr style=\"background-color: #f1697dff;\">";
			}
			else if (pkg.ack == -1)
			{
				html += "<tr style=\"background-color: #7ff1c5ff;\">";
			}
			else if (pkg.ack == -2)
			{
				html += "<tr style=\"background-color: #73caf0ff;\">";
			}
			else
			{
				html += "<tr style=\"background-color: #f55353ff;\">";
			}
			html += "<td>" + String(strTime) + "</td>";
			html += "<td>" + String(pkg.callsign) + "</td>";
			html += "<td style=\"text-align: left;\">" + String(pkg.text) + "</td>";
			if (pkg.ack > 0)
			{
				html += "<td>" + String(pkg.ack) + "/" + String(config.msg_retry) + "</td>";
			}
			else if (pkg.ack == -1)
			{
				html += "<td>RX</td>";
			}
			else if (pkg.ack == -2)
			{
				html += "<td>TX</td>";
			}
			else
			{
				html += "<td>TF</td>";
			}
			html += "<td>" + String(pkg.msgID) + "</td></tr>";
		}
	}
	log_d("HTML Length=%d Byte gethtml:%d event_cnt:%d", html.length(),gethtml,message_events.count());
	if(gethtml) return html;
	if (message_events.count() >0){
		size_t len = html.length();
		char *info = (char *)calloc(len, sizeof(char));
		if (info)
		{
			html.toCharArray(info, len, 0);
			html.clear();
			message_events.send(info, "chatMsg", time(NULL), 5000);
			free(info);
		}	
	}
	return "";
}

void handle_storage(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	adcEn=-1;
	dacEn=-1;
	delay(100);

	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	String dirname = "/";
	char strTime[100];

	unsigned long cardTotal = LITTLEFS.totalBytes();
	unsigned long cardUsed = LITTLEFS.usedBytes();

	String webString = "<div style=\"font-size: 8pt;text-align:left;\">";
	webString += "<b>Total space: </b>";
	if (cardTotal > 1000000)
		webString += String((double)cardTotal / 1048576, 2) + " MByte ,";
	else
		webString += String((double)cardTotal / 1024, 2) + " KByte ,";
	webString += "<b>Used space: </b>";
	webString += String((double)cardUsed / 1024, 2) + " KByte";

	webString += "</br>Listing directory: </b>" + dirname + "</div>\n";

	File root = LITTLEFS.open(dirname);
	if (!root)
	{
		webString += "Failed to open directory\n";
		// return;
	}
	if (!root.isDirectory())
	{
		webString += "Not a directory";
		// return;
	}

	File file = root.openNextFile();
	webString += "<table border=\"1\"><tr align=\"center\" bgcolor=\"#03DDFC\"><td><b>DIRECTORY</b></td><td width=\"150\"><b>FILE NAME</b></td><td width=\"100\"><b>SIZE(Byte)</b></td><td width=\"170\"><b>DATE TIME</b></td><td><b>DEL</b></td></tr>";
	while (file)
	{
		if (file.isDirectory())
		{
			// webString += "<tr><td>DIR : ");
			webString += "<tr><td>" + String(file.name()) + "</td>";
			time_t t = file.getLastWrite();
			struct tm *tmstruct = localtime(&t);
			sprintf(strTime, "<td></td><td></td><td align=\"right\">%d-%02d-%02d %02d:%02d:%02d</td>", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
			webString += String(strTime);
			// if (levels) {
			//	listDir(fs, file.name(), levels - 1);
			// }
			webString += "<td></td></tr>\n";
		}
		else
		{
			/*Serial.print("  FILE: ");
			Serial.print(file.name());*/
			// String fName = String(file.name()).substring(1);
			String fName = String(file.name());
			webString += "<tr><td>/</td><td align=\"right\"><a href=\"/download?FILE=" + fName + "\" target=\"_blank\">" + fName + "</a></td>";
			// Serial.print("  SIZE: ");
			webString += "<td align=\"right\">" + String(file.size()) + "</td>";
			time_t t = file.getLastWrite();
			struct tm *tmstruct = localtime(&t);
			sprintf(strTime, "<td align=\"right\">%d-%02d-%02d %02d:%02d:%02d</td>", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
			webString += String(strTime);
			webString += "<td align=\"center\"><a href=\"/delete?FILE=" + fName + "\">X</a></td></tr>\n";
		}
		file = root.openNextFile();
	}
	webString += "</table>\n";
	webString += "<form accept-charset=\"UTF-8\" action=\"/format\" class=\"form-horizontal\" id=\"format_form\" method=\"post\">\n";
	webString += "<div><button class=\"button\" type='submit' id='format_form_sumbit'  name=\"commit\"> FORMAT </button></div>\n";
	webString += "</form><br/>\n";
	webString += "</body>\n</html>\n";
	char *info = (char *)calloc(webString.length(), sizeof(char));
	if (info)
	{
		webString.toCharArray(info, webString.length(), 0);
		webString.clear();
		request->send(200, "text/html", info); // send to someones browser when asked
		free(info);
	}
	adcEn=1;
	dacEn=0;	
}

void handle_download(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	String dataType = "";
	String path = "";

	if (request->args() > 0)
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "FILE")
			{
				path = request->arg(i);
				break;
			}
		}
	}

	if (path.endsWith(".src"))
		path = path.substring(0, path.lastIndexOf("."));
	else if (path.endsWith(".htm"))
		dataType = "text/html";
	else if (path.endsWith(".csv"))
		dataType = "text/csv";
	else if (path.endsWith(".css"))
		dataType = "text/css";
	else if (path.endsWith(".xml"))
		dataType = "text/xml";
	else if (path.endsWith(".png"))
		dataType = "image/png";
	else if (path.endsWith(".gif"))
		dataType = "image/gif";
	else if (path.endsWith(".jpg"))
		dataType = "image/jpeg";
	else if (path.endsWith(".ico"))
		dataType = "image/x-icon";
	else if (path.endsWith(".svg"))
		dataType = "image/svg+xml";
	else if (path.endsWith(".ico"))
		dataType = "image/x-icon";
	else if (path.endsWith(".js"))
		dataType = "application/javascript";
	else if (path.endsWith(".pdf"))
		dataType = "application/pdf";
	else if (path.endsWith(".zip"))
		dataType = "application/zip";
	else if (path.endsWith(".cfg"))
		dataType = "text/html";
	else if (path.endsWith(".json"))
		dataType = "application/json";
	else if (path.endsWith(".gz"))
	{
		if (path.startsWith("/gz/htm"))
			dataType = "text/html";
		else if (path.startsWith("/gz/css"))
			dataType = "text/css";
		else if (path.startsWith("/gz/csv"))
			dataType = "text/csv";
		else if (path.startsWith("/gz/xml"))
			dataType = "text/xml";
		else if (path.startsWith("/gz/js"))
			dataType = "application/javascript";
		else if (path.startsWith("/gz/svg"))
			dataType = "image/svg+xml";
		else
			dataType = "application/x-gzip";
	}

	if (path != "" && dataType != "")
	{
		String file = "/" + path;
		request->send(LITTLEFS, file, dataType, true);
		// AsyncWebServerResponse *response = request->beginResponse(LITTLEFS, file, dataType, true);
		// response->addHeader("Content-Disposition","attachment");
		// request->send(response);
	}
	else
	{
		if (dataType != "")
			request->send_P(404, PSTR("text/plain"), PSTR("ContentType Not Support"));
		else
			request->send_P(404, PSTR("text/plain"), PSTR("File Not found"));
	}
}

void handle_delete(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	String html = "FAIL";
	String dataType = "text/plain";
	String path;
	if (request->args() > 0)
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "FILE")
			{
				path = request->arg(i);
#ifdef DEBUG
				Serial.println("Deleting file: " + path);
#endif
				if (LITTLEFS.remove("/" + path))
				{
					html = "File deleted";
#ifdef DEBUG
					Serial.println("File deleted");
#endif
				}
				else
				{
					html = "Delete failed";
#ifdef DEBUG
					Serial.println("Delete failed");
#endif
				}
				break;
			}
		}
	}
	request->send(200, "text/html", html); // send to someones browser when asked
}

void handle_format(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	String html = "FAIL";
	if (request->args() > 0)
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "commit")
			{
				if (request->arg(i) == "FORMAT")
				{
					LITTLEFS.format();
					html = "OK";
					break;
				}
			}
		}
	}

	request->send(200, "text/html", html); // send to someones browser when asked
}

// handle_radio moved to src/web_config.cpp

// handle_vpn moved to src/web_config.cpp

// handle_mqtt moved to src/web_config.cpp

// handle_msg moved to src/web_config.cpp

// handle_mod moved to src/web_config.cpp

// handle_system moved to src/web_config.cpp

// handle_igate moved to src/web_aprs.cpp

// handle_digi moved to src/web_aprs.cpp

// handle_wx moved to src/web_aprs.cpp

// handle_tlm moved to src/web_aprs.cpp


// handle_tracker moved to src/web_aprs.cpp

// handle_wireless moved to src/web_aprs.cpp

//extern String lastPkgRaw;
//extern float dBV;
//extern int mVrms;
// void handle_realtime(AsyncWebServerRequest *request)
// {
// 	// char jsonMsg[1000];
// 	char *jsonMsg;
// 	time_t timeStamp;
// 	time(&timeStamp);

// 	if (afskSync && (lastPkgRaw.length() > 5))
// 	{
// 		int input_length = lastPkgRaw.length();
// 		jsonMsg = (char *)malloc((input_length * 2) + 200);
// 		char *input_buffer = (char *)malloc(input_length + 2);
// 		char *output_buffer = (char *)malloc(input_length * 2);
// 		if (output_buffer)
// 		{
// 			// lastPkgRaw.toCharArray(input_buffer, lastPkgRaw.length(), 0);
// 			memcpy(input_buffer, lastPkgRaw.c_str(), lastPkgRaw.length());
// 			lastPkgRaw.clear();
// 			encode_base64((unsigned char *)input_buffer, input_length, (unsigned char *)output_buffer);
// 			// Serial.println(output_buffer);
// 			sprintf(jsonMsg, "{\"Active\":\"1\",\"mVrms\":\"%d\",\"RAW\":\"%s\",\"timeStamp\":\"%li\"}", mVrms, output_buffer, timeStamp);
// 			// Serial.println(jsonMsg);
// 			free(input_buffer);
// 			free(output_buffer);
// 		}
// 	}
// 	else
// 	{
// 		jsonMsg = (char *)malloc(100);
// 		if (afskSync)
// 			sprintf(jsonMsg, "{\"Active\":\"1\",\"mVrms\":\"%d\",\"RAW\":\"REVDT0RFIEZBSUwh\",\"timeStamp\":\"%li\"}", mVrms, timeStamp);
// 		else
// 			sprintf(jsonMsg, "{\"Active\":\"0\",\"mVrms\":\"0\",\"RAW\":\"\",\"timeStamp\":\"%li\"}", timeStamp);
// 	}
// 	afskSync = false;
// 	request->send(200, "text/html", String(jsonMsg));

// 	delay(100);
// 	free(jsonMsg);
// }

//void handle_ws(String Raw,uint16_t mVrms)
void handle_ws(char *Raw,size_t len,uint16_t mVrms)
{
	if (ws.count() < 1)
	return;

	char *jsonMsg;
	time_t timeStamp;
	time(&timeStamp);

	if (len > 5)
	{
		int input_length = len;
		jsonMsg = (char *)calloc((input_length * 2) + 200, sizeof(char));
		if (jsonMsg)
		{
			char *input_buffer = (char *)calloc(input_length + 2, sizeof(char));
			char *output_buffer = (char *)calloc(input_length * 2, sizeof(char));
			if (output_buffer)
			{
				memset(input_buffer, 0, (input_length + 2));
				memset(output_buffer, 0, (input_length * 2));
				// lastPkgRaw.toCharArray(input_buffer, input_length, 0);
				memcpy(input_buffer, Raw, len);
				encode_base64((unsigned char *)input_buffer, input_length, (unsigned char *)output_buffer);
				// Serial.println(output_buffer);
				sprintf(jsonMsg, "{\"Active\":\"1\",\"mVrms\":\"%d\",\"RAW\":\"%s\",\"timeStamp\":\"%li\"}", mVrms, output_buffer, timeStamp);
				// Serial.println(jsonMsg);
				free(input_buffer);
				free(output_buffer);
			}
			ws.textAll(jsonMsg);
			free(jsonMsg);
		}
	}
	else
	{
		jsonMsg = (char *)calloc(300, sizeof(char));
		if (jsonMsg)
		{
			if (mVrms>0)
				sprintf(jsonMsg, "{\"Active\":\"1\",\"mVrms\":\"%d\",\"RAW\":\"REVDT0RFIEZBSUwh\",\"timeStamp\":\"%li\"}", mVrms, timeStamp);
			else
				sprintf(jsonMsg, "{\"Active\":\"0\",\"mVrms\":\"0\",\"RAW\":\"\",\"timeStamp\":\"%li\"}", timeStamp);
			ws.textAll(jsonMsg);
			free(jsonMsg);
		}
	}
}

void handle_ws_gnss(char *nmea, size_t size)
{
	if(ws_gnss.count() < 1)
		return;

	time_t timeStamp;
	time(&timeStamp);
	unsigned int output_length = encode_base64_length(size);
	unsigned char nmea_enc[output_length];
	char jsonMsg[output_length + 100];
	encode_base64((unsigned char *)nmea, size, (unsigned char *)nmea_enc);
	sprintf(jsonMsg, "{\"en\":\"%d\",\"lat\":\"%.5f\",\"lng\":\"%.5f\",\"alt\":\"%.2f\",\"spd\":\"%.2f\",\"csd\":\"%.1f\",\"hdop\":\"%.2f\",\"sat\":\"%d\",\"time\":\"%d\",\"timeStamp\":\"%li\",\"RAW\":\"", (int)config.gnss_enable, gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.speed.kmph(), gps.course.deg(), gps.hdop.hdop(), gps.satellites.value(), gps.time.value(), timeStamp);
	strncat(jsonMsg, (const char *)nmea_enc, output_length);
	strcat(jsonMsg, "\"}");
	ws_gnss.textAll(jsonMsg);
}

void handle_test(AsyncWebServerRequest *request)
{
	// if (request->hasArg("sendBeacon"))
	// {
	// 	String tnc2Raw = send_fix_location();
	// 	if (config.rf_en)
	// 		pkgTxPush(tnc2Raw.c_str(), tnc2Raw.length(), 0);
	// 	// APRS_sendTNC2Pkt(tnc2Raw); // Send packet to RF
	// }
	// else if (request->hasArg("sendRaw"))
	// {
	// 	for (uint8_t i = 0; i < request->args(); i++)
	// 	{
	// 		if (request->argName(i) == "raw")
	// 		{
	// 			if (request->arg(i) != "")
	// 			{
	// 				String tnc2Raw = request->arg(i);
	// 				if (config.rf_en)
	// 				{
	// 					pkgTxPush(tnc2Raw.c_str(), tnc2Raw.length(), 0);
	// 					// APRS_sendTNC2Pkt(request->arg(i)); // Send packet to RF
	// 					// Serial.println("Send RAW: " + tnc2Raw);
	// 				}
	// 			}
	// 			break;
	// 		}
	// 	}
	// }
	// setHTML(6);

	webString = "<html>\n<head>\n";
	webString += "<script src=\"https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js\"></script>\n";
	webString += "<script src=\"https://code.highcharts.com/highcharts.js\"></script>\n";
	webString += "<script src=\"https://code.highcharts.com/highcharts-more.js\"></script>\n";
	webString += "<script language=\"JavaScript\">";
	webString += "$(document).ready(function() {\nvar chart = {\ntype: 'gauge',plotBorderWidth: 1,plotBackgroundColor: {linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 },stops: [[0, '#FFFFC6'],[0.3, '#FFFFFF'],[1, '#FFF4C6']]},plotBackgroundImage: null,height: 200};\n";
	webString += "var credits = {enabled: false};\n";
	webString += "var title = {text: 'RX/AUDIO VU Meter'};\n";
	webString += "var pane = [{startAngle: -45,endAngle: 45,background: null,center: ['50%', '145%'],size: 300}];\n";
	webString += "var yAxis = [{min: -40,max: 1,minorTickPosition: 'outside',tickPosition: 'outside',labels: {rotation: 'auto',distance: 20},\n";
	webString += "plotBands: [{from: -10,to: 1,color: '#C02316',innerRadius: '100%',outerRadius: '105%'},{from: -20,to: -10,color: '#00C000',innerRadius: '100%',outerRadius: '105%'},{from: -30,to: -20,color: '#AFFF0F',innerRadius: '100%',outerRadius: '105%'},{from: -40,to: -30,color: '#C0A316',innerRadius: '100%',outerRadius: '105%'}],\n";
	webString += "pane: 0,title: {text: '<span style=\"font-size:12px\">dBV</span>',y: -40}}];\n";
	webString += "var plotOptions = {gauge: {dataLabels: {enabled: false},dial: {radius: '100%'}}};\n";
	webString += "var series= [{data: [-40],yAxis: 0}];\n";
	webString += "var json = {};\n json.chart = chart;\n json.credits = credits;\n json.title = title;\n json.pane = pane;\n json.yAxis = yAxis;\n json.plotOptions = plotOptions;\n json.series = series;\n";
	// Add some life
	webString += "var chartFunction = function (chart) { \n"; // the chart may be destroyed
	webString += "var Vrms=0;\nvar dBV=-40;\nvar active=0;var raw=\"\";var timeStamp;\n";
	webString += "if (chart.series) {\n";
	webString += "var left = chart.series[0].points[0];\n";
	webString += "var host='ws://'+location.hostname+':81/ws'\n";
	webString += "const ws = new WebSocket(host);\n";
	webString += "ws.onopen = function() { console.log('Connection opened');};\n ws.onclose = function() { console.log('Connection closed');};\n";
	webString += "ws.onmessage = function(event) {\n  console.log(event.data);\n";
	webString += "const jsonR=JSON.parse(event.data);\n";
	webString += "active=parseInt(jsonR.Active);\n";
	webString += "Vrms=parseFloat(jsonR.mVrms)/1000;\n";
	webString += "dBV=20.0*Math.log10(Vrms);\n";
	webString += "if(dBV<-40) dBV=-40;\n";
	webString += "raw=jsonR.RAW;\n";
	webString += "timeStamp=Number(jsonR.timeStamp);\n";
	webString += "if(active==1){\nleft.update(dBV,false);\nchart.redraw();\n";
	webString += "var date=new Date(timeStamp * 1000);\n";
	webString += "var head=date+\"[\"+Vrms.toFixed(3)+\"Vrms,\"+dBV.toFixed(1)+\"dBV]\\n\";\n";
	// webString += "document.getElementById(\"raw_txt\").value+=head+atob(raw)+\"\\n\";\n";
	webString += "var textArea=document.getElementById(\"raw_txt\");\n";
	webString += "textArea.value+=head+atob(raw)+\"\\n\";\n";
	webString += "textArea.scrollTop = textArea.scrollHeight;\n";
	webString += "}\n";
	webString += "}\n";
	webString += "}};\n";
	webString += "$('#vumeter').highcharts(json, chartFunction);\n";
	webString += "});\n</script>\n";
	webString += "</head><body>\n<table>\n";
	// webString += "<tr><td><form accept-charset=\"UTF-8\" action=\"/test\" class=\"form-horizontal\" id=\"test_form\" method=\"post\">\n";
	// webString += "<div style=\"margin-left: 20px;\"><input type='submit' class=\"btn btn-danger\" name=\"sendBeacon\" value='SEND BEACON'></div><br />\n";
	// webString += "<div style=\"margin-left: 20px;\">TNC2 RAW: <input id=\"raw\" name=\"raw\" type=\"text\" size=\"60\" value=\"" + String(config.aprs_mycall) + ">APE32I,WIDE1-1:>Test Status\"/></div>\n";
	// webString += "<div style=\"margin-left: 20px;\"><input type='submit' class=\"btn btn-primary\" name=\"sendRaw\" value='SEND RAW'></div> <br />\n";
	// webString += "</form></td></tr>\n";
	// webString += "<tr><td><hr width=\"80%\" /></td></tr>\n";
	webString += "<tr><td><div id=\"vumeter\" style=\"width: 300px; height: 200px; margin: 10px;\"></div></td>\n";
	webString += "<tr><td><div style=\"margin: 15px;\">Terminal<br /><textarea id=\"raw_txt\" name=\"raw_txt\" rows=\"50\" cols=\"80\" /></textarea></div></td></tr>\n";
	webString += "</table>\n";

	webString += "</body></html>\n";
	request->send(200, "text/html", webString); // send to someones browser when asked

	delay(100);
	webString.clear();
}

void handle_about(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	char strCID[50];
	uint64_t chipid = ESP.getEfuseMac();
	sprintf(strCID, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

	webString.clear();
	webString += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;\"><td width=\"49%\" style=\"border:unset;\">";

	webString += "<table>";
	webString += "<th colspan=\"2\"><span><b>System Information</b></span></th>\n";
	// webString += "<tr><th width=\"200\"><span><b>Name</b></span></th><th><span><b>Information</b></span></th></tr>";
	webString += "<tr><td align=\"right\"><b>Hardware Version: </b></td><td align=\"left\">";
#if defined(CONFIG_IDF_TARGET_ESP32)
	webString += "ESP32-WROOM,ESP32 DoIt DevKit";
#elif defined(ESP32C3_MINI)
	webString += "ESP32C3-Mini,ESP32-C3 DIY";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
	webString += "ESP32C3,ESP32-C3 DIY";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
	webString += "ESP32C6,ESP32-C6 DIY";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
	webString += "ESP32-S3-DevKit,ESP32-S3-WROOM";
#else
	webString += "UNKNOWN,ESP32 DIY";
#endif
	webString += "</td></tr>";
	webString += "<tr><td align=\"right\"><b>Firmware Version: </b></td><td align=\"left\"> V" + String(VERSION) + String(VERSION_BUILD) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>RF LoRa Chip: </b></td><td align=\"left\"> " + String(RF_TYPE[config.rf_type]) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>ESP32 Model: </b></td><td align=\"left\"> " + String(ESP.getChipModel()) + "</td></tr>";
	webString += "<tr><td align=\"right\"><b>Revision: </b></td><td align=\"left\"> " + String(ESP.getChipRevision()) + "</td></tr>";
	webString += "<tr><td align=\"right\"><b>Chip ID: </b></td><td align=\"left\"> " + String(strCID) + "</td></tr>";
	webString += "<tr><td align=\"right\"><b>Flash: </b></td><td align=\"left\">" + String(ESP.getFlashChipSize() / 1024) + " KByte</td></tr>";
	webString += "<tr><td align=\"right\"><b>PSRAM: </b></td><td align=\"left\">" + String((float)ESP.getFreePsram() / 1024, 1) + "/" + String((float)ESP.getPsramSize() / 1024, 1) + " KByte</td></tr>";
	webString += "<tr><td align=\"right\"><b>FILE SYSTEM: </b></td><td align=\"left\">" + String((float)LITTLEFS.usedBytes() / 1024, 1) + "/" + String((float)LITTLEFS.totalBytes() / 1024, 1) + " KByte</td></tr>";
	webString += "</table>";
	webString += "</td><td width=\"2%\" style=\"border:unset;\"></td>";
	webString += "<td width=\"49%\" style=\"border:unset;\">";

	webString += "<table>";
	webString += "<th colspan=\"2\"><span><b>Developer/Support Information</b></span></th>\n";
	webString += "<tr><td align=\"right\"><b>Author: </b></td><td align=\"left\">Mr.Somkiat Nakhonthai </td></tr>";
	webString += "<tr><td align=\"right\"><b>Callsign: </b></td><td align=\"left\">HS5TQA,Atten,Nakhonthai</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Country: </b></td><td align=\"left\">Bangkok,Thailand</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Github: </b></td><td align=\"left\"><a href=\"https://github.com/nakhonthai\" target=\"_github\">https://github.com/nakhonthai</a></td></tr>";
	webString += "<tr><td align=\"right\"><b>Youtube: </b></td><td align=\"left\"><a href=\"https://www.youtube.com/@HS5TQA\" target=\"_youtube\">https://www.youtube.com/@HS5TQA</a></td></tr>";
	webString += "<tr><td align=\"right\"><b>Facebook: </b></td><td align=\"left\"><a href=\"https://www.facebook.com/atten\" target=\"_facebook\">https://www.facebook.com/atten</a></td></tr>";
	webString += "<tr><td align=\"right\"><b>Chat: </b></td><td align=\"left\">Telegram:<a href=\"https://t.me/HS5TQA\" target=\"_line\">@HS5TQA</a> , WeChat:HS5TQA</td></tr>";
	webString += "<tr><td align=\"right\"><b>Sponsors: </b></td><td align=\"left\"><a href=\"https://github.com/sponsors/nakhonthai\" target=\"_sponsor\">https://github.com/sponsors/nakhonthai</a></td></tr>";
	webString += "<tr><td align=\"right\"><b>Donate: </b></td><td align=\"left\"><a href=\"https://www.paypal.me/0hs5tqa0\" target=\"_sponsor\">https://www.paypal.me/0hs5tqa0</a></td></tr>";

	webString += "</table>";
	webString += "</td></tr></table><br />";

	webString += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;\"><td width=\"49%\" style=\"border:unset;\">";

	webString += "<table>\n";
	webString += "<th colspan=\"2\"><span><b>WiFi Status</b></span></th>\n";
	webString += "<tr><td align=\"right\"><b>Mode:</b></td>\n";
	webString += "<td align=\"left\">";
	if (config.wifi_mode == WIFI_AP_FIX)
	{
		webString += "AP";
	}
	else if (config.wifi_mode == WIFI_STA_FIX)
	{
		webString += "STA";
	}
	else if (config.wifi_mode == WIFI_AP_STA_FIX)
	{
		webString += "AP+STA";
	}
	else
	{
		webString += "OFF";
	}
	uint8_t proto=0;
	esp_wifi_get_protocol(WIFI_IF_STA, &proto);
	webString += " (802.11";
	if(proto & WIFI_PROTOCOL_11B)
		webString += "b";
	if(proto & WIFI_PROTOCOL_11G)
		webString += "g";
	if(proto & WIFI_PROTOCOL_11N)
		webString += "n";
	if(proto & WIFI_PROTOCOL_LR)
		webString += "lr";
	webString += ")";

	wifi_power_t wpr = WiFi.getTxPower();
	String wifipower = "";
	if (wpr < 8)
	{
		wifipower = "-1 dBm";
	}
	else if (wpr < 21)
	{
		wifipower = "2 dBm";
	}
	else if (wpr < 29)
	{
		wifipower = "5 dBm";
	}
	else if (wpr < 35)
	{
		wifipower = "8.5 dBm";
	}
	else if (wpr < 45)
	{
		wifipower = "11 dBm";
	}
	else if (wpr < 53)
	{
		wifipower = "13 dBm";
	}
	else if (wpr < 61)
	{
		wifipower = "15 dBm";
	}
	else if (wpr < 69)
	{
		wifipower = "17 dBm";
	}
	else if (wpr < 75)
	{
		wifipower = "18.5 dBm";
	}
	else if (wpr < 77)
	{
		wifipower = "19 dBm";
	}
	else if (wpr < 80)
	{
		wifipower = "19.5 dBm";
	}
	else
	{
		wifipower = "20 dBm";
	}

	webString += "</td></tr>\n";
	webString += "<tr><td align=\"right\" width=\"30%\"><b>MAC:</b></td>\n";
	webString += "<td align=\"left\">" + String(WiFi.macAddress()) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Channel:</b></td>\n";
	webString += "<td align=\"left\">" + String(WiFi.channel()) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>TX Power:</b></td>\n";
	webString += "<td align=\"left\">" + wifipower + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>SSID:</b></td>\n";
	webString += "<td align=\"left\">" + String(WiFi.SSID()) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Local IP:</b></td>\n";
	webString += "<td align=\"left\">" + WiFi.localIP().toString() + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Gateway IP:</b></td>\n";
	webString += "<td align=\"left\">" + WiFi.gatewayIP().toString() + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>DNS:</b></td>\n";
	webString += "<td align=\"left\">" + WiFi.dnsIP().toString() + "</td></tr>\n";
	webString += "</table>\n";

	webString += "</td><td width=\"2%\" style=\"border:unset;\"></td>";
	webString += "<td width=\"49%\" style=\"border:unset;\">";
	webString += "<table>\n";
	#ifdef PPPOS
	webString += "<th colspan=\"2\"><span><b>PPPoS Status</b></span></th>\n";
	webString += "<tr><td align=\"right\" width=\"30%\"><b>Manufacturer:</b></td>\n";
	webString += "<td align=\"left\">" + String(pppStatus.manufacturer) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Model:</b></td>\n";
	webString += "<td align=\"left\">" + String(pppStatus.model) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>IMEI:</b></td>\n";
	webString += "<td align=\"left\">" + String(pppStatus.imei) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>IMSI:</b></td>\n";
	webString += "<td align=\"left\">" + String(pppStatus.imsi) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Operator:</b></td>\n";
	webString += "<td align=\"left\">" + String(pppStatus.oper) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>RSSI:</b></td>\n";
	webString += "<td align=\"left\">" + String(pppStatus.rssi) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>IP:</b></td>\n";
	webString += "<td align=\"left\">" + String(IPAddress(pppStatus.ip)) + "</td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Gateway:</b></td>\n";
	webString += "<td align=\"left\">" + String(IPAddress(pppStatus.gateway)) + "</td></tr>\n";
	// webString += "<tr><td align=\"right\"><b>DNS:</b></td>\n";
	// webString += "<td align=\"left\">" + String(IPAddress(pppStatus.dns)) + "</td></tr>\n";
	#endif
	webString += "</table>\n";
	webString += "</td></tr></table><br />";

	// webString += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;\"><td width=\"96%\" style=\"border:unset;\">";

	webString += "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form' class=\"form-horizontal\">\n";
	webString += "<table>";
	webString += "<th colspan=\"2\"><span><b>Firmware Update</b></span></th>\n";
	webString += "<tr><td align=\"right\"><b>File:</b></td><td align=\"left\"><input id=\"file\" name=\"update\" type=\"file\" onchange='sub(this)' /></td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Progress:</b></td><td><div id='prgbar'><div id='bar' style=\"width: 0px;\"><label id='prg'></label></div></div></td></tr>\n";
	webString += "<tr><td align=\"right\"><b>Support Firmware:</b></td><td align=\"left\"><a target=\"_download\" href=\"https://github.com/nakhonthai/ESP32APRS_Audio/releases\">https://github.com/nakhonthai/ESP32APRS_Audio/releases</a></td></tr>\n";
	webString += "</table><br />\n";
	webString += "<div class=\"col-sm-3 col-xs-4\"><input type='submit' class=\"btn btn-danger\" id=\"update_sumbit\" value='Firmware Update'></div>\n";

	webString += "</form>\n";
	// webString += "</td></tr></table><br />";

	webString += "<script>"
				 "function sub(obj){"
				 "var fileName = obj.value.split('\\\\');"
				 "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
				 "};"
				 "$('form').submit(function(e){"
				 "e.preventDefault();"
				 "var form = $('#upload_form')[0];"
				 "var data = new FormData(form);"
				 "document.getElementById('update_sumbit').disabled = true;"
				 "$.ajax({"
				 "url: '/update',"
				 "type: 'POST',"
				 "data: data,"
				 "contentType: false,"
				 "processData:false,"
				 "xhr: function() {"
				 "var xhr = new window.XMLHttpRequest();"
				 "xhr.upload.addEventListener('progress', function(evt) {"
				 "if (evt.lengthComputable) {"
				 "var per = evt.loaded / evt.total;"
				 "$('#prg').html(Math.round(per*100) + '%');"
				 "$('#bar').css('width',Math.round(per*100) + '%');"
				 "}"
				 "}, false);"
				 "return xhr;"
				 "},"
				 "success:function(d, s) {"
				 "alert('Wait for system reboot 10sec') "
				 "},"
				 "error: function (a, b, c) {"
				 "}"
				 "});"
				 "});"
				 "</script>";

	webString += "</body></html>\n";
	request->send(200, "text/html", webString); // send to someones browser when asked
}

void handle_gnss(AsyncWebServerRequest *request)
{
	webString = "<html>\n<head>\n";
	webString += "<script src=\"https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js\"></script>\n";
	webString += "<script src=\"https://code.highcharts.com/highcharts.js\"></script>\n";
	webString += "<script src=\"https://code.highcharts.com/highcharts-more.js\"></script>\n";
	webString += "<script language=\"JavaScript\">";

	// Add some life
	webString += "function gnss() { \n"; // the chart may be destroyed
	webString += "var raw=\"\";var timeStamp;\n";
	webString += "var host='ws://'+location.hostname+':81/ws_gnss'\n";
	webString += "const ws = new WebSocket(host);\n";
	webString += "ws.onopen = function() { console.log('Connection opened');};\n ws.onclose = function() { console.log('Connection closed');};\n";
	webString += "ws.onmessage = function(event) {\n  console.log(event.data);\n";
	webString += "const jsonR=JSON.parse(event.data);\n";
	webString += "document.getElementById(\"en\").innerHTML=parseInt(jsonR.en);\n";
	webString += "document.getElementById(\"lat\").innerHTML=parseFloat(jsonR.lat);\n";
	webString += "document.getElementById(\"lng\").innerHTML=parseFloat(jsonR.lng);\n";
	webString += "document.getElementById(\"alt\").innerHTML=parseFloat(jsonR.alt);\n";
	webString += "document.getElementById(\"spd\").innerHTML=parseFloat(jsonR.spd);\n";
	webString += "document.getElementById(\"csd\").innerHTML=parseFloat(jsonR.csd);\n";
	webString += "document.getElementById(\"hdop\").innerHTML=parseFloat(jsonR.hdop);\n";
	webString += "document.getElementById(\"sat\").innerHTML=parseInt(jsonR.sat);\n";
	webString += "document.getElementById(\"time\").innerHTML=parseInt(jsonR.time);\n";
	webString += "raw=jsonR.RAW;\n";
	webString += "timeStamp=Number(jsonR.timeStamp);\n";
	webString += "var textArea=document.getElementById(\"raw_txt\");\n";
	webString += "textArea.value+=atob(raw)+\"\\n\";\n";
	webString += "textArea.scrollTop = textArea.scrollHeight;\n";
	webString += "}\n";
	webString += "};\n</script>\n";
	webString += "</head><body onload=\"gnss()\">\n";

	webString += "<table width=\"200\" border=\"1\">";
	webString += "<th colspan=\"2\" style=\"background-color: #00BCD4;\"><span><b>GNSS Information</b></span></th>\n";
	// webString += "<tr><th width=\"200\"><span><b>Name</b></span></th><th><span><b>Information</b></span></th></tr>";
	webString += "<tr><td align=\"right\"><b>Enable: </b></td><td align=\"left\"> <label id=\"en\">" + String(config.gnss_enable) + "</label></td></tr>";
	webString += "<tr><td align=\"right\"><b>Latitude: </b></td><td align=\"left\"> <label id=\"lat\">" + String(gps.location.lat(), 5) + "</label></td></tr>";
	webString += "<tr><td align=\"right\"><b>Longitude: </b></td><td align=\"left\"> <label id=\"lng\">" + String(gps.location.lng(), 5) + "</label></td></tr>";
	webString += "<tr><td align=\"right\"><b>Altitude: </b></td><td align=\"left\"> <label id=\"alt\">" + String(gps.altitude.meters(), 2) + "</label> m.</td></tr>";
	webString += "<tr><td align=\"right\"><b>Speed: </b></td><td align=\"left\"> <label id=\"spd\">" + String(gps.speed.kmph(), 2) + "</label> km/h</td></tr>";
	webString += "<tr><td align=\"right\"><b>Course: </b></td><td align=\"left\"> <label id=\"csd\">" + String(gps.course.deg(), 1) + "</label></td></tr>";
	webString += "<tr><td align=\"right\"><b>HDOP: </b></td><td align=\"left\"> <label id=\"hdop\">" + String(gps.hdop.hdop(), 2) + "</label> </td></tr>";
	webString += "<tr><td align=\"right\"><b>SAT: </b></td><td align=\"left\"> <label id=\"sat\">" + String(gps.satellites.value()) + "</label> </td></tr>";
	webString += "<tr><td align=\"right\"><b>Time: </b></td><td align=\"left\"> <label id=\"time\">" + String(gps.time.value()) + "</label> </td></tr>";
	webString += "</table><table>";
	webString += "<tr><td><b>Terminal:</b><br /><textarea id=\"raw_txt\" name=\"raw_txt\" rows=\"30\" cols=\"80\" /></textarea></td></tr>\n";
	webString += "</table>\n";

	webString += "</body></html>\n";
	request->send(200, "text/html", webString); // send to someones browser when asked

	delay(100);
	webString.clear();
}

void handle_default()
{
	defaultSetting = true;
	defaultConfig();
	defaultSetting = false;
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

	if (type == WS_EVT_CONNECT)
	{

		log_d("Websocket client connection received");
	}
	else if (type == WS_EVT_DISCONNECT)
	{

		log_d("Client disconnected");
	}
}

// void handle_vpn_request(AsyncWebServerRequest *request) {
//     HTTPClient http;

//     String url = "http://vpn.nakhonthai.net:82/wg/create";

//     String mac = WiFi.macAddress();
//     mac.replace(":", "");

//     String payload = "{\"name\":\"" + mac + "\"}";

//     http.begin(url);
//     http.addHeader("Content-Type", "application/json");

//     int httpCode = http.POST(payload);

//     if (httpCode > 0) {
//         String res = http.getString();
//         request->send(200, "application/json", res);
//     } else {
//         request->send(500, "text/plain", "Error contacting VPN server");
//     }

//     http.end();
// }


bool webServiceBegin = true;
void webService()
{
	if (webServiceBegin)
	{
		webServiceBegin = false;
	}
	else
	{
		return;
	}
	// Generate a per-boot CSRF token (128-bit random, hex-encoded)
	{
		uint32_t r[4] = {esp_random(), esp_random(), esp_random(), esp_random()};
		snprintf(csrfToken, sizeof(csrfToken), "%08x%08x%08x%08x", r[0], r[1], r[2], r[3]);
	}
	ws.onEvent(onWsEvent);

	// web client handlers
	async_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
					{ setMainPage(request); });
	async_server.on("/symbol", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_symbol(request); });
	// async_server.on("/symbol2", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
	// 				{ handle_symbol2(request); });
	async_server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_logout(request); });
	async_server.on("/radio", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_radio(request); });
	async_server.on("/vpn", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_vpn(request); });
#ifdef MQTT
	async_server.on("/mqtt", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_mqtt(request); });
#endif
	async_server.on("/msg", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_msg(request); });					
	async_server.on("/mod", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_mod(request); });
	async_server.on("/default", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_default(); });
	async_server.on("/igate", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_igate(request); });
	async_server.on("/digi", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_digi(request); });
	async_server.on("/tracker", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_tracker(request); });
	async_server.on("/wx", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_wx(request); });
	async_server.on("/tlm", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_tlm(request); });
	async_server.on("/sensor", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_sensor(request); });
	async_server.on("/system", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_system(request); });
	async_server.on("/wireless", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_wireless(request); });
	async_server.on("/tnc2", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_test(request); });
	async_server.on("/gnss", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_gnss(request); });
	// async_server.on("/realtime", HTTP_GET, [](AsyncWebServerRequest *request)
	// 				{ handle_realtime(request); });
	async_server.on("/about", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_about(request); });
	async_server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_dashboard(request); });
	async_server.on("/sidebarInfo", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_sidebar(request); });
	async_server.on("/sysinfo", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_sysinfo(request); });
	// async_server.on("/lastHeard", HTTP_GET, [](AsyncWebServerRequest *request)
	// 				{ handle_lastHeard(request); });
	async_server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_css(request); });
	async_server.on("/jquery-3.7.1.js", HTTP_GET, [](AsyncWebServerRequest *request)
					{ handle_jquery(request); });
	async_server.on("/storage", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_storage(request); });
	async_server.on("/download", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_download(request); });
	async_server.on("/delete", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_delete(request); });
	async_server.on("/format", HTTP_GET | HTTP_POST, [](AsyncWebServerRequest *request)
					{ handle_format(request); });
	//async_server.on("/api/vpnreq", HTTP_GET, handle_vpn_request);
	async_server.on(
		"/update", HTTP_POST, [](AsyncWebServerRequest *request)
		{
  		bool espShouldReboot = !Update.hasError();
  		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", espShouldReboot ? "<h1><strong>Update DONE</strong></h1><br><a href='/'>Return Home</a>" : "<h1><strong>Update FAILED</strong></h1><br><a href='/updt'>Retry?</a>");
  		response->addHeader("Connection", "close");
  		request->send(response); },
		[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
		{
			if (!index)
			{
				log_d("Update Start: %s\n", filename.c_str());
				if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
				{
					Update.printError(Serial);
				}
				else
				{
					adcEn=-1;
					dacEn=-1;
					delay(500);
					// disableLoopWDT();
					// disableCore0WDT();
					// disableCore1WDT();
					//  vTaskSuspend(taskAPRSPollHandle);
					//  vTaskSuspend(taskAPRSHandle);
					//  vTaskSuspend(taskSensorHandle);
					//  vTaskSuspend(taskSerialHandle);
					//  vTaskSuspend(taskGPSHandle);
					//  vTaskSuspend(taskSensorHandle);
				}
			}
			if (!Update.hasError())
			{
				if (Update.write(data, len) != len)
				{
					Update.printError(Serial);
				}
			}
			if (final)
			{
				if (Update.end(true))
				{
					log_d("Update Success: %uByte\n", index + len);
					delay(1000);
					esp_restart();
				}
				else
				{
					Update.printError(Serial);
				}
			}
		});

	lastheard_events.onConnect([](AsyncEventSourceClient *client)
							   {
    if(client->lastId()){
      log_d("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    String html = event_lastHeard(true);
    client->send(html.c_str(), "lastHeard", time(NULL), 5000); });
	async_server.addHandler(&lastheard_events);

	message_events.onConnect([](AsyncEventSourceClient *client)
							 {
    if(client->lastId()){
      log_d("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
	String html = event_chatMessage(true);
    client->send(html.c_str(), "chatMsg", time(NULL), 5000); });
	async_server.addHandler(&message_events);
	
	async_server.onNotFound(notFound);
	async_server.begin();
	async_websocket.addHandler(&ws);
	async_websocket.addHandler(&ws_gnss);
	async_websocket.begin();
}
