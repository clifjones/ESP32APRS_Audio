#include "webservice.h"
#include "pkg_list.h"
#include <LibAPRSesp.h>

#ifdef SH1106
#include <Adafruit_SH1106.h>
#else
#include "Adafruit_SSD1306.h"
#endif
#ifndef SCREEN_ADDRESS
#define SCREEN_ADDRESS 0x3C
#endif
#ifdef OLED
#ifdef SH1106
extern Adafruit_SH1106 display;
#else
extern Adafruit_SSD1306 display;
#endif
#endif // OLED

void handle_radio(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	// bool noiseEn=false;
	bool radioEnable = false;
	if (request->hasArg("commitRadio"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "radioEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						radioEnable = true;
					}
				}
			}

			if (request->argName(i) == "nw_band")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.band = request->arg(i).toInt();
						// if (request->arg(i).toInt())
						// 	config.band = 1;
						// else
						// 	config.band = 0;
					}
				}
			}

			if (request->argName(i) == "volume")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.volume = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rf_power")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						if (request->arg(i).toInt())
							config.rf_power = true;
						else
							config.rf_power = false;
					}
				}
			}

			if (request->argName(i) == "sql_level")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.sql_level = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "tx_freq")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.freq_tx = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "rx_freq")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.freq_rx = request->arg(i).toFloat();
				}
			}

			if (request->argName(i) == "tx_offset")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.offset_tx = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "rx_offset")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.offset_rx = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "tx_ctcss")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.tone_tx = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "rx_ctcss")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.tone_rx = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "rf_type")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.rf_type = request->arg(i).toInt();
				}
			}
		}
		// config.noise=noiseEn;
		// config.agc=agcEn;
		config.rf_en = radioEnable;
		String html = "OK";
		request->send(200, "text/html", html); // send to someones browser when asked
		saveConfiguration("/default.cfg", config);
		delay(500);
		RF_MODULE(false);
	}
	else if (request->hasArg("commitTNC"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool hpf = 0;
		bool lpf = 0;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "HPF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						hpf = true;
					}
				}
			}
			if (request->argName(i) == "LPF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						lpf = true;
					}
				}
			}
			if (request->argName(i) == "timeSlot")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.tx_timeslot = request->arg(i).toInt();
					}
				}
			}
			if (request->argName(i) == "preamble")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.preamble = request->arg(i).toInt();
					}
				}
			}
			if (request->argName(i) == "modem_type")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.modem_type = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "fx25_mode")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.fx25_mode = request->arg(i).toInt();
				}
			}
		}
		config.audio_hpf = hpf;
		config.audio_lpf = lpf;
		String html = "OK";
		request->send(200, "text/html", html); // send to someones browser when asked
		saveConfiguration("/default.cfg", config);
		afskSetModem(config.modem_type,config.audio_lpf,config.tx_timeslot,config.preamble*100,config.fx25_mode);
	}
	else
	{
		String html = "<script type=\"text/javascript\">\n";
		html += "var sliderVol = document.getElementById(\"sliderVolume\");\n";
		html += "var outputVol = document.getElementById(\"volShow\");\n";
		html += "var sliderSql = document.getElementById(\"sliderSql\");\n";
		html += "var outputSql = document.getElementById(\"sqlShow\");\n";
		html += "outputVol.innerHTML = sliderVol.value;\n";
		html += "outputSql.innerHTML = sliderSql.value;\n";
		html += "\n";
		html += "sliderVol.oninput = function () {\n";
		html += "outputVol.innerHTML = this.value;\n";
		html += "}\n";
		html += "sliderSql.oninput = function () {\n";
		html += "outputSql.innerHTML = this.value;\n";
		html += "}\n";
		html += "\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "if(e.currentTarget.id===\"formRadio\") document.getElementById(\"submitRadio\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formTNC\") document.getElementById(\"submitTNC\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/radio',\n";
		html += "type: 'POST',\n";
		html += "data: data,\n";
		html += "contentType: false,\n";
		html += "processData: false,\n";
		html += "success: function (data) {\n";
		html += "alert(\"Submited Successfully\");\n";
		html += "},\n";
		html += "error: function (data) {\n";
		html += "alert(\"An error occurred.\");\n";
		html += "}\n";
		html += "});\n";
		html += "});\n";
		html += "function rfType(){\n";
		html += "var type = document.getElementById(\"rf_type\").value;\n";
		html += "if(type==1||type==4||type==7){document.getElementById(\"tx_freq\").setAttribute(\"max\",174);document.getElementById(\"rx_freq\").setAttribute(\"max\",174);};\n";
		html += "if(type==1){document.getElementById(\"tx_freq\").setAttribute(\"min\",134);document.getElementById(\"rx_freq\").setAttribute(\"min\",134);};\n";
		html += "if(type==4||type==7){document.getElementById(\"tx_freq\").setAttribute(\"min\",136);document.getElementById(\"rx_freq\").setAttribute(\"min\",136);};\n";
		html += "if(type==2||type==5||type==8){document.getElementById(\"tx_freq\").setAttribute(\"max\",470);document.getElementById(\"rx_freq\").setAttribute(\"max\",470);};\n";
		html += "if(type==2||type==5||type==8){document.getElementById(\"tx_freq\").setAttribute(\"min\",400);document.getElementById(\"rx_freq\").setAttribute(\"min\",400);};\n";
		html += "if(type==3){document.getElementById(\"tx_freq\").setAttribute(\"min\",320);document.getElementById(\"rx_freq\").setAttribute(\"min\",320);};\n";
		html += "if(type==3){document.getElementById(\"tx_freq\").setAttribute(\"max\",400);document.getElementById(\"rx_freq\").setAttribute(\"max\",400);};\n";
		html += "if(type==6){document.getElementById(\"tx_freq\").setAttribute(\"min\",350);document.getElementById(\"rx_freq\").setAttribute(\"min\",350);};\n";
		html += "if(type==6){document.getElementById(\"tx_freq\").setAttribute(\"max\",390);document.getElementById(\"rx_freq\").setAttribute(\"max\",390);};\n";
		html += "if(type==1||type==4||type==7){document.getElementById(\"tx_freq\").setAttribute(\"value\",144.390);document.getElementById(\"rx_freq\").setAttribute(\"value\",144.390);};\n";
		html += "if(type==2||type==5||type==8){document.getElementById(\"tx_freq\").setAttribute(\"value\",432.5);document.getElementById(\"rx_freq\").setAttribute(\"value\",432.5);};\n";
		html += "\n";
		html += "}\n";
		html += "</script>\n";
		html += "<form id='formRadio' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>RF Analog Module</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String radioEnFlag = "";
		if (config.rf_en)
			radioEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"radioEnable\" value=\"OK\" " + radioEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Module Type:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"rf_type\" id=\"rf_type\" onchange=\"rfType()\">\n";
		for (int i = 0; i < 10; i++)
		{
			if (config.rf_type == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(RF_TYPE[i]) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(RF_TYPE[i]) + "</option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		float freqMin = 0;
		float freqMax = 0;
		switch (config.rf_type)
		{
		case RF_SA868_VHF:
			freqMin = 134.0F;
			freqMax = 174.0F;
			break;
		case RF_SR_1WV:
		case RF_SR_2WVS:
			freqMin = 136.0F;
			freqMax = 174.0F;
			break;
		case RF_SA868_350:
			freqMin = 320.0F;
			freqMax = 400.0F;
			break;
		case RF_SR_1W350:
			freqMin = 350.0F;
			freqMax = 390.0F;
			break;
		case RF_SA868_UHF:
		case RF_SR_1WU:
		case RF_SR_2WUS:
			freqMin = 400.0F;
			freqMax = 470.0F;
			break;
		default:
			freqMin = 134.0F;
			freqMax = 500.0F;
			break;
		}
		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX Frequency:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input type=\"number\" id=\"tx_freq\" name=\"tx_freq\" min=\"" + String(freqMin, 4) + "\" max=\"" + String(freqMax, 4) + "\"\n";
		html += "step=\"0.0001\" value=\"" + String(config.freq_tx, 4) + "\" /> MHz</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>RX Frequency:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input type=\"number\" id=\"rx_freq\" name=\"rx_freq\" min=\"" + String(freqMin, 4) + "\" max=\"" + String(freqMax, 4) + "\"\n";
		html += "step=\"0.0001\" value=\"" + String(config.freq_rx, 4) + "\" /> Mhz</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX CTCSS:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"tx_ctcss\" id=\"tx_ctcss\">\n";
		for (int i = 0; i < 39; i++)
		{
			if (config.tone_tx == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(ctcss[i], 1) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(ctcss[i], 1) + "</option>\n";
		}
		html += "</select> Hz\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>RX CTCSS:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"rx_ctcss\" id=\"rx_ctcss\">\n";
		html += "<option value=\"0\" selected>0.0</option>\n";
		for (int i = 0; i < 39; i++)
		{
			if (config.tone_rx == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(ctcss[i], 1) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(ctcss[i], 1) + "</option>\n";
		}
		html += "</select> Hz\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Narrow/Wide:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"nw_band\" id=\"nw_band\">\n";
		String cmSelNWT = "";
		String cmSelNWF = "";
		if (config.band)
		{
			cmSelNWT = "selected";
		}
		else
		{
			cmSelNWF = "selected";
		}
		html += "<option value=\"0\" " + cmSelNWF + ">12.5KHz</option>\n";
		html += "<option value=\"1\" " + cmSelNWT + ">25.0KHz</option>\n";		
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX Power:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"rf_power\" id=\"rf_power\">\n";
		String cmRfPwrF = "";
		String cmRfPwrT = "";
		if (config.rf_power)
		{
			cmRfPwrT = "selected";
		}
		else
		{
			cmRfPwrF = "selected";
		}
		html += "<option value=\"1\" " + cmRfPwrT + ">HIGH</option>\n";
		html += "<option value=\"0\" " + cmRfPwrF + ">LOW</option>\n";
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>VOLUME:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input id=\"sliderVolume\" name=\"volume\" type=\"range\"\n";
		html += "min=\"1\" max=\"8\" value=\"" + String(config.volume) + "\" /><b><span style=\"font-size: 14pt;\" id=\"volShow\">" + String(config.volume) + "</span></b></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>SQL Level:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input id=\"sliderSql\" name=\"sql_level\" type=\"range\"\n";
		html += "min=\"0\" max=\"8\" value=\"" + String(config.sql_level) + "\" /><b><span style=\"font-size: 14pt;\" id=\"sqlShow\">" + String(config.sql_level) + "</span></b></td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitRadio'  name=\"commitRadio\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitRadio\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form>";

		// AFSK,TNC Configuration
		html += "<form id='formTNC' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>AFSK/TNC Configuration</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Modem Type:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"modem_type\" id=\"modem_type\" \">\n";
		for (int i = 0; i < 3; i++)
		{
			if (config.modem_type == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(MODEM_TYPE[i]) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(MODEM_TYPE[i]) + "</option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>FX.25 Mode:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"fx25_mode\" id=\"fx25_mode\" \">\n";
		for (int i = 0; i < 3; i++)
		{
			if (config.fx25_mode == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(FX25_MODE[i]) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(FX25_MODE[i]) + "</option>\n";
		}
		html += "</select>  (FX.25 = AX.25 + FEC)\n";
		html += "</td>\n";
		html += "<tr>\n";
		// html += "<td align=\"right\"><b>Audio HPF:</b></td>\n";
		// String strFlag = "";
		// if (config.audio_hpf)
		// 	strFlag = "checked";
		// html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"HPF\" value=\"OK\" " + strFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"><i> *Audio high pass filter >1KHz cutoff 10Khz</i></label></td>\n";
		// html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Deemphasis Audio:</b></td>\n";
		String strFlag = "";
		if (config.audio_lpf)
			strFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"LPF\" value=\"OK\" " + strFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"><i> *Audio low pass filter 1hz-2.5KHz</i></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX Time Slot:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input type=\"number\" name=\"timeSlot\" min=\"200\" max=\"99999\"\n";
		html += "step=\"100\" value=\"" + String(config.tx_timeslot) + "\" /> mSec.</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Preamble:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"preamble\">\n";
		for (int i = 1; i < 11; i++)
		{
			if (config.preamble == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(i * 100) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(i * 100) + "</option>\n";
		}
		html += "</select> mSec.\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitTNC'  name=\"commitTNC\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitTNC\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form>";
		request->send(200, "text/html", html); // send to someones browser when asked
	}
}

void handle_vpn(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	if (request->hasArg("commitVPN"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool vpnEn = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "vpnEnable")
			{
				if (request->arg(i) != "")
				{
					// if (isValidNumber(request->arg(i)))
					if (String(request->arg(i)) == "OK")
						vpnEn = true;
				}
			}

			// if (request->argName(i) == "taretime") {
			//	if (request->arg(i) != "")
			//	{
			//		//if (isValidNumber(request->arg(i)))
			//		if (String(request->arg(i)) == "OK")
			//			taretime = true;
			//	}
			// }
			if (request->argName(i) == "wg_port")
			{
				if (request->arg(i) != "")
				{
					config.wg_port = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "wg_public_key")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wg_public_key, request->arg(i).c_str(), sizeof(config.wg_public_key));
					config.wg_public_key[44] = 0;
				}
			}

			if (request->argName(i) == "wg_private_key")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wg_private_key, request->arg(i).c_str(), sizeof(config.wg_private_key));
					config.wg_private_key[44] = 0;
				}
			}

			if (request->argName(i) == "wg_peer_address")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wg_peer_address, request->arg(i).c_str(), sizeof(config.wg_peer_address));
				}
			}

			if (request->argName(i) == "wg_local_address")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wg_local_address, request->arg(i).c_str(), sizeof(config.wg_local_address));
				}
			}

			if (request->argName(i) == "wg_netmask_address")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wg_netmask_address, request->arg(i).c_str(), sizeof(config.wg_netmask_address));
				}
			}

			if (request->argName(i) == "wg_gw_address")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wg_gw_address, request->arg(i).c_str(), sizeof(config.wg_gw_address));
				}
			}
		}

		config.vpn = vpnEn;
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else
	{

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "if(e.currentTarget.id===\"formVPN\") document.getElementById(\"submitVPN\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/vpn',\n";
		html += "type: 'POST',\n";
		html += "data: data,\n";
		html += "contentType: false,\n";
		html += "processData: false,\n";
		html += "success: function (data) {\n";
		html += "alert(\"Submited Successfully\");\n";
		html += "},\n";
		html += "error: function (data) {\n";
		html += "alert(\"An error occurred.\");\n";
		html += "}\n";
		html += "});\n";
		html += "});\n";

		String ESP32_ID = WiFi.macAddress();
		ESP32_ID.replace(":", "");
		html += "function loadVPNConfig() {\n";
		html += "    const url = \"http://vpn1.nakhonthai.net:81/wg/create\";\n";
		html += "    const espID = {'name': '" + ESP32_ID + "'};\n";
		html += "    fetch(url,{\n";
		html += "        method: 'POST',\n";
		html += "        body: JSON.stringify(espID),\n";
		html += "        headers: { 'Content-Type': 'application/json', 'Access-Control-Allow-Headers': 'Content-Type', 'Access-Control-Allow-Origin': '*','Access-Control-Allow-Methods': 'POST,GET,OPTIONS'}\n";        
    	html += "    })\n";
    	html += ".then(response => response.json())\n";
    	html += ".then(data => {\n";
        html += "console.log(\"VPN Data:\", data);\n";
		html += "document.getElementById(\"wg_enable\").checked = true;\n";
        html += "document.getElementById(\"wg_peer_address\").value = data.Enpoint.split(\":\")[0];\n";
        html += "document.getElementById(\"wg_port\").value = data.Enpoint.split(\":\")[1];\n";
        html += "document.getElementById(\"wg_local_address\").value = data.Address;\n";
        html += "document.getElementById(\"wg_netmask_address\").value = \"255.255.255.0\";\n";
        html += "document.getElementById(\"wg_gw_address\").value = data.Gateway;\n";
        html += "document.getElementById(\"wg_public_key\").value = data.PublicKey;\n";
        html += "document.getElementById(\"wg_private_key\").value = data.PrivateKey;\n";
    	html += "})\n";
    	html += ".catch(err => console.error(\"VPN API Error:\", err));\n}\n";
		html += "</script>\n";
		// ===== JavaScript AJAX =====
    // html += "<script>\n";
    // html += "function loadVPNConfig() {\n";
    // html += "  $.ajax({\n";
    // html += "    url: '/api/vpnreq',\n";
    // html += "    method: 'GET',\n";
    // html += "    dataType: 'json',\n";
    // html += "    success: function(data) {\n";
    // html += "       console.log(data);\n";
    // html += "       let ep = data.Enpoint.split(':');\n";
    // html += "       $('#wg_peer_address').val(ep[0]);\n";
    // html += "       $('#wg_port').val(ep[1]);\n";
    // html += "       $('#wg_local_address').val(data.Address);\n";
    // html += "       $('#wg_public_key').val(data.PublicKey);\n";
    // html += "       $('#wg_private_key').val(data.PrivateKey);\n";
    // html += "    },\n";
    // html += "    error: function(e) {\n";
    // html += "       alert('โหลดข้อมูล VPN ไม่สำเร็จ');\n";
    // html += "       console.log(e);\n";
    // html += "    }\n";
    // html += "  });\n";
    // html += "}\n";
    // html += "</script>";

		// html += "<h2>System Setting</h2>\n";
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromVPN\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Wireguard Configuration</b></span></th>\n";
		html += "<tr>";

		String syncFlage = "";
		if (config.vpn)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" id=\"wg_enable\" name=\"vpnEnable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Address</b></td>\n";
		html += "<td style=\"text-align: left;\"><input  size=\"20\" maxlength=\"32\" id=\"wg_peer_address\" name=\"wg_peer_address\" type=\"text\" value=\"" + htmlEncode(config.wg_peer_address) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Port</b></td>\n";
		html += "<td style=\"text-align: left;\"><input id=\"wg_port\" size=\"5\" name=\"wg_port\" type=\"number\" value=\"" + String(config.wg_port) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Local Address</b></td>\n";
		html += "<td style=\"text-align: left;\"><input id=\"wg_local_address\" name=\"wg_local_address\" type=\"text\" value=\"" + htmlEncode(config.wg_local_address) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Netmask</b></td>\n";
		html += "<td style=\"text-align: left;\"><input id=\"wg_netmask_address\" name=\"wg_netmask_address\" type=\"text\" value=\"" + htmlEncode(config.wg_netmask_address) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Gateway</b></td>\n";
		html += "<td style=\"text-align: left;\"><input id=\"wg_gw_address\" name=\"wg_gw_address\" type=\"text\" value=\"" + htmlEncode(config.wg_gw_address) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Public Server Key</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"50\" maxlength=\"44\" id=\"wg_public_key\" name=\"wg_public_key\" type=\"text\" value=\"" + htmlEncode(config.wg_public_key) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Private Client Key</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"50\" maxlength=\"44\" id=\"wg_private_key\" name=\"wg_private_key\" type=\"text\" value=\"" + htmlEncode(config.wg_private_key) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitVPN'  name=\"commitVPN\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitVPN\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br /><br />";

		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromGetVPN\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Helper: Free VPN Wireguard for Web Service</b></span></th>\n";
		html += "<tr><td align=\"left\">1. Click New Register button to get VPN config from web service.</td></tr>\n";
		html += "<tr><td align=\"left\">2. The VPN config will fill in the form automatically.</td></tr>\n";
		html += "<tr><td align=\"left\">3. Click Apply Change and reboot again.</td></tr>\n";
		html += "<tr><td align=\"left\">4. Enjoy your free VPN service!</td></tr>\n";
		if(String(config.wg_local_address).startsWith("10.44.")) {	
			int lastoct=String(config.wg_local_address).substring(String(config.wg_local_address).lastIndexOf('.')+1).toInt();
			//String url="http://"+String(config.wg_peer_address)+":"+String(8000+lastoct);
			//html += "<tr><td>Your External Host IP: <a href=\"" + url + "\">" + url + "</a></td></tr>\n";
			int thirdoct=String(config.wg_local_address).substring(String(config.wg_local_address).indexOf('.',String(config.wg_local_address).indexOf('.')+1)+1,String(config.wg_local_address).lastIndexOf('.')).toInt();
			String url = "http://vpn" + String(thirdoct) + ".nakhonthai.net:" + String((thirdoct*10000)+8000 + lastoct);
			html += "<tr><td>Your External by AMPR URL: <a href=\"" + url + "\" target=\"_blank\">" + url + "</a></td></tr>\n";
			url = "http://vpn.nakhonthai.net:" + String((thirdoct*10000)+8000 + lastoct);
			html += "<tr><td>Fast Direct URL: <a href=\"" + url + "\" target=\"_blank\">" + url + "</a></td></tr>\n";
		}
		html += "<tr><td><button type=\"button\" onclick=\"loadVPNConfig()\">New Register</button></td></tr>\n";
		html += "</table><br />\n";
		html += "</form>";

		request->send(200, "text/html", html); // send to someones browser when asked
	}
}

#ifdef MQTT
void handle_mqtt(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	if (request->hasArg("commitMQTT"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool mqttEn = false;
		config.mqtt_topic_flag = 0;
		config.mqtt_subscribe_flag = 0;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						mqttEn = true;
				}
			}

			if (request->argName(i) == "host")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.mqtt_host, request->arg(i).c_str(), sizeof(config.mqtt_host));
				}
			}
			if (request->argName(i) == "port")
			{
				if (request->arg(i) != "")
				{
					config.mqtt_port = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "user")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.mqtt_user, request->arg(i).c_str(), sizeof(config.mqtt_user));
				}
			}
			if (request->argName(i) == "pass")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.mqtt_pass, request->arg(i).c_str(), sizeof(config.mqtt_pass));
				}
			}
			if (request->argName(i) == "topic")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.mqtt_topic, request->arg(i).c_str(), sizeof(config.mqtt_topic));
				}
			}
			if (request->argName(i) == "subscribe")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.mqtt_subscribe, request->arg(i).c_str(), sizeof(config.mqtt_subscribe));
				}
			}

			if (request->argName(i) == "TopicTNC")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_TOPIC_TNC;
				}
			}
			if (request->argName(i) == "TopicSts")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_TOPIC_STATUS;
				}
			}
			if (request->argName(i) == "TopicTlm")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_TOPIC_TELEMETRY;
				}
			}
			if (request->argName(i) == "TopicWX")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_TOPIC_WX;
				}
			}
			if (request->argName(i) == "TopicSensor")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_TOPIC_SENSOR;
				}
			}

			if (request->argName(i) == "subCMD")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_SUBSCRIBE_CMD;
				}
			}
			if (request->argName(i) == "subTNC")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_SUBSCRIBE_TNC;
				}
			}
			if (request->argName(i) == "subMsg")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.mqtt_topic_flag |= MQTT_SUBSCRIBE_MESSAGE;
				}
			}
		}

		config.en_mqtt = mqttEn;
		clientMQTT.disconnect();
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else
	{

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "if(e.currentTarget.id===\"formVPN\") document.getElementById(\"submitMQTT\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/mqtt',\n";
		html += "type: 'POST',\n";
		html += "data: data,\n";
		html += "contentType: false,\n";
		html += "processData: false,\n";
		html += "success: function (data) {\n";
		html += "alert(\"Submited Successfully\");\n";
		html += "},\n";
		html += "error: function (data) {\n";
		html += "alert(\"An error occurred.\");\n";
		html += "}\n";
		html += "});\n";
		html += "});\n";
		html += "</script>\n";

		// html += "<h2>System Setting</h2>\n";
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromMQTT\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>MQTT Configuration</b></span></th>\n";
		html += "<tr>";

		String syncFlage = "";
		if (config.en_mqtt)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Address:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input  size=\"30\" maxlength=\"32\" name=\"host\" type=\"text\" value=\"" + htmlEncode(config.mqtt_host) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Port:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"5\"  maxlength=\"5\"  name=\"port\" type=\"number\" value=\"" + String(config.mqtt_port) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>User:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"32\" name=\"user\" type=\"text\" value=\"" + htmlEncode(config.mqtt_user) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Password:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"40\" maxlength=\"63\" name=\"pass\" type=\"password\" value=\"" + htmlEncode(config.mqtt_pass) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Topic:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"50\" maxlength=\"32\" id=\"topic\" name=\"topic\" type=\"text\" value=\"" + htmlEncode(config.mqtt_topic) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Topic Flag:</b></td>\n";

		html += "<td align=\"center\">\n";
		html += "<fieldset id=\"TopicGrp\">\n";
		html += "<legend>Topic Flags Send out MQTT</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		String topicFlageEn = "";
		if (config.mqtt_topic_flag & MQTT_TOPIC_TNC)
			topicFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"TopicTNC\" type=\"checkbox\" value=\"OK\" " + topicFlageEn + "/>TNC</td>\n";

		topicFlageEn = "";
		if (config.mqtt_topic_flag & MQTT_TOPIC_STATUS)
			topicFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"TopicSts\" type=\"checkbox\" value=\"OK\" " + topicFlageEn + "/>Status</td>\n";

		topicFlageEn = "";
		if (config.mqtt_topic_flag & MQTT_TOPIC_TELEMETRY)
			topicFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"TopicTlm\" type=\"checkbox\" value=\"OK\" " + topicFlageEn + "/>Telemetry</td>\n";

		topicFlageEn = "";
		if (config.mqtt_topic_flag & MQTT_TOPIC_WX)
			topicFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"TopicWX\" type=\"checkbox\" value=\"OK\" " + topicFlageEn + "/>Weather</td>\n";

		topicFlageEn = "";
		if (config.mqtt_topic_flag & MQTT_TOPIC_SENSOR)
			topicFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"TopicSensor\" type=\"checkbox\" value=\"OK\" " + topicFlageEn + "/>Sensor</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td></tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Subscription:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"50\" maxlength=\"32\" name=\"subscribe\" type=\"text\" value=\"" + htmlEncode(config.mqtt_subscribe) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Subscription Flag:</b></td>\n";

		html += "<td align=\"center\">\n";
		html += "<fieldset id=\"SubGrp\">\n";
		html += "<legend>Subscription Flags Receive</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		String subFlageEn = "";
		if (config.mqtt_subscribe_flag & MQTT_SUBSCRIBE_CMD)
			subFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"subCMD\" type=\"checkbox\" value=\"OK\" " + subFlageEn + "/>AT-Command</td>\n";

		subFlageEn = "";
		if (config.mqtt_subscribe_flag & MQTT_SUBSCRIBE_TNC)
			subFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"subTNC\" type=\"checkbox\" value=\"OK\" " + subFlageEn + "/>TNC</td>\n";

		subFlageEn = "";
		if (config.mqtt_subscribe_flag & MQTT_SUBSCRIBE_MESSAGE)
			subFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"subMsg\" type=\"checkbox\" value=\"OK\" " + subFlageEn + "/>Message</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td></tr>\n";

		html += "</table><br />\n";
		html += "<td><input class=\"button\" id=\"submitMQTT\" name=\"commitMQTT\" type=\"submit\" value=\"Save Config\" maxlength=\"80\"/></td>\n";
		html += "<input type=\"hidden\" name=\"commitMQTT\"/>\n";
		html += csrfField();
		html += "</form>\n";

		request->send(200, "text/html", html); // send to someones browser when asked
	}
}
#endif

void handle_msg(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	if (request->hasArg("commitChat"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		String toCall;
		String msg = "";
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "toCall")
			{
				if (request->arg(i) != "")
				{
					// strcpy(toCall, request->arg(i).c_str());
					toCall = request->arg(i);
				}
			}
			if (request->argName(i) == "msg")
			{
				if (request->arg(i) != "")
				{
					// strcpy(toCall, request->arg(i).c_str());
					msg = request->arg(i);
				}
			}
		}
		log_d("Chat to %s | msg %s", toCall.c_str(), msg.c_str());
		sendAPRSMessage(toCall, msg, config.msg_encrypt);
		String html = "Send completed";
		request->send(200, "text/html", html); // send to someones browser when asked
	}
	else if (request->hasArg("commitMSG"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool msgEn = false;
		bool msgRf = false;
		bool msgInet = false;
		bool msgEncrypt = false;

		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						msgEn = true;
				}
			}
			if (request->argName(i) == "msgRf")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						msgRf = true;
				}
			}
			if (request->argName(i) == "msgInet")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						msgInet = true;
				}
			}
			if (request->argName(i) == "encrypt")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						msgEncrypt = true;
				}
			}

			if (request->argName(i) == "mycall")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.msg_mycall, request->arg(i).c_str(), sizeof(config.msg_mycall));
					config.msg_mycall[9] = 0;
				}
			}

			if (request->argName(i) == "key")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.msg_key, request->arg(i).c_str(), sizeof(config.msg_key));
					config.msg_key[32] = 0;
				}
			}

			if (request->argName(i) == "retry")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.msg_retry = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "path")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.msg_path = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "timeout")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.msg_interval = request->arg(i).toInt();
				}
			}
		}

		config.msg_enable = msgEn;
		config.msg_rf = msgRf;
		config.msg_inet = msgInet;
		config.msg_encrypt = msgEncrypt;

		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else
	{

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "if(e.currentTarget.id===\"formMSG\") document.getElementById(\"submitMSG\").disabled=true;\n";
		// html += "if(e.currentTarget.id===\"formChat\") document.getElementById(\"submitI2C0\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/msg',\n";
		html += "type: 'POST',\n";
		html += "data: data,\n";
		html += "contentType: false,\n";
		html += "processData: false,\n";
		html += "success: function (data) {\n";
		html += "if(e.currentTarget.id===\"formMSG\") alert(\"Submited Successfully\");\n";
		html += "},\n";
		html += "error: function (data) {\n";
		html += "if(e.currentTarget.id===\"formMSG\") alert(\"An error occurred.\");\n";
		html += "}\n";
		html += "});\n";
		html += "});\n";

		// html += "if (!!window.EventSource) {";
		// html += "var source = new EventSource('/eventMsg');";

		// html += "source.addEventListener('open', function(e) {";
		// html += "console.log(\"Events MSG Connected\");";
		// html += "}, false);";
		// html += "source.addEventListener('error', function(e) {";
		// html += "if (e.target.readyState != EventSource.OPEN) {";
		// html += "console.log(\"Events MSG Disconnected\");";
		// html += "}\n}, false);";
		// html += "source.addEventListener('chatMsg', function(e) {";
		// // webString += "console.log(\"lastHeard\", e.data);";
		// html += "var lh=document.getElementById(\"chatMsg\");";
		// html += "if(lh != null) {lh.innerHTML = e.data;}";
		// html += "}, false);\n}";
		html += "</script>\n";

		// html += "<h2>System Setting</h2>\n";
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"formMSG\" method=\"post\">\n";
		html += "<table width=\"90%\">\n";
		html += "<th colspan=\"2\"><span><b>Message Configuration</b></span></th>\n";
		html += "<tr>";

		String syncFlage = "";
		if (config.msg_enable)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>My Callsign:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input  size=\"20\" maxlength=\"9\" name=\"mycall\" type=\"text\" value=\"" + String(config.msg_mycall) + "\" /> *<i>Callsign with SSID (Ex. HS5TQA-12)</i></td>\n";
		html += "</tr>\n";

		String msg2RFFlag = "";
		String msg2INETFlag = "";
		if (config.msg_rf)
			msg2RFFlag = "checked";
		if (config.msg_inet)
			msg2INETFlag = "checked";
		html += "<tr><td style=\"text-align: right;\"><b>TX Channel:</b></td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"msgRf\" value=\"OK\" " + msg2RFFlag + "/>RF <input type=\"checkbox\" name=\"msgInet\" value=\"OK\" " + msg2INETFlag + "/>Internet </td></tr>\n";

		html += "<tr>";
		syncFlage = "";
		if (config.msg_encrypt)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Encryption</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"encrypt\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>AES Key:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input  size=\"40\" maxlength=\"33\" name=\"key\" type=\"text\" value=\"" + htmlEncode(config.msg_key) + "\" /> *<i>ASCII HEX 16Byte</i></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Send Retry:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input  min=\"0\" max=\"99\"   name=\"retry\" type=\"number\" value=\"" + String(config.msg_retry) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Send Timeout:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input  min=\"1\" max=\"9999\"   name=\"timeout\" type=\"number\" value=\"" + String(config.msg_interval) + "\" /> Sec.</td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"path\" id=\"path\">\n";
		for (uint8_t pthIdx = 0; pthIdx < PATH_LEN; pthIdx++)
		{
			if (config.msg_path == pthIdx)
			{
				html += "<option value=\"" + String(pthIdx) + "\" selected>" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(pthIdx) + "\">" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
		}
		html += "</select></td>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitMSG'  name=\"commitMSG\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitMSG\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br /><br />";

		html += "<table width=\"90%\">\n";
		html += "<th style=\"background-color: #070ac2;\">CHAT MESSAGE</th>\n";

		html += "<tr><td>\n";
		html += "<table id=\"chatMsg\">\n";
		html += event_chatMessage(true);
		html += "</table>\n";

		html += "</td></tr><tr><td colspan=\"5\">";

		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"formChat\" method=\"post\">\n";
		html += "<table>\n";

		html += "<tr>\n";
		html += "<td align=\"left\"><b>TO:</b><input size=\"10\" name=\"toCall\" id=\"toCall\" type=\"text\" value=\"\" /> <b>MSG:</b><input size=\"80\" name=\"msg\" id=\"msg\" type=\"text\" value=\"\" /></td>\n";

		html += "<td align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitChat\" name=\"commitChat\" type=\"submit\" value=\"Send\"/>\n";
		html += "<input type=\"hidden\" name=\"commitChat\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form><br />\n";

		html += "</td></tr></table>";

		request->send(200, "text/html", html); // send to someones browser when asked		
	}
}

void handle_mod(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	if (request->hasArg("commitGNSS"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					// if (isValidNumber(request->arg(i)))
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "atc")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.gnss_at_command, request->arg(i).c_str(), sizeof(config.gnss_at_command));
				}
				else
				{
					memset(config.gnss_at_command, 0, sizeof(config.gnss_at_command));
				}
			}

			if (request->argName(i) == "Host")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.gnss_tcp_host, request->arg(i).c_str(), sizeof(config.gnss_tcp_host));
				}
			}

			if (request->argName(i) == "Port")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.gnss_tcp_port = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "channel")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.gnss_channel = request->arg(i).toInt();
				}
			}
		}

		config.gnss_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitUART0"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "baudrate")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart0_baudrate = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rx")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart0_rx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "tx")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart0_tx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rts")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart0_rts_gpio = request->arg(i).toInt();
				}
			}
		}

		config.uart0_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitUART1"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "baudrate")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart1_baudrate = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rx")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart1_rx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "tx")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart1_tx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rts")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart1_rts_gpio = request->arg(i).toInt();
				}
			}
		}

		config.uart1_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitUART2"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			Serial.print("SERVER ARGS ");
			Serial.print(request->argName(i));
			Serial.print("=");
			Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "baudrate")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart2_baudrate = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rx")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart2_rx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "tx")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.uart2_tx_gpio = request->arg(i).toInt();
				}
			}

			// if (request->argName(i) == "rts")
			// {
			// 	if (isValidNumber(request->arg(i)))
			// 	{
			// 		config.uart2_rts_gpio = request->arg(i).toInt();
			// 	}
			// }
		}

		config.uart2_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitMODBUS"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					// if (isValidNumber(request->arg(i)))
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "channel")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.modbus_channel = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "address")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.modbus_address = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "de")
			{
				if (request->arg(i) != "")
				{
					config.modbus_de_gpio = request->arg(i).toInt();
				}
			}
		}

		config.modbus_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitTNC"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					// if (isValidNumber(request->arg(i)))
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "channel")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.ext_tnc_channel = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "mode")
			{
				if (isValidNumber(request->arg(i)))
				{
					config.ext_tnc_mode = request->arg(i).toInt();
				}
			}
		}

		config.ext_tnc_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitONEWIRE"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					// if (isValidNumber(request->arg(i)))
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "data")
			{
				if (request->arg(i) != "")
				{
					config.onewire_gpio = request->arg(i).toInt();
				}
			}
		}

		config.onewire_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitRF"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "sql_active")
			{
				if (request->arg(i) != "")
				{
					config.rf_sql_active = (bool)request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "pd_active")
			{
				if (request->arg(i) != "")
				{
					config.rf_pd_active = (bool)request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "pwr_active")
			{
				if (request->arg(i) != "")
				{
					config.rf_pwr_active = (bool)request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "ptt_active")
			{
				if (request->arg(i) != "")
				{
					config.rf_ptt_active = (bool)request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "baudrate")
			{
				if (request->arg(i) != "")
				{
					config.rf_baudrate = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rx")
			{
				if (request->arg(i) != "")
				{
					config.rf_rx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "tx")
			{
				if (request->arg(i) != "")
				{
					config.rf_tx_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "pd")
			{
				if (request->arg(i) != "")
				{
					config.rf_pd_gpio = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "pwr")
			{
				if (request->arg(i) != "")
				{
					config.rf_pwr_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "ptt")
			{
				if (request->arg(i) != "")
				{
					config.rf_ptt_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "sql")
			{
				if (request->arg(i) != "")
				{
					config.rf_sql_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "atten")
			{
				if (request->arg(i) != "")
				{
					config.adc_atten = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "offset")
			{
				if (request->arg(i) != "")
				{
					config.adc_dc_offset = request->arg(i).toInt();
				}
			}
		}
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitI2C0"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "sda")
			{
				if (request->arg(i) != "")
				{
					config.i2c_sda_pin = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "sck")
			{
				if (request->arg(i) != "")
				{
					config.i2c_sck_pin = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "freq")
			{
				if (request->arg(i) != "")
				{
					config.i2c_freq = request->arg(i).toInt();
				}
			}
		}

		config.i2c_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitI2C1"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "sda")
			{
				if (request->arg(i) != "")
				{
					config.i2c1_sda_pin = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "sck")
			{
				if (request->arg(i) != "")
				{
					config.i2c1_sck_pin = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "freq")
			{
				if (request->arg(i) != "")
				{
					config.i2c1_freq = request->arg(i).toInt();
				}
			}
		}

		config.i2c1_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitCOUNTER0"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "gpio")
			{
				if (request->arg(i) != "")
				{
					config.counter0_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "active")
			{
				if (request->arg(i) != "")
				{
					config.counter0_active = (bool)request->arg(i).toInt();
				}
			}
		}

		config.counter0_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}
	else if (request->hasArg("commitCOUNTER1"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool En = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}

			if (request->argName(i) == "gpio")
			{
				if (request->arg(i) != "")
				{
					config.counter1_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "active")
			{
				if (request->arg(i) != "")
				{
					config.counter1_active = (bool)request->arg(i).toInt();
				}
			}
		}

		config.counter0_enable = En;
		saveConfiguration("/default.cfg", config);
		String html = "OK";
		request->send(200, "text/html", html);
	}else if (request->hasArg("commitCMD"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool mqtt = false;
		bool msg = false;
		bool bluetooth = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "mqtt")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						mqtt = true;
					}
				}
			}

			if (request->argName(i) == "msg")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						msg = true;
					}
				}
			}

			if (request->argName(i) == "bluetooth")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						bluetooth = true;
					}
				}
			}

			if (request->argName(i) == "uart")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.at_cmd_uart = request->arg(i).toInt();
				}
			}
		}
		config.at_cmd_mqtt = mqtt;
		config.at_cmd_msg = msg;
		config.at_cmd_bluetooth = bluetooth;
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	#ifdef PPPOS
	else if (request->hasArg("commitPPPoS"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool pppEn = false;
		bool pppGnss = false;
		bool pppNapt = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "pppEn")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						pppEn = true;
					}
				}
			}

			if (request->argName(i) == "pppGnss")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						pppGnss = true;
					}
				}
			}

			if (request->argName(i) == "pppNapt")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						pppNapt = true;
					}
				}
			}

			if (request->argName(i) == "pppAPN")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.ppp_apn, request->arg(i).c_str(), sizeof(config.ppp_apn));
				}
			}

			if (request->argName(i) == "pppPin")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.ppp_pin, request->arg(i).c_str(), sizeof(config.ppp_pin));
				}
			}

			if (request->argName(i) == "rstDly")
			{
				if (request->arg(i) != "")
				{
					config.ppp_rst_delay = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "baudrate")
			{
				if (request->arg(i) != "")
				{
					config.ppp_serial_baudrate = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "port")
			{
				if (request->arg(i) != "")
				{
					config.ppp_serial = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "rx")
			{
				if (request->arg(i) != "")
				{
					config.ppp_rx_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "tx")
			{
				if (request->arg(i) != "")
				{
					config.ppp_tx_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "rst")
			{
				if (request->arg(i) != "")
				{
					config.ppp_rst_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "rst_active")
			{
				if (request->arg(i) != "")
				{
					config.ppp_rst_active = (bool)request->arg(i).toInt();
				}
			}

			// if (request->argName(i) == "pppSerial")
			// {
			// 	if (request->arg(i) != "")
			// 	{
			// 		if (isValidNumber(request->arg(i)))
			// 			config.ppp_serial = request->arg(i).toInt();
			// 	}
			// }
		}
		config.ppp_enable = pppEn;
		config.ppp_gnss = pppGnss;
		config.ppp_napt = pppNapt;
		if (config.ppp_enable)
		{
			if (config.ppp_serial == 0)
			{
				config.uart0_enable = false;
			}
			else if (config.ppp_serial == 1)
			{
				config.uart1_enable = false;
			}
			else if (config.ppp_serial == 2)
			{
				config.uart2_enable = false;
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	#endif
	else
	{

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "if(e.currentTarget.id===\"formUART0\") document.getElementById(\"submitURAT0\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formUART1\") document.getElementById(\"submitURAT1\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formUART1\") document.getElementById(\"submitURAT1\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formGNSS\") document.getElementById(\"submitGNSS\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formMODBUS\") document.getElementById(\"submitMODBUS\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formTNC\") document.getElementById(\"submitTNC\").disabled=true;\n";
		// html += "if(e.currentTarget.id===\"formONEWIRE\") document.getElementById(\"submitONEWIRE\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formRF\") document.getElementById(\"submitRF\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formI2C0\") document.getElementById(\"submitI2C0\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formI2C1\") document.getElementById(\"submitI2C1\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formCOUNT0\") document.getElementById(\"submitCOUNT0\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formCOUNT1\") document.getElementById(\"submitCOUNT1\").disabled=true;\n";
		#ifdef PPPOS
		html += "if(e.currentTarget.id===\"formPPPoS\") document.getElementById(\"submitPPPoS\").disabled=true;\n";
		#endif
		html += "$.ajax({\n";
		html += "url: '/mod',\n";
		html += "type: 'POST',\n";
		html += "data: data,\n";
		html += "contentType: false,\n";
		html += "processData: false,\n";
		html += "success: function (data) {\n";
		html += "alert(\"Submited Successfully\\nRequire hardware RESET!\");\n";
		html += "},\n";
		html += "error: function (data) {\n";
		html += "alert(\"An error occurred.\");\n";
		html += "}\n";
		html += "});\n";
		html += "});\n";
		html += "</script>\n";

		html += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;vertical-align:top\"><td width=\"32%\" style=\"border:unset;\">";
		// html += "<h2>System Setting</h2>\n";
		/**************UART0(USB) Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromUART0\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>UART0 Modify</b></span></th>\n";
		html += "<tr>";

		String enFlage = "";
		if (config.uart0_enable)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>RX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"rx\" type=\"number\" value=\"" + String(config.uart0_rx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"tx\" type=\"number\" value=\"" + String(config.uart0_tx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>RTS/DE GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"rts\" type=\"number\" value=\"" + String(config.uart0_rts_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Baudrate:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"baudrate\" id=\"baudrate\">\n";
		for (int i = 0; i < 13; i++)
		{
			if (config.uart0_baudrate == baudrate[i])
				html += "<option value=\"" + String(baudrate[i]) + "\" selected>" + String(baudrate[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(baudrate[i]) + "\" >" + String(baudrate[i]) + " </option>\n";
		}
		html += "</select> bps\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitUART0\" name=\"commitUART0\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitUART0\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";

		html += "</form><br />\n";
		html += "</td><td width=\"32%\" style=\"border:unset;\">";

		/**************UART1 Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromUART1\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>UART1 Modify</b></span></th>\n";
		html += "<tr>";

		enFlage = "";
		if (config.uart1_enable)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>RX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"rx\" type=\"number\" value=\"" + String(config.uart1_rx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"tx\" type=\"number\" value=\"" + String(config.uart1_tx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>RTS/DE GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"rts\" type=\"number\" value=\"" + String(config.uart1_rts_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Baudrate:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"baudrate\" id=\"baudrate\">\n";
		for (int i = 0; i < 13; i++)
		{
			if (config.uart1_baudrate == baudrate[i])
				html += "<option value=\"" + String(baudrate[i]) + "\" selected>" + String(baudrate[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(baudrate[i]) + "\" >" + String(baudrate[i]) + " </option>\n";
		}
		html += "</select> bps\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitUART1\" name=\"commitUART1\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitUART1\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";

		html += "</form><br />\n";
		html += "</td><td width=\"32%\" style=\"border:unset;\">";

		/**************UART2 Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromUART2\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>UART2 Modify</b></span></th>\n";
		html += "<tr>";

		enFlage = "";
		if (config.uart2_enable)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>RX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"rx\" type=\"number\" value=\"" + String(config.uart2_rx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"tx\" type=\"number\" value=\"" + String(config.uart2_tx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>RTS/DE GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"rts\" type=\"number\" value=\"" + String(config.uart2_rts_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Baudrate:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"baudrate\" id=\"baudrate\">\n";
		for (int i = 0; i < 13; i++)
		{
			if (config.uart2_baudrate == baudrate[i])
				html += "<option value=\"" + String(baudrate[i]) + "\" selected>" + String(baudrate[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(baudrate[i]) + "\" >" + String(baudrate[i]) + " </option>\n";
		}
		html += "</select> bps\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitUART2\" name=\"commitUART2\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitUART2\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";

		html += "</form><br />\n";
		html += "</td></tr></table>\n";

		html += "</td><td width=\"32%\" style=\"border:unset;\">";

		/**************1-Wire Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromONEWIRE\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>1-Wire Bus Modify</b></span></th>\n";
		html += "<tr>";

		String syncFlage = "";
		if (config.onewire_enable)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"data\" type=\"number\" value=\"" + String(config.onewire_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitONEWIRE\" name=\"commitONEWIRE\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitONEWIRE\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form><br />\n";

		html += "</td></tr></table>\n";

		html += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;vertical-align:top\"><td width=\"50%\" style=\"border:unset;vertical-align:top\">";
		/**************RF GPIO******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromRF\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>RF GPIO Modify</b></span></th>\n";
		html += "<tr>";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>ADC Attenuation:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"atten\" id=\"atten\">\n";
		for (int i = 0; i < 5; i++)
		{
			if (config.adc_atten == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(ADC_ATTEN[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(ADC_ATTEN[i]) + " </option>\n";
		}
		html += "</select> DC-Offset: " + String(config.adc_dc_offset) + "mV\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>UART2 Baudrate:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"baudrate\" id=\"baudrate\">\n";
		for (int i = 0; i < 13; i++)
		{
			if (config.rf_baudrate == baudrate[i])
				html += "<option value=\"" + String(baudrate[i]) + "\" selected>" + String(baudrate[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(baudrate[i]) + "\" >" + String(baudrate[i]) + " </option>\n";
		}
		html += "</select> bps\n";
		html += "</td>\n";
		html += "</tr>\n";		

		// html += "<tr>\n";
		// html += "<td align=\"right\"><b>ADC DC OFFSET:</b></td>\n";
		// html += "<td style=\"text-align: left;\"><input min=\"100\" max=\"2500\" name=\"offset\" type=\"number\" value=\"" + String(config.adc_dc_offset) + "\" /> mV     (Current: " + String(offset) + " mV)</td>\n";
		// html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>UART2 RX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"rx\" type=\"number\" value=\"" + String(config.rf_rx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>UART2 TX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"tx\" type=\"number\" value=\"" + String(config.rf_tx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		String LowFlag = "", HighFlag = "";
		LowFlag = "";
		HighFlag = "";
		if (config.rf_pd_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PD GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"pd\" type=\"number\" value=\"" + String(config.rf_pd_gpio) + "\" /> Active:<input type=\"radio\" name=\"pd_active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"pd_active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		LowFlag = "";
		HighFlag = "";
		if (config.rf_pwr_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>H/L GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"pwr\" type=\"number\" value=\"" + String(config.rf_pwr_gpio) + "\" /> Active:<input type=\"radio\" name=\"pwr_active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"pwr_active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		LowFlag = "";
		HighFlag = "";
		if (config.rf_sql_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>SQL GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"sql\" type=\"number\" value=\"" + String(config.rf_sql_gpio) + "\" /> Active:<input type=\"radio\" name=\"sql_active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"sql_active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		LowFlag = "";
		HighFlag = "";
		if (config.rf_ptt_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PTT GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\"  name=\"ptt\" type=\"number\" value=\"" + String(config.rf_ptt_gpio) + "\" /> Active:<input type=\"radio\" name=\"ptt_active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"ptt_active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitRF\" name=\"commitRF\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitRF\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";

		html += "</td><td width=\"23%\" style=\"border:unset;\">";

		/**************I2C_0 Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromI2C0\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>I2C_0(OLED) Modify</b></span></th>\n";
		html += "<tr>";

		syncFlage = "";
		if (config.i2c_enable)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>SDA GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"sda\" type=\"number\" value=\"" + String(config.i2c_sda_pin) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>SCK GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"sck\" type=\"number\" value=\"" + String(config.i2c_sck_pin) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Frequency:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"1000\" max=\"800000\" name=\"freq\" type=\"number\" value=\"" + String(config.i2c_freq) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitI2C0\" name=\"commitI2C0\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitI2C0\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";

		/**************Counter_0 Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromCOUNTER0\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Counter_0 Modify</b></span></th>\n";
		html += "<tr>";

		syncFlage = "";
		if (config.counter0_enable)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>INPUT GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"gpio\" type=\"number\" value=\"" + String(config.counter0_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		LowFlag = "";
		HighFlag = "";
		if (config.counter0_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\">Active</td>\n";
		html += "<td style=\"text-align: left;\"><input type=\"radio\" name=\"active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitCOUNTER0\" name=\"commitCOUNTER0\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitCOUNTER0\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";

		html += "</td><td width=\"23%\" style=\"border:unset;\">";
		/**************I2C_1 Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromI2C1\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>I2C_1 Modify</b></span></th>\n";
		html += "<tr>";

		syncFlage = "";
		if (config.i2c1_enable)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>SDA GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"sda\" type=\"number\" value=\"" + String(config.i2c1_sda_pin) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>SCK GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"sck\" type=\"number\" value=\"" + String(config.i2c1_sck_pin) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Frequency:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"1000\" max=\"800000\" name=\"freq\" type=\"number\" value=\"" + String(config.i2c1_freq) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitI2C1\" name=\"commitI2C1\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitI2C1\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";

		/**************Counter_1 Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromCOUNTER1\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Counter_1 Modify</b></span></th>\n";
		html += "<tr>";

		syncFlage = "";
		if (config.counter1_enable)
			syncFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + syncFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>INPUT GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"gpio\" type=\"number\" value=\"" + String(config.counter1_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		LowFlag = "";
		HighFlag = "";
		if (config.counter1_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\">Active</td>\n";
		html += "<td style=\"text-align: left;\"><input type=\"radio\" name=\"active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitCOUNTER1\" name=\"commitCOUNTER1\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitCOUNTER1\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";

		html += "</td></tr></table>\n";
		html += "<br />\n";

		//******************
		html += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;vertical-align:top\"><td width=\"50%\" style=\"border:unset;vertical-align:top\">";
		/**************GNSS Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromGNSS\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>GNSS Modify</b></span></th>\n";
		html += "<tr>";

		enFlage = "";
		if (config.gnss_enable)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PORT:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"channel\" id=\"channel\">\n";
		for (int i = 0; i < 5; i++)
		{
			if (config.gnss_channel == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(GNSS_PORT[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(GNSS_PORT[i]) + " </option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<td align=\"right\"><b>AT Command:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"30\" size=\"20\" id=\"atc\" name=\"atc\" type=\"text\" value=\"" + htmlEncode(config.gnss_at_command) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<td align=\"right\"><b>TCP Host:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"20\" size=\"15\" id=\"Host\" name=\"Host\" type=\"text\" value=\"" + htmlEncode(config.gnss_tcp_host) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>TCP Port:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"1024\" max=\"65535\"  id=\"Port\" name=\"Port\" type=\"number\" value=\"" + String(config.gnss_tcp_port) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitGNSS\" name=\"commitGNSS\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitGNSS\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";

		html += "</form><br />\n";

		html += "</td><td width=\"23%\" style=\"border:unset;\">";

		/**************MODBUS Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromMODBUS\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>MODBUS Modify</b></span></th>\n";
		html += "<tr>";

		enFlage = "";
		if (config.modbus_enable)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PORT:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"channel\" id=\"channel\">\n";
		for (int i = 0; i < 5; i++)
		{
			if (config.modbus_channel == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(GNSS_PORT[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(GNSS_PORT[i]) + " </option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Address:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"address\" type=\"number\" value=\"" + String(config.modbus_address) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>DE:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\""+String(GPIO_NUM_MAX)+"\" name=\"de\" type=\"number\" value=\"" + String(config.modbus_de_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitMODBUS\" name=\"commitMODBUS\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitMODBUS\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";

		html += "</td><td width=\"23%\" style=\"border:unset;\">";

		/**************External TNC Modify******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"fromTNC\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>External TNC Modify</b></span></th>\n";
		html += "<tr>";

		enFlage = "";
		if (config.ext_tnc_enable)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PORT:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"channel\" id=\"channel\">\n";
		for (int i = 0; i < 5; i++)
		{
			if (config.ext_tnc_channel == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(TNC_PORT[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(TNC_PORT[i]) + " </option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>MODE:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"mode\" id=\"mode\">\n";
		for (int i = 0; i < 4; i++)
		{
			if (config.ext_tnc_mode == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(TNC_MODE[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(TNC_MODE[i]) + " </option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<input class=\"button\" id=\"submitTNC\" name=\"commitTNC\" type=\"submit\" value=\"Apply\" maxlength=\"80\"/>\n";
		html += "<input type=\"hidden\" name=\"commitTNC\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form>\n";
		html += "</td></tr></table>\n";
		html += "<br />\n";

		html += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;vertical-align:top\"><td width=\"50%\" style=\"border:unset;vertical-align:top\">";

		/************************ AT-COMMAND **************************/
		html += "<form id='formATCommand' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>AT-COMMAND CHANNEL</b></span></th>\n";
		html += "<tr>\n";
		html += "<td width=\"150\" align=\"right\"><b>MQTT:</b></td>\n";
		String cmdFlag = "";
		if (config.at_cmd_mqtt)
			cmdFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"mqtt\" value=\"OK\" " + cmdFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>MESSAGE:</b></td>\n";
		cmdFlag = "";
		if (config.at_cmd_msg)
			cmdFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"msg\" value=\"OK\" " + cmdFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>BLUETOOTH:</b></td>\n";
		cmdFlag = "";
		if (config.at_cmd_bluetooth)
			cmdFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"bluetooth\" value=\"OK\" " + cmdFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>UART PORT:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"uart\" id=\"cmdUart\">\n";
		for (int i = 0; i < 5; i++)
		{
			if (config.at_cmd_uart == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(TNC_PORT[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(TNC_PORT[i]) + " </option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitCMD'  name=\"commit\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitCMD\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";

		#ifdef PPPOS
		html += "<br />\n";

		html += "<table style=\"text-align:unset;border-width:0px;background:unset\"><tr style=\"background:unset;vertical-align:top\"><td width=\"50%\" style=\"border:unset;vertical-align:top\">";

		/************************ PPPoS **************************/

		html += "<form id='formPPPoS' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>PPP Over Serial (GSM/4G-LTE)</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String pppEnFlag = "";
		if (config.ppp_enable)
			pppEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"pppEn\" value=\"OK\" " + pppEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<td align=\"right\"><b>GNSS:</b></td>\n";
		pppEnFlag = "";
		if (config.ppp_gnss)
			pppEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"pppGnss\" value=\"OK\" " + pppEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>NAPT:</b></td>\n";
		pppEnFlag = "";
		if (config.ppp_napt)
			pppEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"pppNapt\" value=\"OK\" " + pppEnFlag + "><span class=\"slider round\"></span></label> *WiFi NAT</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>APN:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"20\" name=\"pppAPN\" type=\"text\" value=\"" + String(config.ppp_apn) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";

		html += "<td align=\"right\"><b>PIN:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"0\" max=\"999999\" name=\"pppPin\" type=\"number\" value=\"" + String(config.ppp_pin) + "\" /> <i>*PIN of SIM</i></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";

		html += "<td align=\"right\"><b>RX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\"50\" name=\"rx\" type=\"number\" value=\"" + String(config.ppp_rx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>TX GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\"50\" name=\"tx\" type=\"number\" value=\"" + String(config.ppp_tx_gpio) + "\" /></td>\n";
		html += "</tr>\n";

		LowFlag = "";
		HighFlag = "";
		if (config.ppp_rst_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>RESET GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\"50\"  name=\"rst\" type=\"number\" value=\"" + String(config.ppp_rst_gpio) + "\" /> Active:<input type=\"radio\" name=\"rst_active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"rst_active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		html += "<td align=\"right\"><b>RESET DELAY:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"0\" max=\"999999\" name=\"rstDly\" type=\"number\" value=\"" + String(config.ppp_rst_delay, DEC) + "\" /> mSec.</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PORT:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"port\" id=\"port\">\n";
		for (int i = 0; i < 2; i++)
		{
			if (config.ppp_serial == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(GNSS_PORT[i + 1]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(GNSS_PORT[i + 1]) + " </option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Baudrate:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"baudrate\" id=\"baudrate\">\n";
		for (int i = 0; i < 13; i++)
		{
			if (config.ppp_serial_baudrate == baudrate[i])
				html += "<option value=\"" + String(baudrate[i]) + "\" selected>" + String(baudrate[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(baudrate[i]) + "\" >" + String(baudrate[i]) + " </option>\n";
		}
		html += "</select> bps\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitPPPoS'  name=\"commitPPPoS\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitPPPoS\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form>";

		html += "</td></tr></table>\n";
		#endif
		if ((ESP.getFreeHeap() / 1000) > 120)
		{
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			size_t len = html.length();
			char *info = (char *)calloc(len, sizeof(char));
			if (info)
			{

				html.toCharArray(info, len, 0);
				html.clear();
				AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)info, len);

				response->addHeader("Sensor", "content");
				request->send(response);
				free(info);
			}
			else
			{
				log_d("Can't define calloc info size %d", len);
			}
		}
		//request->send(200, "text/html", html); // send to someones browser when asked
	}
}

void handle_system(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	if (request->hasArg("updateTimeZone"))
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "SetTimeZone")
			{
				if (request->arg(i) != "")
				{
					config.timeZone = request->arg(i).toFloat();
					// Serial.println("WEB Config Time Zone);
					configTime(3600 * config.timeZone, 0, config.ntp_host);
				}
				break;
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}else if (request->hasArg("updateHostName"))
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "SetHostName")
			{
				if (request->arg(i) != "")
				{
					// Serial.println("WEB Config NTP");
					strlcpy(config.host_name, request->arg(i).c_str(), sizeof(config.host_name));
				}
				break;
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("updateTimeNtp"))
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "SetTimeNtp")
			{
				if (request->arg(i) != "")
				{
					// Serial.println("WEB Config NTP");
					strlcpy(config.ntp_host, request->arg(i).c_str(), sizeof(config.ntp_host));
					configTime(3600 * config.timeZone, 0, config.ntp_host);
				}
				break;
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}else if (request->hasArg("updateAutoReset"))
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{

			if (request->argName(i) == "SetAutoReset")
			{
				if (request->arg(i) != "")
				{
					config.reset_timeout = request->arg(i).toInt();
				}
				break;
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("updateTime"))
	{
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "SetTime")
			{
				if (request->arg(i) != "")
				{
					// struct tm tmn;
					String date = getValue(request->arg(i), ' ', 0);
					String time = getValue(request->arg(i), ' ', 1);
					int yyyy = getValue(date, '-', 0).toInt();
					int mm = getValue(date, '-', 1).toInt();
					int dd = getValue(date, '-', 2).toInt();
					int hh = getValue(time, ':', 0).toInt();
					int ii = getValue(time, ':', 1).toInt();
					int ss = getValue(time, ':', 2).toInt();
					// int ss = 0;

					tmElements_t timeinfo;
					timeinfo.Year = yyyy - 1970;
					timeinfo.Month = mm;
					timeinfo.Day = dd;
					timeinfo.Hour = hh;
					timeinfo.Minute = ii;
					timeinfo.Second = ss;
					time_t timeStamp = makeTime(timeinfo);

					// tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec

					time_t rtc = timeStamp - (config.timeZone * 3600);
					timeval tv = {rtc, 0};
					timezone tz = {(0) + DST_MN, 0};
					settimeofday(&tv, &tz);

					// Serial.println("Update TIME " + request->arg(i));
					Serial.print("Set New Time at ");
					Serial.print(dd);
					Serial.print("/");
					Serial.print(mm);
					Serial.print("/");
					Serial.print(yyyy);
					Serial.print(" ");
					Serial.print(hh);
					Serial.print(":");
					Serial.print(ii);
					Serial.print(":");
					Serial.print(ss);
					Serial.print(" ");
					Serial.println(timeStamp);
				}
				break;
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("REBOOT"))
	{
		TLM_SEQ = 0;
		IGATE_TLM_SEQ = 0;
		DIGI_TLM_SEQ = 0;
		esp_restart();
	}
	else if (request->hasArg("Factory"))
	{
		defaultConfig();
	}
	else if (request->hasArg("LoadCFG"))
	{
		if (loadConfiguration("/default.cfg", config))
		{
			String html = "OK";
			request->send(200, "text/html", html);
		}
	}
	else if (request->hasArg("commitWebAuth"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "webauth_user")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.http_username, request->arg(i).c_str(), sizeof(config.http_username));
				}
			}
			if (request->argName(i) == "webauth_pass")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.http_password, request->arg(i).c_str(), sizeof(config.http_password));
				}
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("commitPath"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "path1")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.path[0], request->arg(i).c_str(), sizeof(config.path[0]));
				}
			}
			if (request->argName(i) == "path2")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.path[1], request->arg(i).c_str(), sizeof(config.path[1]));
				}
			}
			if (request->argName(i) == "path3")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.path[1], request->arg(i).c_str(), sizeof(config.path[1]));
				}
			}
			if (request->argName(i) == "path4")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.path[3], request->arg(i).c_str(), sizeof(config.path[3]));
				}
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("commitPWR"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool PwrEn = false;
		config.pwr_sleep_activate = 0;

		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "pwr_active")
			{
				if (request->arg(i) != "")
				{
					config.pwr_active = (bool)request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						PwrEn = true;
					}
				}
			}
			if (request->argName(i) == "pwr")
			{
				if (request->arg(i) != "")
				{
					config.pwr_gpio = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "sleep")
			{
				if (request->arg(i) != "")
				{
					config.pwr_sleep_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "stb")
			{
				if (request->arg(i) != "")
				{
					config.pwr_stanby_delay = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "mode")
			{
				if (request->arg(i) != "")
				{
					config.pwr_mode = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "FilterTelemetry")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_TELEMETRY;
				}
			}

			if (request->argName(i) == "FilterStatus")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_STATUS;
				}
			}

			if (request->argName(i) == "FilterWeather")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_WX;
				}
			}

			if (request->argName(i) == "FilterTracker")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_TRACKER;
				}
			}

			if (request->argName(i) == "FilterIGate")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_IGATE;
				}
			}

			if (request->argName(i) == "FilterDigi")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_DIGI;
				}
			}

			if (request->argName(i) == "FilterQuery")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_QUERY;
				}
			}

			if (request->argName(i) == "FilterWifi")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.pwr_sleep_activate |= ACTIVATE_WIFI;
				}
			}
		}
		config.pwr_en = PwrEn;
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("commitLOG"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool PwrEn = false;
		config.log = 0;

		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));

			if (request->argName(i) == "logStatus")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.log |= LOG_STATUS;
				}
			}

			if (request->argName(i) == "logWeather")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.log |= LOG_WX;
				}
			}

			if (request->argName(i) == "logTracker")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.log |= LOG_TRACKER;
				}
			}

			if (request->argName(i) == "logIgate")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.log |= LOG_IGATE;
				}
			}

			if (request->argName(i) == "logDigi")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.log |= LOG_DIGI;
				}
			}
		}
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else if (request->hasArg("commitDISP"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool dispRX = false;
		bool dispTX = false;
		bool dispRF = false;
		bool dispINET = false;
		bool oledEN = false;
		bool dispFlip = false;

		config.dispFilter = 0;

		for (uint8_t i = 0; i < request->args(); i++)
		{
			// Serial.print("SERVER ARGS ");
			// Serial.print(request->argName(i));
			// Serial.print("=");
			// Serial.println(request->arg(i));
			if (request->argName(i) == "oledEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						oledEN = true;
					}
				}
			}
			if (request->argName(i) == "dispFlip")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						dispFlip = true;
					}
				}
			}
			if (request->argName(i) == "filterMessage")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_MESSAGE;
				}
			}

			if (request->argName(i) == "filterTelemetry")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_TELEMETRY;
				}
			}

			if (request->argName(i) == "filterStatus")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_STATUS;
				}
			}

			if (request->argName(i) == "filterWeather")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_WX;
				}
			}

			if (request->argName(i) == "filterObject")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_OBJECT;
				}
			}

			if (request->argName(i) == "filterItem")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_ITEM;
				}
			}

			if (request->argName(i) == "filterQuery")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_QUERY;
				}
			}
			if (request->argName(i) == "filterBuoy")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_BUOY;
				}
			}
			if (request->argName(i) == "filterPosition")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.dispFilter |= FILTER_POSITION;
				}
			}

			if (request->argName(i) == "dispRF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						dispRF = true;
				}
			}

			if (request->argName(i) == "dispINET")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						dispINET = true;
				}
			}
			if (request->argName(i) == "txdispEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						dispTX = true;
				}
			}
			if (request->argName(i) == "rxdispEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						dispRX = true;
				}
			}

			if (request->argName(i) == "dispBright")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.disp_brightness = request->arg(i).toInt();
#ifdef ST7735_LED_K_Pin
						ledcWrite(0, (uint32_t)config.disp_brightness);
#endif
					}
				}
			}

			if (request->argName(i) == "dispDelay")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.dispDelay = request->arg(i).toInt();
						if (config.dispDelay < 0)
							config.dispDelay = 0;
					}
				}
			}

			if (request->argName(i) == "oled_timeout")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.oled_timeout = request->arg(i).toInt();
						if (config.oled_timeout < 0)
							config.oled_timeout = 0;
					}
				}
			}
			if (request->argName(i) == "filterDX")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
					{
						config.filterDistant = request->arg(i).toInt();
					}
				}
			}
		}

		#ifdef OLED
		if(oledEN && !config.oled_enable)
		{
			//display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false); // initialize with the I2C addr 0x3C (for the 128x64)
            // Initialising the UI will init the display too.
#ifdef SH1106
            display.begin(SH1106_SWITCHCAPVCC, SCREEN_ADDRESS, false);
#else
            display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false);
#endif
            display.clearDisplay();
		}
		config.oled_enable = oledEN;
		config.dispINET = dispINET;
		config.dispRF = dispRF;
		config.rx_display = dispRX;
		config.tx_display = dispTX;
		config.disp_flip = dispFlip;
		#endif // OLED
		// config.filterMessage = filterMessage;
		// config.filterStatus = filterStatus;
		// config.filterTelemetry = filterTelemetry;
		// config.filterWeather = filterWeather;
		// config.filterTracker = filterTracker;
		// config.filterMove = filterMove;
		// config.filterPosition = filterPosition;
		String html;
		if (saveConfiguration("/default.cfg", config))
		{
			html = "Setup completed successfully";
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			html = "Save config failed.";
			request->send(501, "text/html", html); // Not Implemented
		}
	}
	else
	{
		struct tm tmstruct;
		char strTime[20];
		tmstruct.tm_year = 0;
		getLocalTime(&tmstruct, 100);
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "if(e.currentTarget.id===\"formHostName\") document.getElementById(\"updateHostName\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formTime\") document.getElementById(\"updateTime\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formNTP\") document.getElementById(\"updateTimeNtp\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formTimeZone\") document.getElementById(\"updateTimeZone\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formReboot\") document.getElementById(\"REBOOT\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formDisp\") document.getElementById(\"submitDISP\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formWebAuth\") document.getElementById(\"submitWebAuth\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formPWR\") document.getElementById(\"submitPWR\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formPath\") document.getElementById(\"submitPath\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/system',\n";
		html += "type: 'POST',\n";
		html += "data: data,\n";
		html += "contentType: false,\n";
		html += "processData: false,\n";
		html += "success: function (data) {\n";
		html += "alert(\"Submited Successfully\");\n";
		html += "},\n";
		html += "error: function (data) {\n";
		html += "alert(\"An error occurred.\");\n";
		html += "}\n";
		html += "});\n";
		html += "});\n";
		html += "</script>\n";

		// html += "<h2>System Setting</h2>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>System Setting</b></span></th>\n";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\">Host Name:</td>\n";
		html += "<td style=\"text-align: left;\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formHostName\" method=\"post\"><input name=\"SetHostName\" type=\"text\" value=\"" + String(config.host_name) + "\" />\n";
		html += "<button type='submit' id='updateHostName'  name=\"commit\"> Apply </button>\n";
		html += "<input type=\"hidden\" name=\"updateHostName\"/></form>\n</td>\n";		
		html += "</tr>\n";
		html += "<tr>";
		// html += "<form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formTime\" method=\"post\">\n";
		html += "<td style=\"text-align: right;\">LOCAL DATE/TIME </td>\n";
		html += "<td style=\"text-align: left;\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formTime\" method=\"post\">\n<input name=\"SetTime\" type=\"text\" value=\"" + String(strTime) + "\" />\n";
		html += "<span class=\"input-group-addon\">\n<span class=\"glyphicon glyphicon-calendar\">\n</span></span>\n";
		// html += "<div class=\"col-sm-3 col-xs-6\"><button class=\"btn btn-primary\" data-args=\"[true]\" data-method=\"getDate\" type=\"button\" data-related-target=\"#SetTime\" />Get Date</button></div>\n";
		html += "<button type='submit' id='updateTime'  name=\"commit\"> Time Update </button>\n";
		html += "<input type=\"hidden\" name=\"updateTime\"/></form>\n</td>\n";
		// html += "<input class=\"btn btn-primary\" id=\"updateTime\" name=\"updateTime\" type=\"submit\" value=\"Time Update\" maxlength=\"80\"/></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td style=\"text-align: right;\">NTP Host </td>\n";
		html += "<td style=\"text-align: left;\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formNTP\" method=\"post\"><input name=\"SetTimeNtp\" type=\"text\" value=\"" + String(config.ntp_host) + "\" />\n";
		html += "<button type='submit' id='updateTimeNtp'  name=\"commit\"> NTP Update </button>\n";
		html += "<input type=\"hidden\" name=\"updateTimeNtp\"/></form>\n</td>\n";
		// html += "<input class=\"btn btn-primary\" id=\"updateTimeNtp\" name=\"updateTimeNtp\" type=\"submit\" value=\"NTP Update\" maxlength=\"80\"/></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td style=\"text-align: right;\">Auto REBOOT:</td>\n";
		html += "<td style=\"text-align: left;\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formAutoReset\" method=\"post\"><input  min=\"0\" max=\"65535\"  name=\"SetAutoReset\" type=\"number\" value=\"" + String(config.reset_timeout) + "\" /> Minutes\n";
		html += "<button type='submit' id='updateAutoReset'  name=\"commit\"> Update </button> *<i>0=No reset</i>\n";
		html += "<input type=\"hidden\" name=\"updateAutoReset\"/></form>\n</td>\n";
		// html += "<input class=\"button\" id=\"updateTimeNtp\" name=\"updateTimeNtp\" type=\"submit\" value=\"NTP Update\" maxlength=\"80\"/></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td style=\"text-align: right;\">Time Zone </td>\n";
		html += "<td style=\"text-align: left;\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formTimeZone\" method=\"post\">\n";
		html += "<select name=\"SetTimeZone\" id=\"SetTimeZone\">\n";
		for (int i = 0; i < 40; i++)
		{
			if (config.timeZone == tzList[i].tz)
				html += "<option value=\"" + String(tzList[i].tz, 1) + "\" selected>" + String(tzList[i].name) + " Sec</option>\n";
			else
				html += "<option value=\"" + String(tzList[i].tz, 1) + "\" >" + String(tzList[i].name) + " Sec</option>\n";
		}
		html += "</select>";
		html += "<button type='submit' id='updateTimeZone'  name=\"commit\"> TZ Update </button>\n";
		html += "<input type=\"hidden\" name=\"updateTimeZone\"/></form>\n</td>\n";
		// html += "<input class=\"btn btn-primary\" id=\"updateTimeZone\" name=\"updateTimeZone\" type=\"submit\" value=\"TZ Update\" maxlength=\"80\"/></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td style=\"text-align: right;\">SYSTEM CONTROL </td>\n";
		html += "<td style=\"text-align: left;\"><table><tr><td width=\"100\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formReboot\" method=\"post\"> <button type='submit' id='REBOOT'  name=\"commit\" style=\"background-color:red;color:white\"> REBOOT </button>\n";
		html += " <input type=\"hidden\" name=\"REBOOT\"/></form></td><td width=\"100\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formFactory\" method=\"post\"> <button type='submit' id='Factory'  name=\"commit\" style=\"background-color:orange;color:white\"> Factory Reset </button>\n";
		html += " <input type=\"hidden\" name=\"Factory\"/></form></td><td width=\"100\"><form accept-charset=\"UTF-8\" action=\"#\" enctype='multipart/form-data' id=\"formLoad\" method=\"post\"> <button type='submit' id='LoadCFG'  name=\"commit\" style=\"background-color:green;color:white\"> Load Default </button>\n";
		html += " <input type=\"hidden\" name=\"LoadCFG\"/></form></td></tr></table></td>\n";
		// html += "<td style=\"text-align: left;\"><input type='submit' class=\"btn btn-danger\" id=\"REBOOT\" name=\"REBOOT\" value='REBOOT'></td>\n";
		html += "</tr></table><br /><br />\n";

		/************************ WEB AUTH **************************/
		html += "<form id='formWebAuth' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Web Authentication</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Web USER:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"32\" maxlength=\"32\" class=\"form-control\" name=\"webauth_user\" type=\"text\" value=\"" + htmlEncode(config.http_username) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Web PASSWORD:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"63\" maxlength=\"63\" class=\"form-control\" name=\"webauth_pass\" type=\"password\" value=\"" + htmlEncode(config.http_password) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitWebAuth'  name=\"commit\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitWebAuth\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br /><br />";

		/**************Power Mode******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"formPWR\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Power Save Mode</b></span></th>\n";
		html += "<tr>";

		String enFlage = "";
		if (config.pwr_en)
			enFlage = "checked";
		html += "<td align=\"right\"><b>Enable</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + enFlage + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		String LowFlag = "", HighFlag = "";
		LowFlag = "";
		HighFlag = "";
		if (config.pwr_active)
			HighFlag = "checked=\"checked\"";
		else
			LowFlag = "checked=\"checked\"";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PWR GPIO:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"-1\" max=\"50\"  name=\"pwr\" type=\"number\" value=\"" + String(config.pwr_gpio) + "\" /> Output Active:<input type=\"radio\" name=\"pwr_active\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"pwr_active\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Sleep Interval:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"0\" max=\"9999\" name=\"sleep\" type=\"number\" value=\"" + String(config.pwr_sleep_interval) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>StandBy Delay:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"0\" max=\"9999\" name=\"stb\" type=\"number\" value=\"" + String(config.pwr_stanby_delay) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Power Mode:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"mode\" id=\"mode\">\n";
		for (int i = 0; i < 3; i++)
		{
			if (config.pwr_mode == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(PWR_MODE[i]) + " </option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(PWR_MODE[i]) + " </option>\n";
		}
		html += "</select> A=Reduce Speed(PWR Off),B=Light Sleep(WiFi/PWR Off),C=Deep Sleep(All Off)\n";
		html += "</td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Event Activate:</b><br/>(For Mode C)</td>\n";
		html += "<td style=\"text-align: left;\">";
		html += "<fieldset id=\"FilterGrp\">\n";
		html += "<legend>Events</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		String filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_TRACKER)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterTracker\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Tracker</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_STATUS)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterStatus\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Status</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_TELEMETRY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterTelemetry\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Telemetry</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_WX)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterWeather\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Weather</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_IGATE)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterIGate\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>IGate</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_DIGI)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterDigi\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Digi</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_QUERY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterQuery\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Query</td>\n";

		filterFlageEn = "";
		if (config.pwr_sleep_activate & ACTIVATE_WIFI)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterWifi\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>WiFi</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitPWR'  name=\"commitPWR\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitPWR\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";

		html += "</form><br /><br />\n";

		/**************Log File******************/
		html += "<form accept-charset=\"UTF-8\" action=\"#\" class=\"form-horizontal\" id=\"formLOG\" method=\"post\">\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Log File</b></span></th>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Activate:</b></td>\n";
		html += "<td style=\"text-align: left;\">";
		html += "<fieldset id=\"FilterGrp\">\n";
		html += "<legend>Events</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		filterFlageEn = "";
		if (config.log & LOG_TRACKER)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"logTracker\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Tracker</td>\n";

		filterFlageEn = "";
		if (config.log & LOG_IGATE)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"logIgate\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>IGate</td>\n";

		filterFlageEn = "";
		if (config.log & LOG_DIGI)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"logDigi\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>DIGI</td>\n";

		filterFlageEn = "";
		if (config.log & LOG_WX)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"logWeather\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Weather</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitLOG'  name=\"commitLOG\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitLOG\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";

		html += "</form><br /><br />\n";

		/************************ PATH USER define **************************/
		html += "<form id='formPath' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>PATH USER Define</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH_1:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"72\" maxlength=\"72\" class=\"form-control\" name=\"path1\" type=\"text\" value=\"" + String(config.path[0]) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH_2:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"72\" maxlength=\"72\" class=\"form-control\" name=\"path2\" type=\"text\" value=\"" + String(config.path[1]) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH_3:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"72\" maxlength=\"72\" class=\"form-control\" name=\"path3\" type=\"text\" value=\"" + String(config.path[2]) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH_4:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"72\" maxlength=\"72\" class=\"form-control\" name=\"path4\" type=\"text\" value=\"" + String(config.path[3]) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitPath'  name=\"commitPath\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitPath\"/>\n";
		html += csrfField();
		html += "</td></tr></table>\n";
		html += "</form><br /><br />";
		// delay(1);
// log_d("%s",html.c_str());
// log_d("Length: %d",html.length());
#if defined OLED || defined ST7735_160x80
		html += "<form id='formDisp' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		// html += "<h2>Display Setting</h2>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Display Setting</b></span></th>\n";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>OLED/TFT Enable</b></td>\n";
		String oledFlageEn = "";
		if (config.oled_enable == true)
			oledFlageEn = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"oledEnable\" value=\"OK\" " + oledFlageEn + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		oledFlageEn = "";
		if (config.disp_flip == true)
			oledFlageEn = "checked";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>Flip Rotate</b></td>\n";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"dispFlip\" value=\"OK\" " + oledFlageEn + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>TX Display</b></td>\n";
		String txdispFlageEn = "";
		if (config.tx_display == true)
			txdispFlageEn = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"txdispEnable\" value=\"OK\" " + txdispFlageEn + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"> <i>*All TX Packet for display affter filter.</i></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>RX Display</b></td>\n";
		String rxdispFlageEn = "";
		if (config.rx_display == true)
			rxdispFlageEn = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"rxdispEnable\" value=\"OK\" " + rxdispFlageEn + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"> <i>*All RX Packet for display affter filter.</i></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>Head Up</b></td>\n";
		String hupFlageEn = "";
		if (config.h_up == true)
			hupFlageEn = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"hupEnable\" value=\"OK\" " + hupFlageEn + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"> <i>*The compass will rotate in the direction of movement.</i></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>TFT Brightness</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"dispBright\" id=\"dispBright\">\n";
		for (int i = 0; i < 255; i += 25)
		{
			if (config.disp_brightness == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(i) + "</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(i) + "</option>\n";
		}
		html += "</select>\n";
		html += "</td></tr>\n";

		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>Popup Delay</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"dispDelay\" id=\"dispDelay\">\n";
		for (int i = 0; i < 16; i += 1)
		{
			if (config.dispDelay == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(i) + " Sec</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(i) + " Sec</option>\n";
		}
		html += "</select>\n";
		html += "</td></tr>\n";
		html += "<tr>\n";
		html += "<td style=\"text-align: right;\"><b>OLED/TFT Sleep</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"oled_timeout\" id=\"oled_timeout\">\n";
		for (int i = 0; i <= 600; i += 30)
		{
			if (config.oled_timeout == i)
				html += "<option value=\"" + String(i) + "\" selected>" + String(i) + " Sec</option>\n";
			else
				html += "<option value=\"" + String(i) + "\" >" + String(i) + " Sec</option>\n";
		}
		html += "</select>\n";
		html += "</td></tr>\n";
		String rfFlageEn = "";
		if (config.dispRF == true)
			rfFlageEn = "checked";
		String inetFlageEn = "";
		if (config.dispINET == true)
			inetFlageEn = "checked";
		html += "<tr><td style=\"text-align: right;\"><b>RX Channel</b></td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"dispRF\" value=\"OK\" " + rfFlageEn + "/>RF <input type=\"checkbox\" name=\"dispINET\" value=\"OK\" " + inetFlageEn + "/>Internet </td></tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Filter DX:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input type=\"number\" name=\"filterDX\" min=\"0\" max=\"9999\"\n";
		html += "step=\"1\" value=\"" + String(config.filterDistant) + "\" /> Km.  <label style=\"vertical-align: bottom;font-size: 8pt;\"> <i>*Value 0 is all distant allow.</i></label></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Filter:</b></td>\n";

		html += "<td align=\"center\">\n";
		html += "<fieldset id=\"filterDispGrp\">\n";
		html += "<legend>Filter popup display</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		// html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"dispTNC\" name=\"dispTNC\" type=\"checkbox\" value=\"OK\" " + rfFlageEn + "/>From RF</td>\n";

		// html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"dispINET\" name=\"dispINET\" type=\"checkbox\" value=\"OK\" " + inetFlageEn + "/>From INET</td>\n";

		String filterMessageFlageEn = "";
		if (config.dispFilter & FILTER_MESSAGE)
			filterMessageFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterMessage\" name=\"filterMessage\" type=\"checkbox\" value=\"OK\" " + filterMessageFlageEn + "/>Message</td>\n";

		String filterStatusFlageEn = "";
		if (config.dispFilter & FILTER_STATUS)
			filterStatusFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterStatus\" name=\"filterStatus\" type=\"checkbox\" value=\"OK\" " + filterStatusFlageEn + "/>Status</td>\n";

		String filterTelemetryFlageEn = "";
		if (config.dispFilter & FILTER_TELEMETRY)
			filterTelemetryFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterTelemetry\" name=\"filterTelemetry\" type=\"checkbox\" value=\"OK\" " + filterTelemetryFlageEn + "/>Telemetry</td>\n";

		String filterWeatherFlageEn = "";
		if (config.dispFilter & FILTER_WX)
			filterWeatherFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterWeather\" name=\"filterWeather\" type=\"checkbox\" value=\"OK\" " + filterWeatherFlageEn + "/>Weather</td>\n";

		String filterObjectFlageEn = "";
		if (config.dispFilter & FILTER_OBJECT)
			filterObjectFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterObject\" name=\"filterObject\" type=\"checkbox\" value=\"OK\" " + filterObjectFlageEn + "/>Object</td>\n";

		String filterItemFlageEn = "";
		if (config.dispFilter & FILTER_ITEM)
			filterItemFlageEn = "checked";
		html += "</tr><tr style=\"background:unset;\"><td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterItem\" name=\"filterItem\" type=\"checkbox\" value=\"OK\" " + filterItemFlageEn + "/>Item</td>\n";

		String filterQueryFlageEn = "";
		if (config.dispFilter & FILTER_QUERY)
			filterQueryFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterQuery\" name=\"filterQuery\" type=\"checkbox\" value=\"OK\" " + filterQueryFlageEn + "/>Query</td>\n";

		String filterBuoyFlageEn = "";
		if (config.dispFilter & FILTER_BUOY)
			filterBuoyFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterBuoy\" name=\"filterBuoy\" type=\"checkbox\" value=\"OK\" " + filterBuoyFlageEn + "/>Buoy</td>\n";

		String filterPositionFlageEn = "";
		if (config.dispFilter & FILTER_POSITION)
			filterPositionFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" id=\"filterPosition\" name=\"filterPosition\" type=\"checkbox\" value=\"OK\" " + filterPositionFlageEn + "/>Position</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";

		html += "</td></tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitDISP'  name=\"commitDISP\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitDISP\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
#endif

		if ((ESP.getFreeHeap() / 1000) > 120)
		{
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			size_t len = html.length();
			char *info = (char *)calloc(len, sizeof(char));
			if (info)
			{

				html.toCharArray(info, len, 0);
				html.clear();
				AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)info, len);

				response->addHeader("Sensor", "content");
				request->send(response);
				free(info);
			}
			else
			{
				log_d("Can't define calloc info size %d", len);
			}
		}
		// if ((ESP.getFreeHeap() / 1000) > 100)
		//{
		// request->send(200, "text/html", html); // send to someones browser when asked
		// }
		// else
		// {
		// 	AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)html.c_str(), html.length());
		// 	response->addHeader("System", "content");
		// 	request->send(response);
		// }
		// html.clear();
	}
}
