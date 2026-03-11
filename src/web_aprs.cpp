#include "webservice.h"
#include "pkg_list.h"

void handle_igate(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	bool aprsEn = false;
	bool rf2inetEn = false;
	bool inet2rfEn = false;
	bool posGPS = false;
	bool bcnEN = false;
	bool pos2RF = false;
	bool pos2INET = false;
	bool timeStamp = false;

	if (request->hasArg("commitIGATE"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }

		for (int i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "igateEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						aprsEn = true;
				}
			}
			if (request->argName(i) == "myCall")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					name.toUpperCase();
					strlcpy(config.aprs_mycall, name.c_str(), sizeof(config.aprs_mycall));
				}
			}
			if (request->argName(i) == "igateObject")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					strlcpy(config.igate_object, name.c_str(), sizeof(config.igate_object));
				}
				else
				{
					memset(config.igate_object, 0, sizeof(config.igate_object));
				}
			}
			if (request->argName(i) == "mySSID")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.aprs_ssid = request->arg(i).toInt();
					if (config.aprs_ssid > 15)
						config.aprs_ssid = 13;
				}
			}
			if (request->argName(i) == "igatePosInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "igateSTSInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_sts_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "igatePosLat")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_lat = request->arg(i).toFloat();
				}
			}

			if (request->argName(i) == "igatePosLon")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_lon = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "igatePosAlt")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_alt = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "igatePosSel")
			{
				if (request->arg(i) != "")
				{
					if (request->arg(i).toInt() == 1)
						posGPS = true;
				}
			}

			if (request->argName(i) == "igateTable")
			{
				if (request->arg(i) != "")
				{
					config.igate_symbol[0] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "igateSymbol")
			{
				if (request->arg(i) != "")
				{
					config.igate_symbol[1] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "aprsHost")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.aprs_host, request->arg(i).c_str(), sizeof(config.aprs_host));
				}
			}
			if (request->argName(i) == "aprsPort")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.aprs_port = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "aprsFilter")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.aprs_filter, request->arg(i).c_str(), sizeof(config.aprs_filter));
				}
			}
			if (request->argName(i) == "igatePath")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_path = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "igateComment")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.igate_comment, request->arg(i).c_str(), sizeof(config.igate_comment));
				}
				else
				{
					memset(config.igate_comment, 0, sizeof(config.igate_comment));
				}
			}
			if (request->argName(i) == "igateStatus")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.igate_status, request->arg(i).c_str(), sizeof(config.igate_status));
				}
				else
				{
					memset(config.igate_comment, 0, sizeof(config.igate_comment));
				}
			}
			if (request->argName(i) == "texttouse")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.igate_phg, request->arg(i).c_str(), sizeof(config.igate_phg));
				}
			}

			if (request->argName(i) == "rf2inetEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						rf2inetEn = true;
				}
			}
			if (request->argName(i) == "inet2rfEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						inet2rfEn = true;
				}
			}
			if (request->argName(i) == "igatePos2RF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2RF = true;
				}
			}
			if (request->argName(i) == "igatePos2INET")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2INET = true;
				}
			}
			if (request->argName(i) == "igateBcnEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						bcnEN = true;
				}
			}
			if (request->argName(i) == "igateTimeStamp")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						timeStamp = true;
				}
			}
			if (request->argName(i) == "igateTlmInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.igate_tlm_interval = request->arg(i).toInt();
				}
			}

			String arg;
			for (int x = 0; x < 5; x++)
			{
				arg = "sensorCH" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.igate_tlm_sensor[x] = request->arg(i).toInt();
				}
				arg = "param" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.igate_tlm_PARM[x], request->arg(i).c_str(), sizeof(config.igate_tlm_PARM[x]));
					}
				}
				arg = "unit" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.igate_tlm_UNIT[x], request->arg(i).c_str(), sizeof(config.igate_tlm_UNIT[x]));
					}
				}
				arg = "precision" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.igate_tlm_precision[x] = request->arg(i).toInt();
				}
				arg = "offset" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.igate_tlm_offset[x] = request->arg(i).toFloat();
				}
				for (int y = 0; y < 3; y++)
				{
					arg = "eqns" + String(x) + String((char)(y + 'a'));
					if (request->argName(i) == arg)
					{
						if (isValidNumber(request->arg(i)))
							config.igate_tlm_EQNS[x][y] = request->arg(i).toFloat();
					}
				}
			}
		}

		//waitISRetry = millis() + 10000; // Retry connect 5Sec
		config.igate_en = aprsEn;
		config.rf2inet = rf2inetEn;
		config.inet2rf = inet2rfEn;
		config.igate_gps = posGPS;
		config.igate_bcn = bcnEN;
		config.igate_loc2rf = pos2RF;
		config.igate_loc2inet = pos2INET;
		config.igate_timestamp = timeStamp;

		initInterval = true;
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
		aprsClient.stop();
	}
	else if (request->hasArg("commitIGATEfilter"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		config.rf2inetFilter = 0;
		config.inet2rfFilter = 0;
		for (int i = 0; i < request->args(); i++)
		{
			// config rf2inet filter
			if (request->argName(i) == "rf2inetFilterMessage")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_MESSAGE;
				}
			}

			if (request->argName(i) == "rf2inetFilterTelemetry")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_TELEMETRY;
				}
			}

			if (request->argName(i) == "rf2inetFilterStatus")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_STATUS;
				}
			}

			if (request->argName(i) == "rf2inetFilterWeather")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_WX;
				}
			}

			if (request->argName(i) == "rf2inetFilterObject")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_OBJECT;
				}
			}

			if (request->argName(i) == "rf2inetFilterItem")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_ITEM;
				}
			}

			if (request->argName(i) == "rf2inetFilterQuery")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_QUERY;
				}
			}
			if (request->argName(i) == "rf2inetFilterBuoy")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_BUOY;
				}
			}
			if (request->argName(i) == "rf2inetFilterPosition")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.rf2inetFilter |= FILTER_POSITION;
				}
			}
			// config inet2rf filter

			if (request->argName(i) == "inet2rfFilterMessage")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_MESSAGE;
				}
			}

			if (request->argName(i) == "inet2rfFilterTelemetry")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_TELEMETRY;
				}
			}

			if (request->argName(i) == "inet2rfFilterStatus")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_STATUS;
				}
			}

			if (request->argName(i) == "inet2rfFilterWeather")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_WX;
				}
			}

			if (request->argName(i) == "inet2rfFilterObject")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_OBJECT;
				}
			}

			if (request->argName(i) == "inet2rfFilterItem")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_ITEM;
				}
			}

			if (request->argName(i) == "inet2rfFilterQuery")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_QUERY;
				}
			}
			if (request->argName(i) == "inet2rfFilterBuoy")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_BUOY;
				}
			}
			if (request->argName(i) == "inet2rfFilterPosition")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.inet2rfFilter |= FILTER_POSITION;
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
	else
	{

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		// html += "document.getElementById(\"submitIGATE\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formIgate\") document.getElementById(\"submitIGATE\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formIgateFilter\") document.getElementById(\"submitIGATEfilter\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/igate',\n";
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
		html += "</script>\n<script type=\"text/javascript\">\n";

		html += "function openWindowSymbol() {\n";
		html += "var i, l, options = [{\n";
		html += "value: 'first',\n";
		html += "text: 'First'\n";
		html += "}, {\n";
		html += "value: 'second',\n";
		html += "text: 'Second'\n";
		html += "}],\n";
		html += "newWindow = window.open(\"/symbol\", null, \"height=400,width=400,status=no,toolbar=no,menubar=no,location=no\");\n";
		html += "}\n";

		html += "function setValue(symbol,table) {\n";
		html += "document.getElementById('igateSymbol').value = String.fromCharCode(symbol);\n";
		html += "if(table==1){\n document.getElementById('igateTable').value='/';\n";
		html += "}else if(table==2){\n document.getElementById('igateTable').value='\\\\';\n}\n";
		html += "document.getElementById('igateImgSymbol').src = \"http://aprs.dprns.com/symbols/icons/\"+symbol.toString()+'-'+table.toString()+'.png';\n";
		html += "\n}\n";
		html += "function calculatePHGR(){document.forms.formIgate.texttouse.value=\"PHG\"+calcPower(document.forms.formIgate.power.value)+calcHeight(document.forms.formIgate.haat.value)+calcGain(document.forms.formIgate.gain.value)+calcDirection(document.forms.formIgate.direction.selectedIndex)}function Log2(e){return Math.log(e)/Math.log(2)}function calcPerHour(e){return e<10?e:String.fromCharCode(65+(e-10))}function calcHeight(e){return String.fromCharCode(48+Math.round(Log2(e/10),0))}function calcPower(e){if(e<1)return 0;if(e>=1&&e<4)return 1;if(e>=4&&e<9)return 2;if(e>=9&&e<16)return 3;if(e>=16&&e<25)return 4;if(e>=25&&e<36)return 5;if(e>=36&&e<49)return 6;if(e>=49&&e<64)return 7;if(e>=64&&e<81)return 8;if(e>=81)return 9}function calcDirection(e){if(e==\"0\")return\"0\";if(e==\"1\")return\"1\";if(e==\"2\")return\"2\";if(e==\"3\")return\"3\";if(e==\"4\")return\"4\";if(e==\"5\")return\"5\";if(e==\"6\")return\"6\";if(e==\"7\")return\"7\";if(e==\"8\")return\"8\"}function calcGain(e){return e>9?\"9\":e<0?\"0\":Math.round(e,0)}\n";
		html += "function onRF2INETCheck() {\n";
		html += "if (document.querySelector('#rf2inetEnable').checked) {\n";
		// Checkbox has been checked
		html += "document.getElementById(\"rf2inetFilterGrp\").disabled=false;\n";
		html += "} else {\n";
		// Checkbox has been unchecked
		html += "document.getElementById(\"rf2inetFilterGrp\").disabled=true;\n";
		html += "}\n}\n";
		html += "function onINET2RFCheck() {\n";
		html += "if (document.querySelector('#inet2rfEnable').checked) {\n";
		// Checkbox has been checked
		html += "document.getElementById(\"inet2rfFilterGrp\").disabled=false;\n";
		html += "} else {\n";
		// Checkbox has been unchecked
		html += "document.getElementById(\"inet2rfFilterGrp\").disabled=true;\n";
		html += "}\n}\n";

		html += "function selPrecision(idx) {\n";
		html += "var x=0;\n";
		html += "x = document.getElementsByName(\"precision\"+idx)[0].value;\n";
		html += "document.getElementsByName(\"eqns\"+idx+\"b\")[0].value=1/Math.pow(10,x);\n";
		html += "}\n";
		html += "function selOffset(idx) {\n";
		html += "var x=0;\n";
		html += "x = document.getElementsByName(\"offset\"+idx)[0].value;\n";
		html += "document.getElementsByName(\"eqns\"+idx+\"c\")[0].value=x*(-1);\n";
		html += "}\n";
		html += "</script>\n";
		delay(1);
		/************************ IGATE Mode **************************/
		html += "<form id='formIgate' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		// html += "<h2>[IGATE] Internet Gateway Mode</h2>\n";
		html += "<table>\n";
		// html += "<tr>\n";
		// html += "<th width=\"200\"><span><b>Setting</b></span></th>\n";
		// html += "<th><span><b>Value</b></span></th>\n";
		// html += "</tr>\n";
		html += "<th colspan=\"2\"><span><b>[IGATE] Internet Gateway Mode</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String igateEnFlag = "";
		if (config.igate_en)
			igateEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"igateEnable\" value=\"OK\" " + igateEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station Callsign:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"7\" size=\"6\" id=\"myCall\" name=\"myCall\" type=\"text\" value=\"" + htmlEncode(config.aprs_mycall) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station SSID:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"mySSID\" id=\"mySSID\">\n";
		for (uint8_t ssid = 0; ssid <= 15; ssid++)
		{
			if (config.aprs_ssid == ssid)
			{
				html += "<option value=\"" + String(ssid) + "\" selected>" + String(ssid) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(ssid) + "\">" + String(ssid) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station Symbol:</b></td>\n";
		String table = "1";
		if (config.igate_symbol[0] == 47)
			table = "1";
		if (config.igate_symbol[0] == 92)
			table = "2";
		html += "<td style=\"text-align: left;\">Table:<input maxlength=\"1\" size=\"1\" id=\"igateTable\" name=\"igateTable\" type=\"text\" value=\"" + String(config.igate_symbol[0]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> Symbol:<input maxlength=\"1\" size=\"1\" id=\"igateSymbol\" name=\"igateSymbol\" type=\"text\" value=\"" + String(config.igate_symbol[1]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> <img border=\"1\" style=\"vertical-align: middle;\" id=\"igateImgSymbol\" onclick=\"openWindowSymbol();\" src=\"http://aprs.dprns.com/symbols/icons/" + String((int)config.igate_symbol[1]) + "-" + table + ".png\"> <i>*Click icon for select symbol</i></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Item/Obj Name:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"9\" size=\"9\" id=\"igateObject\" name=\"igateObject\" type=\"text\" value=\"" + htmlEncode(config.igate_object) + "\" /><i> *If not used, leave it blank.In use 3-9 charactor</i></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"igatePath\" id=\"igatePath\">\n";
		for (uint8_t pthIdx = 0; pthIdx < PATH_LEN; pthIdx++)
		{
			if (config.igate_path == pthIdx)
			{
				html += "<option value=\"" + String(pthIdx) + "\" selected>" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(pthIdx) + "\">" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		// html += "<td style=\"text-align: left;\"><input maxlength=\"72\" size=\"72\" id=\"igatePath\" name=\"igatePath\" type=\"text\" value=\"" + String(config.igate_path) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Host:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"20\" size=\"20\" id=\"aprsHost\" name=\"aprsHost\" type=\"text\" value=\"" + htmlEncode(config.aprs_host) + "\" /> *APRS-IS by T2THAI at <a href=\"http://aprs.dprns.com:14501\" target=\"_t2thai\">aprs.dprns.com:14580</a>,CBAPRS at <a href=\"http://aprs.dprns.com:24501\" target=\"_t2thai\">aprs.dprns.com:24580</a></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Port:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"1\" max=\"65535\" step=\"1\" id=\"aprsPort\" name=\"aprsPort\" type=\"number\" value=\"" + String(config.aprs_port) + "\" /> *AMPR Host at <a href=\"http://aprs.hs5tqa.ampr.org:14501\" target=\"_t2thai\">aprs.hs5tqa.ampr.org:14580</a></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Server Filter:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"30\" size=\"30\" id=\"aprsFilter\" name=\"aprsFilter\" type=\"text\" value=\"" + htmlEncode(config.aprs_filter) + "\" /> *Filter: <a target=\"_blank\" href=\"http://www.aprs-is.net/javAPRSFilter.aspx\">http://www.aprs-is.net/javAPRSFilter.aspx</a></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Comment:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"25\" size=\"30\" id=\"igateComment\" name=\"igateComment\" type=\"text\" value=\"" + htmlEncode(config.igate_comment) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Status:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"50\" size=\"60\" id=\"igateStatus\" name=\"igateStatus\" type=\"text\" value=\"" + htmlEncode(config.igate_status) + "\" />  Interval:<input min=\"0\" max=\"3600\" step=\"1\" name=\"igateSTSInv\" type=\"number\" value=\"" + String(config.igate_sts_interval) + "\" />Sec.</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>RF2INET:</b></td>\n";
		String rf2inetEnFlag = "";
		if (config.rf2inet)
			rf2inetEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" id=\"rf2inetEnable\" name=\"rf2inetEnable\" onclick=\"onRF2INETCheck()\" value=\"OK\" " + rf2inetEnFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"><i> *Switch RF to Internet gateway</i></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>INET2RF:</b></td>\n";
		String inet2rfEnFlag = "";
		if (config.inet2rf)
			inet2rfEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" id=\"inet2rfEnable\" name=\"inet2rfEnable\" onclick=\"onINET2RFCheck()\" value=\"OK\" " + inet2rfEnFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"><i> *Switch Internet to RF gateway</i></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Time Stamp:</b></td>\n";
		String timeStampFlag = "";
		if (config.igate_timestamp)
			timeStampFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"igateTimeStamp\" value=\"OK\" " + timeStampFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n<tr>";

		html += "<td align=\"right\"><b>POSITION:</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";
		String igateBcnEnFlag = "";
		if (config.igate_bcn)
			igateBcnEnFlag = "checked";

		html += "<tr><td style=\"text-align: right;\">Beacon:</td><td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"igateBcnEnable\" value=\"OK\" " + igateBcnEnFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\">  Interval:<input min=\"0\" max=\"3600\" step=\"1\" id=\"igatePosInv\" name=\"igatePosInv\" type=\"number\" value=\"" + String(config.igate_interval) + "\" />Sec.</label></td></tr>";
		String igatePosFixFlag = "";
		String igatePosGPSFlag = "";
		String igatePos2RFFlag = "";
		String igatePos2INETFlag = "";
		if (config.igate_gps)
			igatePosGPSFlag = "checked=\"checked\"";
		else
			igatePosFixFlag = "checked=\"checked\"";

		if (config.igate_loc2rf)
			igatePos2RFFlag = "checked";
		if (config.igate_loc2inet)
			igatePos2INETFlag = "checked";
		html += "<tr><td style=\"text-align: right;\">Location:</td><td style=\"text-align: left;\"><input type=\"radio\" name=\"igatePosSel\" value=\"0\" " + igatePosFixFlag + "/>Fix <input type=\"radio\" name=\"igatePosSel\" value=\"1\" " + igatePosGPSFlag + "/>GPS </td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">TX Channel:</td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"igatePos2RF\" value=\"OK\" " + igatePos2RFFlag + "/>RF <input type=\"checkbox\" name=\"igatePos2INET\" value=\"OK\" " + igatePos2INETFlag + "/>Internet </td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Latitude:</td><td style=\"text-align: left;\"><input min=\"-90\" max=\"90\" step=\"0.00001\" id=\"igatePosLat\" name=\"igatePosLat\" type=\"number\" value=\"" + String(config.igate_lat, 5) + "\" />degrees (positive for North, negative for South)</td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Longitude:</td><td style=\"text-align: left;\"><input min=\"-180\" max=\"180\" step=\"0.00001\" id=\"igatePosLon\" name=\"igatePosLon\" type=\"number\" value=\"" + String(config.igate_lon, 5) + "\" />degrees (positive for East, negative for West)</td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Altitude:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"10000\" step=\"0.1\" id=\"igatePosAlt\" name=\"igatePosAlt\" type=\"number\" value=\"" + String(config.igate_alt, 2) + "\" /> meter. *Value 0 is not send height</td></tr>\n";
		html += "</table></td>";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PHG:</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";
		html += "<tr>\n";
		html += "<td align=\"right\">Radio TX Power</td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"power\" id=\"power\">\n";
		html += "<option value=\"1\" selected>1</option>\n";
		html += "<option value=\"5\">5</option>\n";
		html += "<option value=\"10\">10</option>\n";
		html += "<option value=\"15\">15</option>\n";
		html += "<option value=\"25\">25</option>\n";
		html += "<option value=\"35\">35</option>\n";
		html += "<option value=\"50\">50</option>\n";
		html += "<option value=\"65\">65</option>\n";
		html += "<option value=\"80\">80</option>\n";
		html += "</select> Watts</td>\n";
		html += "</tr>\n";
		html += "<tr><td style=\"text-align: right;\">Antenna Gain</td><td style=\"text-align: left;\"><input size=\"3\" min=\"0\" max=\"100\" step=\"0.1\" id=\"gain\" name=\"gain\" type=\"number\" value=\"6\" /> dBi</td></tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\">Height</td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"haat\" id=\"haat\">\n";
		int k = 10;
		for (uint8_t w = 0; w < 10; w++)
		{
			if (w == 0)
			{
				html += "<option value=\"" + String(k) + "\" selected>" + String(k) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(k) + "\">" + String(k) + "</option>\n";
			}
			k += k;
		}
		html += "</select> Feet</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\">Antenna/Direction</td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"direction\" id=\"direction\">\n";
		html += "<option>Omni</option><option>NE</option><option>E</option><option>SE</option><option>S</option><option>SW</option><option>W</option><option>NW</option><option>N</option>\n";
		html += "</select></td>\n";
		html += "</tr>\n";

		html += "<tr><td align=\"right\"><b>PHG Text</b></td><td align=\"left\"><input name=\"texttouse\" type=\"text\" size=\"6\" style=\"background-color: rgb(97, 239, 170);\" value=\"" + htmlEncode(config.igate_phg) + "\"/> <input type=\"button\" value=\"Calculate PHG\" onclick=\"javascript:calculatePHGR()\" /></td></tr>\n";
		html += "</table></td>";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Telemetry:</b><br />(v=0->8280)</td>\n";
		html += "<td align=\"center\"><table>\n";
		html += "<tr><td style=\"text-align: right;\">Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"1000\" step=\"1\" id=\"igateTlmInv\" name=\"igateTlmInv\" type=\"number\" value=\"" + String(config.igate_tlm_interval) + "\" /> *Number of packets interval,<i>Example: 0 not send,1 send every packet</i></label></td></tr>";
		for (int ax = 0; ax < 5; ax++)
		{
			html += "<tr><td align=\"right\"><b>CH A" + String(ax + 1) + ":</b></td>\n";
			html += "<td align=\"center\">\n";
			html += "<table>";

			html += "<tr><td style=\"text-align: right;\">Sensor:</td>\n";
			html += "<td style=\"text-align: left;\">CH: ";
			html += "<select name=\"sensorCH" + String(ax) + "\" id=\"sensorCH" + String(ax) + "\">\n";
			for (uint8_t idx = 0; idx < 11; idx++)
			{
				if (idx == 0)
				{
					if (config.igate_tlm_sensor[ax] == idx)
					{
						html += "<option value=\"" + String(idx) + "\" selected>NONE</option>\n";
					}
					else
					{
						html += "<option value=\"" + String(idx) + "\">NONE</option>\n";
					}
				}
				else
				{
					if (config.igate_tlm_sensor[ax] == idx)
					{
						html += "<option value=\"" + String(idx) + "\" selected>SENSOR#" + String(idx) + "</option>\n";
					}
					else
					{
						html += "<option value=\"" + String(idx) + "\">SENSOR#" + String(idx) + "</option>\n";
					}
				}
			}
			html += "</select></td>\n";

			html += "<td style=\"text-align: left;\">Name: <input maxlength=\"10\" size=\"8\" name=\"param" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.igate_tlm_PARM[ax]) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Unit: <input maxlength=\"8\" size=\"5\" name=\"unit" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.igate_tlm_UNIT[ax]) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Precision: <input min=\"0\" max=\"5\" step=\"1\" type=\"number\" style=\"width: 2em\" name=\"precision" + String(ax) + "\" type=\"text\" value=\"" + String(config.igate_tlm_precision[ax]) + "\"  onchange=\"selPrecision(" + String(ax) + ")\"/></td></tr>\n";

			html += "<tr><td style=\"text-align: right;\">EQNS:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "a\" type=\"number\" value=\"" + String(config.igate_tlm_EQNS[ax][0], 5) + "\" />  b:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "b\" type=\"number\" value=\"" + String(config.igate_tlm_EQNS[ax][1], 5) + "\" /> c:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "c\" type=\"number\" value=\"" + String(config.igate_tlm_EQNS[ax][2], 5) + "\" /> (av<sup>2</sup>+bv+c) </td>\n";
			html += "<td style=\"text-align: left;\">Offset: <input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" type=\"number\" name=\"offset" + String(ax) + "\" type=\"text\" value=\"" + String(config.igate_tlm_offset[ax], 5) + "\" onchange=\"selOffset(" + String(ax) + ")\"/></td></tr>\n";
			html += "</table></td>";
			html += "</tr>\n";
		}
		html += "</table></td></tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitIGATE'  name=\"commitIGATE\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitIGATE\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br /><br />";

		html += "<form id='formIgateFilter' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>[IGATE] Filter</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>RF2INET Filter:</b></td>\n";

		html += "<td align=\"center\">\n";
		if (config.rf2inet)
			html += "<fieldset id=\"rf2inetFilterGrp\">\n";
		else
			html += "<fieldset id=\"rf2inetFilterGrp\" disabled>\n";
		html += "<legend>Filter RF to Internet</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		String filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_MESSAGE)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterMessage\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Message</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_STATUS)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterStatus\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Status</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_TELEMETRY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterTelemetry\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Telemetry</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_WX)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterWeather\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Weather</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_OBJECT)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterObject\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Object</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_ITEM)
			filterFlageEn = "checked";
		html += "</tr><tr style=\"background:unset;\"><td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterItem\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Item</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_QUERY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterQuery\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Query</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_BUOY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterBuoy\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Buoy</td>\n";

		filterFlageEn = "";
		if (config.rf2inetFilter & FILTER_POSITION)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"rf2inetFilterPosition\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Position</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td></tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>INET2RF Filter:</b></td>\n";

		html += "<td align=\"center\">\n";
		if (config.inet2rf)
			html += "<fieldset id=\"inet2rfFilterGrp\">\n";
		else
			html += "<fieldset id=\"inet2rfFilterGrp\" disabled>\n";
		html += "<legend>Filter Internet to RF</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_MESSAGE)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterMessage\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Message</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_STATUS)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterStatus\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Status</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_TELEMETRY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterTelemetry\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Telemetry</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_WX)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterWeather\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Weather</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_OBJECT)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterObject\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Object</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_ITEM)
			filterFlageEn = "checked";
		html += "</tr><tr style=\"background:unset;\"><td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterItem\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Item</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_QUERY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterQuery\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Query</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_BUOY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterBuoy\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Buoy</td>\n";

		filterFlageEn = "";
		if (config.inet2rfFilter & FILTER_POSITION)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"inet2rfFilterPosition\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Position</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td></tr>\n";

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitIGATEfilter'  name=\"commitIGATEfilter\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitIGATEfilter\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
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
		// request->send(200, "text/html", html); // send to someones browser when asked
		//  if ((ESP.getFreeHeap() / 1000) > 100)
		//  {
		//  	request->send(200, "text/html", html); // send to someones browser when asked
		//  }
		//  else
		//  {
		//  	AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)html.c_str(), html.length());
		//  	response->addHeader("IGate", "content");
		//  	request->send(response);
		//  }
		// html.clear();
	}
}

void handle_digi(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	bool digiEn = false;
	bool digiAuto = false;
	bool posGPS = false;
	bool bcnEN = false;
	bool pos2RF = false;
	bool pos2INET = false;
	bool timeStamp = false;

	if (request->hasArg("commitDIGI"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		config.digiFilter = 0;
		for (int i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "digiEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						digiEn = true;
				}
			}
			if (request->argName(i) == "digiAuto")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						digiAuto = true;
				}
			}
			if (request->argName(i) == "myCall")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					name.toUpperCase();
					strlcpy(config.digi_mycall, name.c_str(), sizeof(config.digi_mycall));
				}
			}
			if (request->argName(i) == "mySSID")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_ssid = request->arg(i).toInt();
					if (config.digi_ssid > 15)
						config.digi_ssid = 3;
				}
			}
			if (request->argName(i) == "digiDelay")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_delay = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "digiPosInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "digiSTSInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_sts_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "digiPosLat")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_lat = request->arg(i).toFloat();
				}
			}

			if (request->argName(i) == "digiPosLon")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_lon = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "digiPosAlt")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_alt = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "digiPosSel")
			{
				if (request->arg(i) != "")
				{
					if (request->arg(i).toInt() == 1)
						posGPS = true;
				}
			}

			if (request->argName(i) == "digiTable")
			{
				if (request->arg(i) != "")
				{
					config.digi_symbol[0] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "digiSymbol")
			{
				if (request->arg(i) != "")
				{
					config.digi_symbol[1] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "digiPath")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_path = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "digiComment")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.digi_comment, request->arg(i).c_str(), sizeof(config.digi_comment));
				}
				else
				{
					memset(config.digi_comment, 0, sizeof(config.digi_comment));
				}
			}
			if (request->argName(i) == "texttouse")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.digi_phg, request->arg(i).c_str(), sizeof(config.digi_phg));
				}
			}
			if (request->argName(i) == "digiStatus")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.digi_status, request->arg(i).c_str(), sizeof(config.digi_status));
				}
			}
			if (request->argName(i) == "digiPos2RF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2RF = true;
				}
			}
			if (request->argName(i) == "digiPos2INET")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2INET = true;
				}
			}
			if (request->argName(i) == "digiBcnEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						bcnEN = true;
				}
			}
			// Filter
			if (request->argName(i) == "FilterMessage")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_MESSAGE;
				}
			}

			if (request->argName(i) == "FilterTelemetry")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_TELEMETRY;
				}
			}

			if (request->argName(i) == "FilterStatus")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_STATUS;
				}
			}

			if (request->argName(i) == "FilterWeather")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_WX;
				}
			}

			if (request->argName(i) == "FilterObject")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_OBJECT;
				}
			}

			if (request->argName(i) == "FilterItem")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_ITEM;
				}
			}

			if (request->argName(i) == "FilterQuery")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_QUERY;
				}
			}
			if (request->argName(i) == "FilterBuoy")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_BUOY;
				}
			}
			if (request->argName(i) == "FilterPosition")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						config.digiFilter |= FILTER_POSITION;
				}
			}
			if (request->argName(i) == "digiTimeStamp")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						timeStamp = true;
				}
			}
			if (request->argName(i) == "digiTlmInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.digi_tlm_interval = request->arg(i).toInt();
				}
			}

			String arg;
			for (int x = 0; x < 5; x++)
			{
				arg = "sensorCH" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.digi_tlm_sensor[x] = request->arg(i).toInt();
				}
				arg = "param" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.digi_tlm_PARM[x], request->arg(i).c_str(), sizeof(config.digi_tlm_PARM[x]));
					}
				}
				arg = "unit" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.digi_tlm_UNIT[x], request->arg(i).c_str(), sizeof(config.digi_tlm_UNIT[x]));
					}
				}
				arg = "precision" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.digi_tlm_precision[x] = request->arg(i).toInt();
				}
				arg = "offset" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.digi_tlm_offset[x] = request->arg(i).toFloat();
				}
				for (int y = 0; y < 3; y++)
				{
					arg = "eqns" + String(x) + String((char)(y + 'a'));
					if (request->argName(i) == arg)
					{
						if (isValidNumber(request->arg(i)))
							config.digi_tlm_EQNS[x][y] = request->arg(i).toFloat();
					}
				}
			}
		}
		config.digi_en = digiEn;
		config.digi_auto = digiAuto;
		config.digi_gps = posGPS;
		config.digi_bcn = bcnEN;
		config.digi_loc2rf = pos2RF;
		config.digi_loc2inet = pos2INET;
		config.digi_timestamp = timeStamp;

		initInterval = true;
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
		html += "document.getElementById(\"submitDIGI\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/digi',\n";
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
		html += "</script>\n<script type=\"text/javascript\">\n";
		html += "function openWindowSymbol() {\n";
		html += "var i, l, options = [{\n";
		html += "value: 'first',\n";
		html += "text: 'First'\n";
		html += "}, {\n";
		html += "value: 'second',\n";
		html += "text: 'Second'\n";
		html += "}],\n";
		html += "newWindow = window.open(\"/symbol\", null, \"height=400,width=400,status=no,toolbar=no,menubar=no,titlebar=no,location=no\");\n";
		html += "}\n";

		html += "function setValue(symbol,table) {\n";
		html += "document.getElementById('digiSymbol').value = String.fromCharCode(symbol);\n";
		html += "if(table==1){\n document.getElementById('digiTable').value='/';\n";
		html += "}else if(table==2){\n document.getElementById('digiTable').value='\\\\';\n}\n";
		html += "document.getElementById('digiImgSymbol').src = \"http://aprs.dprns.com/symbols/icons/\"+symbol.toString()+'-'+table.toString()+'.png';\n";
		html += "\n}\n";
		html += "function calculatePHGR(){document.forms.formDIGI.texttouse.value=\"PHG\"+calcPower(document.forms.formDIGI.power.value)+calcHeight(document.forms.formDIGI.haat.value)+calcGain(document.forms.formDIGI.gain.value)+calcDirection(document.forms.formDIGI.direction.selectedIndex)}function Log2(e){return Math.log(e)/Math.log(2)}function calcPerHour(e){return e<10?e:String.fromCharCode(65+(e-10))}function calcHeight(e){return String.fromCharCode(48+Math.round(Log2(e/10),0))}function calcPower(e){if(e<1)return 0;if(e>=1&&e<4)return 1;if(e>=4&&e<9)return 2;if(e>=9&&e<16)return 3;if(e>=16&&e<25)return 4;if(e>=25&&e<36)return 5;if(e>=36&&e<49)return 6;if(e>=49&&e<64)return 7;if(e>=64&&e<81)return 8;if(e>=81)return 9}function calcDirection(e){if(e==\"0\")return\"0\";if(e==\"1\")return\"1\";if(e==\"2\")return\"2\";if(e==\"3\")return\"3\";if(e==\"4\")return\"4\";if(e==\"5\")return\"5\";if(e==\"6\")return\"6\";if(e==\"7\")return\"7\";if(e==\"8\")return\"8\"}function calcGain(e){return e>9?\"9\":e<0?\"0\":Math.round(e,0)}\n";
		html += "function selPrecision(idx) {\n";
		html += "var x=0;\n";
		html += "x = document.getElementsByName(\"precision\"+idx)[0].value;\n";
		html += "document.getElementsByName(\"eqns\"+idx+\"b\")[0].value=1/Math.pow(10,x);\n";
		html += "}\n";
		html += "function selOffset(idx) {\n";
		html += "var x=0;\n";
		html += "x = document.getElementsByName(\"offset\"+idx)[0].value;\n";
		html += "document.getElementsByName(\"eqns\"+idx+\"c\")[0].value=x*(-1);\n";
		html += "}\n";
		html += "</script>\n";

		/************************ DIGI Mode **************************/
		html += "<form id='formDIGI' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		// html += "<h2>[DIGI] Digital Repeater Mode</h2>\n";
		html += "<table>\n";
		// html += "<tr>\n";
		// html += "<th width=\"200\"><span><b>Setting</b></span></th>\n";
		// html += "<th><span><b>Value</b></span></th>\n";
		// html += "</tr>\n";
		html += "<th colspan=\"2\"><span><b>[DIGI] Digital Repeater Mode</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String digiEnFlag = "";
		if (config.digi_en)
			digiEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"digiEnable\" value=\"OK\" " + digiEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Auto Enable:</b></td>\n";
		String digiAutoFlag = "";
		if (config.digi_auto)
			digiAutoFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"digiAuto\" value=\"OK\" " + digiAutoFlag + "><span class=\"slider round\"></span></label> <i>*Automatic enable when APRS-IS disconnected</i></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station Callsign:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"7\" size=\"6\" id=\"myCall\" name=\"myCall\" type=\"text\" value=\"" + htmlEncode(config.digi_mycall) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station SSID:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"mySSID\" id=\"mySSID\">\n";
		for (uint8_t ssid = 0; ssid <= 15; ssid++)
		{
			if (config.digi_ssid == ssid)
			{
				html += "<option value=\"" + String(ssid) + "\" selected>" + String(ssid) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(ssid) + "\">" + String(ssid) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station Symbol:</b></td>\n";
		String table = "1";
		if (config.digi_symbol[0] == 47)
			table = "1";
		if (config.digi_symbol[0] == 92)
			table = "2";
		html += "<td style=\"text-align: left;\">Table:<input maxlength=\"1\" size=\"1\" id=\"digiTable\" name=\"digiTable\" type=\"text\" value=\"" + String(config.digi_symbol[0]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> Symbol:<input maxlength=\"1\" size=\"1\" id=\"digiSymbol\" name=\"digiSymbol\" type=\"text\" value=\"" + String(config.digi_symbol[1]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> <img border=\"1\" style=\"vertical-align: middle;\" id=\"digiImgSymbol\" onclick=\"openWindowSymbol();\" src=\"http://aprs.dprns.com/symbols/icons/" + String((int)config.digi_symbol[1]) + "-" + table + ".png\"> <i>*Click icon for select symbol</i></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"digiPath\" id=\"digiPath\">\n";
		for (uint8_t pthIdx = 0; pthIdx < PATH_LEN; pthIdx++)
		{
			if (config.digi_path == pthIdx)
			{
				html += "<option value=\"" + String(pthIdx) + "\" selected>" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(pthIdx) + "\">" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		// html += "<td style=\"text-align: left;\"><input maxlength=\"72\" size=\"72\" id=\"digiPath\" name=\"digiPath\" type=\"text\" value=\"" + String(config.digi_path) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Comment:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"25\" size=\"30\" id=\"digiComment\" name=\"digiComment\" type=\"text\" value=\"" + htmlEncode(config.digi_comment) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Status:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"50\" size=\"60\" id=\"digiStatus\" name=\"digiStatus\" type=\"text\" value=\"" + htmlEncode(config.digi_status) + "\" />  Interval:<input min=\"0\" max=\"3600\" step=\"1\" name=\"digiSTSInv\" type=\"number\" value=\"" + String(config.digi_sts_interval) + "\" />Sec.</td>\n";
		html += "</tr>\n";

		html += "<tr><td style=\"text-align: right;\"><b>Repeat Delay:</b></td><td style=\"text-align: left;\"><input min=\"0\" max=\"10000\" step=\"100\" id=\"digiDelay\" name=\"digiDelay\" type=\"number\" value=\"" + String(config.digi_delay) + "\" /> mSec. <i>*0 is auto,Other random of delay time</i></td></tr>";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Time Stamp:</b></td>\n";
		String timeStampFlag = "";
		if (config.digi_timestamp)
			timeStampFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"digiTimeStamp\" value=\"OK\" " + timeStampFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr><td align=\"right\"><b>POSITION:</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";
		String digiBcnEnFlag = "";
		if (config.digi_bcn)
			digiBcnEnFlag = "checked";

		html += "<tr><td style=\"text-align: right;\">Beacon:</td><td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"digiBcnEnable\" value=\"OK\" " + digiBcnEnFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\">  Interval:<input min=\"0\" max=\"3600\" step=\"1\" id=\"digiPosInv\" name=\"digiPosInv\" type=\"number\" value=\"" + String(config.digi_interval) + "\" />Sec.</label></td></tr>";
		String digiPosFixFlag = "";
		String digiPosGPSFlag = "";
		String digiPos2RFFlag = "";
		String digiPos2INETFlag = "";
		if (config.digi_gps)
			digiPosGPSFlag = "checked=\"checked\"";
		else
			digiPosFixFlag = "checked=\"checked\"";

		if (config.digi_loc2rf)
			digiPos2RFFlag = "checked";
		if (config.digi_loc2inet)
			digiPos2INETFlag = "checked";
		html += "<tr><td style=\"text-align: right;\">Location:</td><td style=\"text-align: left;\"><input type=\"radio\" name=\"digiPosSel\" value=\"0\" " + digiPosFixFlag + "/>Fix <input type=\"radio\" name=\"digiPosSel\" value=\"1\" " + digiPosGPSFlag + "/>GPS </td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">TX Channel:</td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"digiPos2RF\" value=\"OK\" " + digiPos2RFFlag + "/>RF <input type=\"checkbox\" name=\"digiPos2INET\" value=\"OK\" " + digiPos2INETFlag + "/>Internet </td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Latitude:</td><td style=\"text-align: left;\"><input min=\"-90\" max=\"90\" step=\"0.00001\" id=\"digiPosLat\" name=\"digiPosLat\" type=\"number\" value=\"" + String(config.digi_lat, 5) + "\" />degrees (positive for North, negative for South)</td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Longitude:</td><td style=\"text-align: left;\"><input min=\"-180\" max=\"180\" step=\"0.00001\" id=\"digiPosLon\" name=\"digiPosLon\" type=\"number\" value=\"" + String(config.digi_lon, 5) + "\" />degrees (positive for East, negative for West)</td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Altitude:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"10000\" step=\"0.1\" id=\"digiPosAlt\" name=\"digiPosAlt\" type=\"number\" value=\"" + String(config.digi_alt, 2) + "\" /> meter. *Value 0 is not send height</td></tr>\n";
		html += "</table></td>";
		html += "</tr>\n";
		delay(1);
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PHG:</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";
		html += "<tr>\n";
		html += "<td align=\"right\">Radio TX Power</td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"power\" id=\"power\">\n";
		html += "<option value=\"1\" selected>1</option>\n";
		html += "<option value=\"5\">5</option>\n";
		html += "<option value=\"10\">10</option>\n";
		html += "<option value=\"15\">15</option>\n";
		html += "<option value=\"25\">25</option>\n";
		html += "<option value=\"35\">35</option>\n";
		html += "<option value=\"50\">50</option>\n";
		html += "<option value=\"65\">65</option>\n";
		html += "<option value=\"80\">80</option>\n";
		html += "</select> Watts</td>\n";
		html += "</tr>\n";
		html += "<tr><td style=\"text-align: right;\">Antenna Gain</td><td style=\"text-align: left;\"><input size=\"3\" min=\"0\" max=\"100\" step=\"0.1\" id=\"gain\" name=\"gain\" type=\"number\" value=\"6\" /> dBi</td></tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\">Height</td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"haat\" id=\"haat\">\n";
		int k = 10;
		for (uint8_t w = 0; w < 10; w++)
		{
			if (w == 0)
			{
				html += "<option value=\"" + String(k) + "\" selected>" + String(k) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(k) + "\">" + String(k) + "</option>\n";
			}
			k += k;
		}
		html += "</select> Feet</td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\">Antenna/Direction</td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"direction\" id=\"direction\">\n";
		html += "<option>Omni</option><option>NE</option><option>E</option><option>SE</option><option>S</option><option>SW</option><option>W</option><option>NW</option><option>N</option>\n";
		html += "</select></td>\n";
		html += "</tr>\n";
		html += "<tr><td align=\"right\"><b>PHG Text</b></td><td align=\"left\"><input name=\"texttouse\" type=\"text\" size=\"6\" style=\"background-color: rgb(97, 239, 170);\" value=\"" + htmlEncode(config.digi_phg) + "\"/> <input type=\"button\" value=\"Calculate PHG\" onclick=\"javascript:calculatePHGR()\" /></td></tr>\n";

		html += "</table></tr>";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Filter:</b></td>\n";

		html += "<td align=\"center\">\n";
		html += "<fieldset id=\"FilterGrp\">\n";
		html += "<legend>Filter repeater</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
		html += "<tr style=\"background:unset;\">";

		String filterFlageEn = "";
		if (config.digiFilter & FILTER_MESSAGE)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterMessage\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Message</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_STATUS)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterStatus\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Status</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_TELEMETRY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterTelemetry\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Telemetry</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_WX)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterWeather\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Weather</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_OBJECT)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterObject\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Object</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_ITEM)
			filterFlageEn = "checked";
		html += "</tr><tr style=\"background:unset;\"><td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterItem\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Item</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_QUERY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterQuery\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Query</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_BUOY)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterBuoy\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Buoy</td>\n";

		filterFlageEn = "";
		if (config.digiFilter & FILTER_POSITION)
			filterFlageEn = "checked";
		html += "<td style=\"border:unset;\"><input class=\"field_checkbox\" name=\"FilterPosition\" type=\"checkbox\" value=\"OK\" " + filterFlageEn + "/>Position</td>\n";

		html += "<td style=\"border:unset;\"></td>";
		html += "</tr></table></fieldset>\n";
		html += "</td></tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Telemetry:</b><br />(v=0->8280)</td>\n";
		html += "<td align=\"center\"><table>\n";
		html += "<tr><td style=\"text-align: right;\">Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"1000\" step=\"1\" id=\"digiTlmInv\" name=\"digiTlmInv\" type=\"number\" value=\"" + String(config.digi_tlm_interval) + "\" /> *Number of packets interval,<i>Example: 0 not send,1 send every packet</i></label></td></tr>";
		for (int ax = 0; ax < 5; ax++)
		{
			html += "<tr><td align=\"right\"><b>CH A" + String(ax + 1) + ":</b></td>\n";
			html += "<td align=\"center\">\n";
			html += "<table>";

			html += "<tr><td style=\"text-align: right;\">Sensor:</td>\n";
			html += "<td style=\"text-align: left;\">CH: ";
			html += "<select name=\"sensorCH" + String(ax) + "\" id=\"sensorCH" + String(ax) + "\">\n";
			for (uint8_t idx = 0; idx < 11; idx++)
			{
				if (idx == 0)
				{
					if (config.digi_tlm_sensor[ax] == idx)
					{
						html += "<option value=\"" + String(idx) + "\" selected>NONE</option>\n";
					}
					else
					{
						html += "<option value=\"" + String(idx) + "\">NONE</option>\n";
					}
				}
				else
				{
					if (config.digi_tlm_sensor[ax] == idx)
					{
						html += "<option value=\"" + String(idx) + "\" selected>SENSOR#" + String(idx) + "</option>\n";
					}
					else
					{
						html += "<option value=\"" + String(idx) + "\">SENSOR#" + String(idx) + "</option>\n";
					}
				}
			}
			html += "</select></td>\n";

			html += "<td style=\"text-align: left;\">Name: <input maxlength=\"10\" size=\"8\" name=\"param" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.digi_tlm_PARM[ax]) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Unit: <input maxlength=\"8\" size=\"5\" name=\"unit" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.digi_tlm_UNIT[ax]) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Precision: <input min=\"0\" max=\"5\" step=\"1\" type=\"number\" style=\"width: 2em\" name=\"precision" + String(ax) + "\" type=\"text\" value=\"" + String(config.digi_tlm_precision[ax]) + "\"  onchange=\"selPrecision(" + String(ax) + ")\"/></td></tr>\n";

			html += "<tr><td style=\"text-align: right;\">EQNS:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "a\" type=\"number\" value=\"" + String(config.digi_tlm_EQNS[ax][0], 5) + "\" />  b:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "b\" type=\"number\" value=\"" + String(config.digi_tlm_EQNS[ax][1], 5) + "\" /> c:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "c\" type=\"number\" value=\"" + String(config.digi_tlm_EQNS[ax][2], 5) + "\" /> (av<sup>2</sup>+bv+c) </td>\n";
			html += "<td style=\"text-align: left;\">Offset: <input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" type=\"number\" name=\"offset" + String(ax) + "\" type=\"text\" value=\"" + String(config.digi_tlm_offset[ax], 5) + "\" onchange=\"selOffset(" + String(ax) + ")\" /></td></tr>\n";
			html += "</table></td>";
			html += "</tr>\n";
		}
		html += "</table></td></tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitDIGI'  name=\"commitDIGI\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitDIGI\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
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
		// request->send(200, "text/html", html); // send to someones browser when asked
		//  AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)html.c_str(), html.length());
		//  response->addHeader("Digi", "content");
		//  request->send(response);
		// html.clear();
	}
}

void handle_wx(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	bool En = false;
	bool posGPS = false;
	bool pos2RF = false;
	bool pos2INET = false;
	bool timeStamp = false;
	String arg = "";

	if (request->hasArg("commitWX"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (int x = 0; x < WX_SENSOR_NUM; x++)
			config.wx_sensor_enable[x] = false;

		for (int i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}
			if (request->argName(i) == "Object")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					strlcpy(config.wx_object, name.c_str(), sizeof(config.wx_object));
				}
				else
				{
					config.wx_object[0] = 0;
				}
			}
			if (request->argName(i) == "myCall")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					name.toUpperCase();
					strlcpy(config.wx_mycall, name.c_str(), sizeof(config.wx_mycall));
				}
			}
			if (request->argName(i) == "mySSID")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.wx_ssid = request->arg(i).toInt();
					if (config.wx_ssid > 15)
						config.wx_ssid = 3;
				}
			}
			// if (request->argName(i) == "channel")
			// {
			// 	if (request->arg(i) != "")
			// 	{
			// 		if (isValidNumber(request->arg(i)))
			// 			config.wx_channel = request->arg(i).toInt();
			// 	}
			// }
			if (request->argName(i) == "PosInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.wx_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "PosLat")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.wx_lat = request->arg(i).toFloat();
				}
			}

			if (request->argName(i) == "PosLon")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.wx_lon = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "PosAlt")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.wx_alt = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "PosSel")
			{
				if (request->arg(i) != "")
				{
					if (request->arg(i).toInt() == 1)
						posGPS = true;
				}
			}

			if (request->argName(i) == "Path")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.wx_path = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "Comment")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wx_comment, request->arg(i).c_str(), sizeof(config.wx_comment));
				}
				else
				{
					memset(config.wx_comment, 0, sizeof(config.wx_comment));
				}
			}
			if (request->argName(i) == "Pos2RF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2RF = true;
				}
			}
			if (request->argName(i) == "Pos2INET")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2INET = true;
				}
			}
			if (request->argName(i) == "wxTimeStamp")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						timeStamp = true;
				}
			}
			for (int x = 0; x < WX_SENSOR_NUM; x++)
			{
				arg = "senEn" + String(x);
				if (request->argName(i) == arg)
				{

					if (request->arg(i) != "")
					{
						if (String(request->arg(i)) == "OK")
							config.wx_sensor_enable[x] = true;
					}
				}
				arg = "sensorCH" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.wx_sensor_ch[x] = request->arg(i).toInt();
				}
				arg = "avgSel" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						if (request->arg(i).toInt() == 1)
							config.wx_sensor_avg[x] = true;
						else if (request->arg(i).toInt() == 0)
							config.wx_sensor_avg[x] = false;
					}
				}
			}
		}
		config.wx_en = En;
		config.wx_gps = posGPS;
		config.wx_2rf = pos2RF;
		config.wx_2inet = pos2INET;
		config.wx_timestamp = timeStamp;

		initInterval = true;
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
		html += "document.getElementById(\"submitWX\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/wx',\n";
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

		/************************ WX Mode **************************/
		html += "<form id='formWX' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>[WX] Weather Station</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String EnFlag = "";
		if (config.wx_en)
			EnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + EnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station Callsign:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"7\" size=\"6\" id=\"myCall\" name=\"myCall\" type=\"text\" value=\"" + htmlEncode(config.wx_mycall) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station SSID:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"mySSID\" id=\"mySSID\">\n";
		for (uint8_t ssid = 0; ssid <= 15; ssid++)
		{
			if (config.wx_ssid == ssid)
			{
				html += "<option value=\"" + String(ssid) + "\" selected>" + String(ssid) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(ssid) + "\">" + String(ssid) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Object Name:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"9\" size=\"9\" name=\"Object\" type=\"text\" value=\"" + htmlEncode(config.wx_object) + "\" /><i> *If not used, leave it blank.In use 3-9 charactor</i></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"Path\" id=\"Path\">\n";
		for (uint8_t pthIdx = 0; pthIdx < PATH_LEN; pthIdx++)
		{
			if (config.wx_path == pthIdx)
			{
				html += "<option value=\"" + String(pthIdx) + "\" selected>" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(pthIdx) + "\">" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		// html += "<td style=\"text-align: left;\"><input maxlength=\"72\" size=\"72\" name=\"Path\" type=\"text\" value=\"" + String(config.wx_path) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Comment:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"50\" size=\"50\" name=\"Comment\" type=\"text\" value=\"" + htmlEncode(config.wx_comment) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>Time Stamp:</b></td>\n";
		String timeStampFlag = "";
		if (config.wx_timestamp)
			timeStampFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"wxTimeStamp\" value=\"OK\" " + timeStampFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";

		html += "<tr><td align=\"right\"><b>POSITION:</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";
		html += "<tr><td style=\"text-align: right;\">Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"3600\" step=\"1\" id=\"PosInv\" name=\"PosInv\" type=\"number\" value=\"" + String(config.wx_interval) + "\" />Sec.</td></tr>";
		String PosFixFlag = "";
		String PosGPSFlag = "";
		String Pos2RFFlag = "";
		String Pos2INETFlag = "";
		if (config.wx_gps)
			PosGPSFlag = "checked=\"checked\"";
		else
			PosFixFlag = "checked=\"checked\"";

		if (config.wx_2rf)
			Pos2RFFlag = "checked";
		if (config.wx_2inet)
			Pos2INETFlag = "checked";
		html += "<tr><td style=\"text-align: right;\">Location:</td><td style=\"text-align: left;\"><input type=\"radio\" name=\"PosSel\" value=\"0\" " + PosFixFlag + "/>Fix <input type=\"radio\" name=\"PosSel\" value=\"1\" " + PosGPSFlag + "/>GPS </td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">TX Channel:</td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"Pos2RF\" value=\"OK\" " + Pos2RFFlag + "/>RF <input type=\"checkbox\" name=\"Pos2INET\" value=\"OK\" " + Pos2INETFlag + "/>Internet </td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Latitude:</td><td style=\"text-align: left;\"><input min=\"-90\" max=\"90\" step=\"0.00001\" name=\"PosLat\" type=\"number\" value=\"" + String(config.wx_lat, 5) + "\" />degrees (positive for North, negative for South)</td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Longitude:</td><td style=\"text-align: left;\"><input min=\"-180\" max=\"180\" step=\"0.00001\" name=\"PosLon\" type=\"number\" value=\"" + String(config.wx_lon, 5) + "\" />degrees (positive for East, negative for West)</td></tr>\n";
		html += "<tr><td style=\"text-align: right;\">Altitude:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"10000\" step=\"0.1\" name=\"PosAlt\" type=\"number\" value=\"" + String(config.wx_alt, 2) + "\" /> meter. *The altitude in meters(m) above sea level</td></tr>\n";
		html += "</table></td>";
		html += "</tr>\n";

		// html += "<tr>\n";
		// html += "<td align=\"right\"><b>PORT:</b></td>\n";
		// html += "<td style=\"text-align: left;\">\n";
		// html += "<select name=\"channel\" id=\"channel\">\n";
		// for (int i = 0; i < 5; i++)
		// {
		// 	if (config.wx_channel == i)
		// 		html += "<option value=\"" + String(i) + "\" selected>" + String(WX_PORT[i]) + " </option>\n";
		// 	else
		// 		html += "<option value=\"" + String(i) + "\" >" + String(WX_PORT[i]) + " </option>\n";
		// }
		// html += "</select>\n";
		// html += "</td>\n";
		// html += "</tr>\n";
		/************************ Sensor Config Mode **************************/
		// html += "<form id='formSENSOR' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		// html += "<table>\n";
		// html += "<th colspan=\"2\"><span><b>Sensor Config</b></span></th>\n";

		html += "<tr><td align=\"right\"><b>SENSOR:<br />Selection</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";

		for (int ax = 0; ax < WX_SENSOR_NUM; ax++)
		{
			html += "<tr><td align=\"right\"><b>" + String(WX_SENSOR[ax]) + ":</b> \n";
			EnFlag = "";
			if (config.wx_sensor_enable[ax])
				EnFlag = "checked";
			html += "<label class=\"switch\"><input type=\"checkbox\" name=\"senEn" + String(ax) + "\" value=\"OK\" " + EnFlag + "><span class=\"slider round\"></span></label>";
			html += "</td>\n";

			// html += "<td style=\"text-align: lefe;\">Sensor:</td>\n";
			html += "<td style=\"text-align: left;\">Sensor Channel: ";
			html += "<select name=\"sensorCH" + String(ax) + "\" id=\"sensorCH" + String(ax) + "\">\n";
			for (uint8_t idx = 0; idx < 11; idx++)
			{
				if (idx == 0)
				{
					if (config.wx_sensor_ch[ax] == idx)
					{
						html += "<option value=\"" + String(idx) + "\" selected>NONE</option>\n";
					}
					else
					{
						html += "<option value=\"" + String(idx) + "\">NONE</option>\n";
					}
				}
				else
				{
					if (config.wx_sensor_ch[ax] == idx)
					{
						html += "<option value=\"" + String(idx) + "\" selected>SENSOR#" + String(idx) + "</option>\n";
					}
					else
					{
						html += "<option value=\"" + String(idx) + "\">SENSOR#" + String(idx) + "</option>\n";
					}
				}
			}
			html += "</select>\n";
			String avgFlag = "";
			String sampleFlag = "";
			if (config.wx_sensor_avg[ax])
				avgFlag = "checked=\"checked\"";
			else
				sampleFlag = "checked=\"checked\"";
			html += "<input type=\"radio\" name=\"avgSel" + String(ax) + "\" value=\"0\" " + sampleFlag + "/>Sample <input type=\"radio\" name=\"avgSel" + String(ax) + "\" value=\"1\" " + avgFlag + "/>Average";
			html += "</td></tr>";
		}
		html += "</table></td></tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitWX'  name=\"commitWX\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitWX\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
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
		// request->send(200, "text/html", html); // send to someones browser when asked
		//  if ((ESP.getFreeHeap() / 1000) > 110)
		//  {
		//  	request->send(200, "text/html", html); // send to someones browser when asked
		//  }
		//  else
		//  {
		//  	AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)html.c_str(), html.length());
		//  	response->addHeader("Weather", "content");
		//  	request->send(response);
		//  }
		// html.clear();
	}
}

void handle_tlm(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	bool En = false;
	bool pos2RF = false;
	bool pos2INET = false;
	String arg = "";

	if (request->hasArg("commitTLM"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (int i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "Enable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						En = true;
				}
			}
			if (request->argName(i) == "myCall")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					name.toUpperCase();
					strlcpy(config.tlm0_mycall, name.c_str(), sizeof(config.tlm0_mycall));
				}
			}
			if (request->argName(i) == "mySSID")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.tlm0_ssid = request->arg(i).toInt();
					if (config.tlm0_ssid > 15)
						config.tlm0_ssid = 3;
				}
			}
			if (request->argName(i) == "infoInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.tlm0_info_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "dataInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.tlm0_data_interval = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "Path")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.tlm0_path = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "Comment")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.tlm0_comment, request->arg(i).c_str(), sizeof(config.tlm0_comment));
				}
				else
				{
					memset(config.tlm0_comment, 0, sizeof(config.tlm0_comment));
				}
			}
			if (request->argName(i) == "Pos2RF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2RF = true;
				}
			}
			if (request->argName(i) == "Pos2INET")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2INET = true;
				}
			}
			for (int x = 0; x < 13; x++)
			{
				arg = "sensorCH" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.tml0_data_channel[x] = request->arg(i).toInt();
				}
				arg = "param" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.tlm0_PARM[x], request->arg(i).c_str(), sizeof(config.tlm0_PARM[x]));
					}
				}
				arg = "unit" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.tlm0_UNIT[x], request->arg(i).c_str(), sizeof(config.tlm0_UNIT[x]));
					}
				}
				if (x < 5)
				{
					for (int y = 0; y < 3; y++)
					{
						arg = "eqns" + String(x) + String((char)(y + 'a'));
						if (request->argName(i) == arg)
						{
							if (isValidNumber(request->arg(i)))
								config.tlm0_EQNS[x][y] = request->arg(i).toFloat();
						}
					}
				}
			}
			uint8_t b = 1;
			for (int x = 0; x < 8; x++)
			{
				arg = "bitact" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
					{
						if (request->arg(i).toInt() == 1)
						{
							config.tlm0_BITS_Active |= b;
						}
						else
						{
							config.tlm0_BITS_Active &= ~b;
						}
					}
				}
				b <<= 1;
			}
		}
		config.tlm0_en = En;
		config.tlm0_2rf = pos2RF;
		config.tlm0_2inet = pos2INET;

		initInterval = true;
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
		html += "document.getElementById(\"submitTLM\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/tlm',\n";
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

		/************************ TLM Mode **************************/
		html += "<form id='formTLM' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>System Telemetry</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String EnFlag = "";
		if (config.tlm0_en)
			EnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"Enable\" value=\"OK\" " + EnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station Callsign:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"7\" size=\"6\" id=\"myCall\" name=\"myCall\" type=\"text\" value=\"" + htmlEncode(config.tlm0_mycall) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Station SSID:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"mySSID\" id=\"mySSID\">\n";
		for (uint8_t ssid = 0; ssid <= 15; ssid++)
		{
			if (config.tlm0_ssid == ssid)
			{
				html += "<option value=\"" + String(ssid) + "\" selected>" + String(ssid) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(ssid) + "\">" + String(ssid) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		html += "</tr>\n";

		html += "<tr>\n";
		html += "<td align=\"right\"><b>PATH:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"Path\" id=\"Path\">\n";
		for (uint8_t pthIdx = 0; pthIdx < PATH_LEN; pthIdx++)
		{
			if (config.tlm0_path == pthIdx)
			{
				html += "<option value=\"" + String(pthIdx) + "\" selected>" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
			else
			{
				html += "<option value=\"" + String(pthIdx) + "\">" + String(PATH_NAME[pthIdx]) + "</option>\n";
			}
		}
		html += "</select></td>\n";
		// html += "<td style=\"text-align: left;\"><input maxlength=\"72\" size=\"72\" name=\"Path\" type=\"text\" value=\"" + String(config.tlm0_path) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Comment:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"50\" size=\"50\" name=\"Comment\" type=\"text\" value=\"" + htmlEncode(config.tlm0_comment) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<tr><td style=\"text-align: right;\">Info Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"3600\" step=\"1\" name=\"infoInv\" type=\"number\" value=\"" + String(config.tlm0_info_interval) + "\" />Sec.</td></tr>";
		html += "<tr><td style=\"text-align: right;\">Data Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"3600\" step=\"1\" name=\"dataInv\" type=\"number\" value=\"" + String(config.tlm0_data_interval) + "\" />Sec.</td></tr>";

		String Pos2RFFlag = "";
		String Pos2INETFlag = "";
		if (config.tlm0_2rf)
			Pos2RFFlag = "checked";
		if (config.tlm0_2inet)
			Pos2INETFlag = "checked";
		html += "<tr><td style=\"text-align: right;\">TX Channel:</td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"Pos2RF\" value=\"OK\" " + Pos2RFFlag + "/>RF <input type=\"checkbox\" name=\"Pos2INET\" value=\"OK\" " + Pos2INETFlag + "/>Internet </td></tr>\n";

		// html += "<tr>\n";
		// html += "<td align=\"right\"><b>Time Stamp:</b></td>\n";
		// String timeStampFlag = "";
		// if (config.wx_timestamp)
		// 	timeStampFlag = "checked";
		// html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"wxTimeStamp\" value=\"OK\" " + timeStampFlag + "><span class=\"slider round\"></span></label></td>\n";
		// html += "</tr>\n";
		for (int ax = 0; ax < 5; ax++)
		{
			html += "<tr><td align=\"right\"><b>Channel A" + String(ax + 1) + ":</b></td>\n";
			html += "<td align=\"center\">\n";
			html += "<table>";

			// html += "<tr><td style=\"text-align: right;\">Name:</td><td style=\"text-align: center;\"><i>Sensor Type</i></td><td style=\"text-align: center;\"><i>Parameter</i></td><td style=\"text-align: center;\"><i>Unit</i></td></tr>\n";

			html += "<tr><td style=\"text-align: right;\">Type/Name:</td>\n";
			html += "<td style=\"text-align: left;\">Sensor Type: ";
			html += "<select name=\"sensorCH" + String(ax) + "\" id=\"sensorCH" + String(ax) + "\">\n";
			for (uint8_t idx = 0; idx < SYSTEM_LEN; idx++)
			{
				if (config.tml0_data_channel[ax] == idx)
				{
					html += "<option value=\"" + String(idx) + "\" selected>" + String(SYSTEM_NAME[idx]) + "</option>\n";
				}
				else
				{
					html += "<option value=\"" + String(idx) + "\">" + String(SYSTEM_NAME[idx]) + "</option>\n";
				}
			}
			html += "</select></td>\n";

			html += "<td style=\"text-align: left;\">Parameter: <input maxlength=\"10\" size=\"8\" name=\"param" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.tlm0_PARM[ax]) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Unit: <input maxlength=\"8\" size=\"5\" name=\"unit" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.tlm0_UNIT[ax]) + "\" /></td></tr>\n";
			html += "<tr><td style=\"text-align: right;\">EQNS:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-9999\" max=\"9999\" step=\"0.0001\" name=\"eqns" + String(ax) + "a\" type=\"number\" value=\"" + String(config.tlm0_EQNS[ax][0], 3) + "\" />  b:<input min=\"-9999\" max=\"9999\" step=\"0.0001\" name=\"eqns" + String(ax) + "b\" type=\"number\" value=\"" + String(config.tlm0_EQNS[ax][1], 3) + "\" /> c:<input min=\"-9999\" max=\"9999\" step=\"0.0001\" name=\"eqns" + String(ax) + "c\" type=\"number\" value=\"" + String(config.tlm0_EQNS[ax][2], 3) + "\" /> (av<sup>2</sup>+bv+c)</td></tr>\n";
			html += "</table></td>";
			html += "</tr>\n";
		}

		uint8_t b = 1;
		for (int ax = 0; ax < 8; ax++)
		{
			html += "<tr><td align=\"right\"><b>Channel B" + String(ax + 1) + ":</b></td>\n";
			html += "<td align=\"center\">\n";
			html += "<table>";

			// html += "<tr><td style=\"text-align: right;\">Type/Name:</td>\n";
			html += "<td style=\"text-align: left;\">Type: ";
			html += "<select name=\"sensorCH" + String(ax + 5) + "\" id=\"sensorCH" + String(ax) + "\">\n";
			for (uint8_t idx = 0; idx < SYSTEM_BIT_LEN; idx++)
			{
				if (config.tml0_data_channel[ax + 5] == idx)
				{
					html += "<option value=\"" + String(idx) + "\" selected>" + String(SYSTEM_BITS_NAME[idx]) + "</option>\n";
				}
				else
				{
					html += "<option value=\"" + String(idx) + "\">" + String(SYSTEM_BITS_NAME[idx]) + "</option>\n";
				}
			}
			html += "</select></td>\n";

			html += "<td style=\"text-align: left;\">Parameter: <input maxlength=\"10\" size=\"8\" name=\"param" + String(ax + 5) + "\" type=\"text\" value=\"" + htmlEncode(config.tlm0_PARM[ax + 5]) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Unit: <input maxlength=\"8\" size=\"5\" name=\"unit" + String(ax + 5) + "\" type=\"text\" value=\"" + htmlEncode(config.tlm0_UNIT[ax + 5]) + "\" /></td>\n";
			String LowFlag = "", HighFlag = "";
			if (config.tlm0_BITS_Active & b)
				HighFlag = "checked=\"checked\"";
			else
				LowFlag = "checked=\"checked\"";
			html += "<td style=\"text-align: left;\"> Active:<input type=\"radio\" name=\"bitact" + String(ax) + "\" value=\"0\" " + LowFlag + "/>LOW <input type=\"radio\" name=\"bitact" + String(ax) + "\" value=\"1\" " + HighFlag + "/>HIGH </td>\n";
			html += "</tr>\n";
			// html += "<tr><td style=\"text-align: right;\">EQNS:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-999\" max=\"999\" step=\"0.1\" name=\"eqns" + String(ax + 1) + "a\" type=\"number\" value=\"" + String(config.tlm0_EQNS[ax][0], 3) + "\" />  b:<input min=\"-999\" max=\"999\" step=\"0.1\" name=\"eqns" + String(ax + 1) + "b\" type=\"number\" value=\"" + String(config.tlm0_EQNS[ax][1], 3) + "\" /> c:<input min=\"-999\" max=\"999\" step=\"0.1\" name=\"eqns" + String(ax + 1) + "c\" type=\"number\" value=\"" + String(config.tlm0_EQNS[ax][2], 3) + "\" /> (av<sup>2</sup>+bv+c)</td></tr>\n";
			html += "</table></td>";
			html += "</tr>\n";
			b <<= 1;
		}

		// html += "<tr><td align=\"right\"><b>Parameter Name:</b></td>\n";
		// html += "<td align=\"center\">\n";
		// html += "<table>";

		// // html += "<tr><td style=\"text-align: right;\">Latitude:</td><td style=\"text-align: left;\"><input min=\"-90\" max=\"90\" step=\"0.00001\" name=\"PosLat\" type=\"number\" value=\"" + String(config.wx_lat, 5) + "\" />degrees (positive for North, negative for South)</td></tr>\n";
		// // html += "<tr><td style=\"text-align: right;\">Longitude:</td><td style=\"text-align: left;\"><input min=\"-180\" max=\"180\" step=\"0.00001\" name=\"PosLon\" type=\"number\" value=\"" + String(config.wx_lon, 5) + "\" />degrees (positive for East, negative for West)</td></tr>\n";
		// // html += "<tr><td style=\"text-align: right;\">Altitude:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"10000\" step=\"0.1\" name=\"PosAlt\" type=\"number\" value=\"" + String(config.wx_alt, 2) + "\" /> meter. *Value 0 is not send height</td></tr>\n";
		// html += "</table></td>";
		// html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitTLM'  name=\"commitTLM\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitTLM\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
		request->send(200, "text/html", html); // send to someones browser when asked
	}
}

extern TaskHandle_t taskSensorHandle;

void handle_sensor(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);
	String arg = "";

	if (request->hasArg("commitSENSOR"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		//vTaskSuspend(taskSensorHandle);
		for (int x = 0; x < SENSOR_NUMBER; x++)
		{
			config.sensor[x].enable = false;
		}
		for (int i = 0; i < request->args(); i++)
		{
			//log_d("Arg %s: %s", request->argName(i).c_str(), request->arg(i).c_str());
			for (int x = 0; x < SENSOR_NUMBER; x++)
			{
				arg = "En" + String(x);
				if (request->argName(i) == arg)
				{

					if (request->arg(i) != "")
					{
						if (String(request->arg(i)) == "OK")
							config.sensor[x].enable = true;
					}
				}
				arg = "sensorCH" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.sensor[x].type = request->arg(i).toInt();
				}
				arg = "sensorP" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.sensor[x].port = request->arg(i).toInt();
				}
				arg = "address" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.sensor[x].address = request->arg(i).toInt();
				}
				arg = "sample" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.sensor[x].samplerate = request->arg(i).toInt();
				}
				arg = "avg" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.sensor[x].averagerate = request->arg(i).toInt();
				}
				arg = "param" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.sensor[x].parm, request->arg(i).c_str(), sizeof(config.sensor[x].parm));
					}
				}
				arg = "unit" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.sensor[x].unit, request->arg(i).c_str(), sizeof(config.sensor[x].unit));
					}
				}
				for (int y = 0; y < 3; y++)
				{
					arg = "eqns" + String(x) + String((char)(y + 'a'));
					if (request->argName(i) == arg)
					{
						if (isValidNumber(request->arg(i)))
							config.sensor[x].eqns[y] = request->arg(i).toFloat();
					}
				}
				//}
			}
		}

		log_d("Sensor Config Updated.");
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
		//vTaskResume(taskSensorHandle);
	}
	else
	{

		String html = "<script type=\"text/javascript\">\n";
		html += "$('form').submit(function (e) {\n";
		html += "e.preventDefault();\n";
		html += "var data = new FormData(e.currentTarget);\n";
		html += "document.getElementById(\"submitSENSOR\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/sensor',\n";
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
		html += "function setElm(name,val) {\n";
		html += "document.getElementById(name).value=val;\n";
		html += "};\n";
		html += "function selSensorType(idx) {\n";
		html += "var x=0;\n";
		html += "var parm=\"param\"+idx;\n";
		html += "var unit=\"unit\"+idx;\n";
		html += "x = document.getElementById(\"sensorCH\"+idx).value;\n";
		html += "if (x==1) {\n";
		html += "setElm(parm,\"Co2\");";
		html += "setElm(unit,\"ppm\");\n";
		html += "}else if (x==2) {\n";
		html += "setElm(parm,\"CH2O\");";
		html += "setElm(unit,\"μg/m³\");\n";
		html += "}else if (x==3) {\n";
		html += "setElm(parm,\"TVOC\");";
		html += "setElm(unit,\"μg/m³\");\n";
		html += "}else if (x==4) {\n";
		html += "setElm(parm,\"PM2.5\");";
		html += "setElm(unit,\"μg/m³\");\n";
		html += "}else if (x==5) {\n";
		html += "setElm(parm,\"PM10.0\");";
		html += "setElm(unit,\"μg/m³\");\n";
		html += "}else if (x==6) {\n";
		html += "setElm(parm,\"Temperature\");";
		html += "setElm(unit,\"°C\");\n";
		html += "}else if (x==7) {\n";
		html += "setElm(parm,\"Humidity\");";
		html += "setElm(unit,\"%RH\");\n";
		html += "}else if (x==8) {\n";
		html += "setElm(parm,\"Pressure\");";
		html += "setElm(unit,\"hPa\");\n";
		html += "}else if (x==9) {\n";
		html += "setElm(parm,\"WindSpeed\");";
		html += "setElm(unit,\"kPh\");\n";
		html += "}else if (x==10) {\n";
		html += "setElm(parm,\"WindCourse\");";
		html += "setElm(unit,\"°\");\n";
		html += "}else if (x==11) {\n";
		html += "setElm(parm,\"Rain\");";
		html += "setElm(unit,\"mm\");\n";
		html += "}else if (x==12) {\n";
		html += "setElm(parm,\"Luminosity\");";
		html += "setElm(unit,\"W/m³\");\n";
		html += "}else if (x==13) {\n";
		html += "setElm(parm,\"SoilTemp\");";
		html += "setElm(unit,\"°C\");\n";
		html += "}else if (x==14) {\n";
		html += "setElm(parm,\"SoilMoisture\");";
		html += "setElm(unit,\"%VWC\");\n";
		html += "}else if (x==15) {\n";
		html += "setElm(parm,\"WaterTemp\");";
		html += "setElm(unit,\"°C\");\n";
		html += "}else if (x==16) {\n";
		html += "setElm(parm,\"WaterTDS\");";
		html += "setElm(unit,\" \");\n";
		html += "}else if (x==17) {\n";
		html += "setElm(parm,\"WaterLevel\");";
		html += "setElm(unit,\"mm\");\n";
		html += "}else if (x==18) {\n";
		html += "setElm(parm,\"WaterFlow\");";
		html += "setElm(unit,\"L/min\");\n";
		html += "}else if (x==19) {\n";
		html += "setElm(parm,\"Voltage\");";
		html += "setElm(unit,\"V\");\n";
		html += "}else if (x==20) {\n";
		html += "setElm(parm,\"Current\");";
		html += "setElm(unit,\"A\");\n";
		html += "}else if (x==21) {\n";
		html += "setElm(parm,\"Power\");";
		html += "setElm(unit,\"W\");\n";
		html += "}else if (x==22) {\n";
		html += "setElm(parm,\"Energy\");";
		html += "setElm(unit,\"Wh\");\n";
		html += "}else if (x==23) {\n";
		html += "setElm(parm,\"Frequency\");";
		html += "setElm(unit,\"Hz\");\n";
		html += "}else if (x==24) {\n";
		html += "setElm(parm,\"PF\");";
		html += "setElm(unit,\" \");\n";
		html += "}else if (x==25) {\n";
		html += "setElm(parm,\"Satellite\");";
		html += "setElm(unit,\" \");\n";
		html += "}else if (x==26) {\n";
		html += "setElm(parm,\"HDOP\");";
		html += "setElm(unit,\" \");\n";
		html += "}else if (x==27) {\n";
		html += "setElm(parm,\"Battery\");";
		html += "setElm(unit,\"V\");\n";
		html += "}else if (x==28) {\n";
		html += "setElm(parm,\"BattLevel\");";
		html += "setElm(unit,\"%\");\n";
		html += "}\n}\n";

		html += "function selSensor(idx) {\n";
		html += "var x=0;\n";
		html += "x = document.getElementById(\"sensorP\"+idx).value;\n";
		html += "if (x>=10 && x<=13) {\n";
#ifdef TTGO_T_Beam_S3_SUPREME_V3
		html += "document.getElementById(\"address\"+idx).value=119;\n";
#else
		html += "document.getElementById(\"address\"+idx).value=118;\n";
#endif
		html += "}else if (x==16 || x==17) {\n";
		html += "document.getElementById(\"address\"+idx).value=90;\n";
		html += "}else if (x==23) {\n";
		html += "document.getElementById(\"address\"+idx).value=1;\n";
		html += "}else if (x==24 || x==25) {\n";
		html += "document.getElementById(\"address\"+idx).value=1000;\n";
		html += "}else{\n";
		html += "document.getElementById(\"address\"+idx).value=0;\n";
		html += "}\n}\n";
		html += "</script>\n";

		/************************ Sensor Monitor **************************/
		// html += "<form id='formSENSOR' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"5\"><span><b>Sensor Monitor</b></span></th>\n";
		int ax = 0;
		for (int r = 0; r < 3; r++)
		{
			html += "<tr>\n";
			for (int c = 0; c < 5; c++)
			{

				html += "<td align=\"center\">\n";
				if (config.sensor[ax].enable)
					html += "<fieldset id=\"SenGrp" + String(ax + 1) + "\">\n";
				else
					html += "<fieldset id=\"SenGrp" + String(ax + 1) + "\" disabled>\n";

				html += "<legend>SEN#" + String(ax + 1) + "-" + String(config.sensor[ax].parm) + "</legend>\n";
				html += "<input id=\"sVal" + String(ax) + "\" style=\"text-align:right;\" size=\"5\" type=\"text\" value=\"" + String(sen[ax].sample, 2) + "\" readonly/> " + String(config.sensor[ax].unit) + "\n";
				html += "</td>\n";
				ax++;
				if (ax >= SENSOR_NUMBER)
					break;
			}
			html += "</tr>\n";
			if (ax >= SENSOR_NUMBER)
				break;
		}
		html += "</table>< /br>\n";

		/************************ Sensor Config Mode **************************/
		html += "<form id='formSENSOR' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>Sensor Config</b></span></th>\n";
		String EnFlag = "";

		for (int ax = 0; ax < SENSOR_NUMBER; ax++)
		{
			html += "<tr><td align=\"right\"><b>SENSOR#" + String(ax + 1) + ":</b><br />\n";
			EnFlag = "";
			if (config.sensor[ax].enable)
				EnFlag = "checked";
			html += "<label class=\"switch\"><input type=\"checkbox\" name=\"En" + String(ax) + "\" value=\"OK\" " + EnFlag + "><span class=\"slider round\"></span></label>";
			html += "</td><td align=\"center\">\n";
			html += "<table>";

			html += "<tr><td style=\"text-align: right;\">Type:</td>\n";
			html += "<td style=\"text-align: left;\">";
			html += "<select name=\"sensorCH" + String(ax) + "\" id=\"sensorCH" + String(ax) + "\" onchange=\"selSensorType(" + String(ax) + ")\">\n";
			// for (uint8_t idx = 0; idx < SENSOR_NAME_NUM; idx++)
			// {
			// 	if (config.sensor[ax].type == idx)
			// 	{
			// 		html += "<option value=\"" + String(idx) + "\" selected>" + String(SENSOR_NAME[idx]) + "</option>\n";
			// 	}
			// 	else
			// 	{
			// 		html += "<option value=\"" + String(idx) + "\">" + String(SENSOR_NAME[idx]) + "</option>\n";
			// 	}
			// }
			html += "</select></td>\n";

			html += "<td style=\"text-align: left;\">Name: <input maxlength=\"15\" size=\"15\" name=\"param" + String(ax) + "\" id=\"param" + String(ax) + "\" type=\"text\" value=\"" + String(config.sensor[ax].parm) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Unit: <input maxlength=\"10\" size=\"5\" name=\"unit" + String(ax) + "\" id=\"unit" + String(ax) + "\" type=\"text\" value=\"" + String(config.sensor[ax].unit) + "\" /></td></tr>\n";
			// html += "<tr><td style=\"text-align: right;\">Port:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-999\" max=\"999\" step=\"0.1\" name=\"eqns" + String(ax) + "a\" type=\"number\" value=\"" + String(config.sensor[ax].eqns[0], 3) + "\" />  b:<input min=\"-999\" max=\"999\" step=\"0.1\" name=\"eqns" + String(ax) + "b\" type=\"number\" value=\"" + String(config.sensor[ax].eqns[1], 3) + "\" /> c:<input min=\"-999\" max=\"999\" step=\"0.1\" name=\"eqns" + String(ax) + "c\" type=\"number\" value=\"" + String(config.sensor[ax].eqns[2], 3) + "\" /> (av<sup>2</sup>+bv+c)</td></tr>\n";
			html += "<tr><td style=\"text-align: right;\">PORT:</td>\n";
			html += "<td style=\"text-align: left;\">";
			html += "<select name=\"sensorP" + String(ax) + "\" id=\"sensorP" + String(ax) + "\" onchange=\"selSensor(" + String(ax) + ")\">\n";
			// for (uint8_t idx = 0; idx < SENSOR_PORT_NUM; idx++)
			// {
			// 	if (config.sensor[ax].port == idx)
			// 	{
			// 		html += "<option value=\"" + String(idx) + "\" selected>" + String(SENSOR_PORT[idx]) + "</option>\n";
			// 	}
			// 	else
			// 	{
			// 		html += "<option value=\"" + String(idx) + "\">" + String(SENSOR_PORT[idx]) + "</option>\n";
			// 	}
			// }
			html += "</select></td>\n";
			html += "<td style=\"text-align: left;\">Addr/Reg/GPIO: <input style=\"text-align:right;\" min=\"0\" max=\"6500\" step=\"1\" name=\"address" + String(ax) + "\" id=\"address" + String(ax) + "\" type=\"number\" value=\"" + String(config.sensor[ax].address) + "\" /></td>\n";
			html += "<td style=\"text-align: left;\">Sample: <input style=\"text-align:right;\" min=\"0\" max=\"9999\" step=\"1\" name=\"sample" + String(ax) + "\" type=\"number\" value=\"" + String(config.sensor[ax].samplerate) + "\" />Sec.\n";
			html += "Average: <input style=\"text-align:right;\" min=\"0\" max=\"999\" step=\"1\" name=\"avg" + String(ax) + "\" type=\"number\" value=\"" + String(config.sensor[ax].averagerate) + "\" />Sec.</td></tr>\n";
			html += "<tr><td style=\"text-align: right;\">EQNS:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-999\" max=\"999\" step=\"0.00001\" name=\"eqns" + String(ax) + "a\" type=\"number\" value=\"" + String(config.sensor[ax].eqns[0], 5) + "\" />  b:<input min=\"-999\" max=\"999\" step=\"0.00001\" name=\"eqns" + String(ax) + "b\" type=\"number\" value=\"" + String(config.sensor[ax].eqns[1], 5) + "\" /> c:<input min=\"-999\" max=\"999\" step=\"0.00001\" name=\"eqns" + String(ax) + "c\" type=\"number\" value=\"" + String(config.sensor[ax].eqns[2], 5) + "\" /> (av<sup>2</sup>+bv+c)</td></tr>\n";
			html += "</table></td>";
			html += "</tr>\n";
		}

		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitSENSOR'  name=\"commitSENSOR\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitSENSOR\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";

		html += "<script type=\"text/javascript\">\n";
		html += "if (typeof typeArry === 'undefined'){let typeArry = [];};\n";
		html += "typeArry = new Array(";
		for (uint8_t idx = 0; idx < SENSOR_NAME_NUM; idx++)
		{
			html += "'" + String(SENSOR_NAME[idx]) + "'";
			if (idx < SENSOR_NAME_NUM - 1)
				html += ",";
		}
		html += ");\n";
		html += "if (typeof portArry === 'undefined'){let portArry = [];};\n";
		html += "portArry = new Array(";
		for (uint8_t idx = 0; idx < SENSOR_PORT_NUM; idx++)
		{
			html += "'" + String(SENSOR_PORT[idx]) + "'";
			if (idx < SENSOR_PORT_NUM - 1)
				html += ",";
		}
		html += ");\n";
		// html += "delete typeSel;delete listType;delete portSel;delete listPort;\n";
		html += "if (typeof typeSel === 'undefined'){var typeSel = [];};\n";
		html += "if (typeof listType === 'undefined'){var listType = [];};\n";
		html += "if (typeof portSel === 'undefined'){var portSel = [];};\n";
		html += "if (typeof listPort === 'undefined'){var listPort = [];};\n";
		for (int i = 0; i < 10; i++)
		{
			html += "listType[" + String(i) + "] = document.querySelector('#sensorCH" + String(i) + "');typeSel[" + String(i) + "]=" + String(config.sensor[i].type) + ";\n";
			html += "listPort[" + String(i) + "] = document.querySelector('#sensorP" + String(i) + "');portSel[" + String(i) + "]=" + String(config.sensor[i].port) + ";\n";
		}

		html += "for (let n = 0; n < 10; n++){\n";
		html += "for (let i = 0; i < typeArry.length; i++) {\n";
		html += "const optionType = new Option(typeArry[i], i);\n";
		html += "listType[n].add(optionType, undefined);\n";
		html += "};\n";
		html += "listType[n].options[typeSel[n]].selected = true;\n";
		html += "for (let p = 0; p < portArry.length; p++) {\n";
		html += "const optionPort = new Option(portArry[p], p);\n";
		html += "listPort[n].add(optionPort, undefined);\n";
		html += "};\n";
		html += "listPort[n].options[portSel[n]].selected = true;\n";
		html += "};\n";

		html += "</script>\n";
		log_d("FreeHeap=%i", ESP.getFreeHeap() / 1000);
		if ((ESP.getFreeHeap() / 1000) > 120)
		{
			request->send(200, "text/html", html); // send to someones browser when asked
		}
		else
		{
			// 	AsyncWebServerResponse *response = request->beginResponse_P(200, String(F("text/html")), (const uint8_t *)html.c_str(), html.length());
			// 	response->addHeader("Sensor", "/");
			// 	request->send(response);
			// 	delay(1000);
			// }
			// html.clear();
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
	}
}

void handle_tracker(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	bool trakerEn = false;
	bool smartEn = false;
	bool compEn = false;

	bool posGPS = false;
	bool bcnEN = false;
	bool pos2RF = false;
	bool pos2INET = false;
	bool optCST = false;
	bool optAlt = false;
	bool optBat = false;
	bool optSat = false;
	bool timeStamp = false;

	if (request->hasArg("commitTRACKER"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "trackerEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						trakerEn = true;
				}
			}
			if (request->argName(i) == "smartBcnEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						smartEn = true;
				}
			}
			if (request->argName(i) == "compressEnable")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						compEn = true;
				}
			}
			if (request->argName(i) == "trackerOptCST")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						optCST = true;
				}
			}
			if (request->argName(i) == "trackerOptAlt")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						optAlt = true;
				}
			}
			if (request->argName(i) == "trackerOptBat")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						optBat = true;
				}
			}
			if (request->argName(i) == "trackerOptSat")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						optSat = true;
				}
			}
			if (request->argName(i) == "myCall")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					name.toUpperCase();
					strlcpy(config.trk_mycall, name.c_str(), sizeof(config.trk_mycall));
				}
			}
			if (request->argName(i) == "trackerObject")
			{
				if (request->arg(i) != "")
				{
					String name = request->arg(i);
					name.trim();
					strlcpy(config.trk_item, name.c_str(), sizeof(config.trk_item));
				}
				else
				{
					memset(config.trk_item, 0, sizeof(config.trk_item));
				}
			}
			if (request->argName(i) == "mySSID")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_ssid = request->arg(i).toInt();
					if (config.trk_ssid > 15)
						config.trk_ssid = 13;
				}
			}
			if (request->argName(i) == "trackerPosInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "trkSTSInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_sts_interval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "trackerPosLat")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_lat = request->arg(i).toFloat();
				}
			}

			if (request->argName(i) == "trackerPosLon")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_lon = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "trackerPosAlt")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_alt = request->arg(i).toFloat();
				}
			}
			if (request->argName(i) == "trackerPosSel")
			{
				if (request->arg(i) != "")
				{
					if (request->arg(i).toInt() == 1)
						posGPS = true;
				}
			}
			if (request->argName(i) == "hspeed")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_hspeed = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "lspeed")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_lspeed = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "slowInterval")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_slowinterval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "maxInterval")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_maxinterval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "minInterval")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_mininterval = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "minAngle")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_minangle = request->arg(i).toInt();
				}
			}

			if (request->argName(i) == "trackerTable")
			{
				if (request->arg(i) != "")
				{
					config.trk_symbol[0] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "trackerSymbol")
			{
				if (request->arg(i) != "")
				{
					config.trk_symbol[1] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "moveTable")
			{
				if (request->arg(i) != "")
				{
					config.trk_symmove[0] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "moveSymbol")
			{
				if (request->arg(i) != "")
				{
					config.trk_symmove[1] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "stopTable")
			{
				if (request->arg(i) != "")
				{
					config.trk_symstop[0] = request->arg(i).charAt(0);
				}
			}
			if (request->argName(i) == "stopSymbol")
			{
				if (request->arg(i) != "")
				{
					config.trk_symstop[1] = request->arg(i).charAt(0);
				}
			}

			if (request->argName(i) == "trackerPath")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_path = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "trkMicEType")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_mice_type = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "trackerComment")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.trk_comment, request->arg(i).c_str(), sizeof(config.trk_comment));
				}
				else
				{
					memset(config.trk_comment, 0, sizeof(config.trk_comment));
				}
			}
			if (request->argName(i) == "trkStatus")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.trk_status, request->arg(i).c_str(), sizeof(config.trk_status));
				}
				else
				{
					memset(config.trk_status, 0, sizeof(config.trk_status));
				}
			}

			if (request->argName(i) == "trackerPos2RF")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2RF = true;
				}
			}
			if (request->argName(i) == "trackerPos2INET")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						pos2INET = true;
				}
			}
			if (request->argName(i) == "trackerTimeStamp")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
						timeStamp = true;
				}
			}
			if (request->argName(i) == "trkTlmInv")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.trk_tlm_interval = request->arg(i).toInt();
				}
			}
			String arg;
			for (int x = 0; x < 5; x++)
			{
				arg = "sensorCH" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.trk_tlm_sensor[x] = request->arg(i).toInt();
				}
				arg = "param" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.trk_tlm_PARM[x], request->arg(i).c_str(), sizeof(config.trk_tlm_PARM[x]));
					}
				}
				arg = "unit" + String(x);
				if (request->argName(i) == arg)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.trk_tlm_UNIT[x], request->arg(i).c_str(), sizeof(config.trk_tlm_UNIT[x]));
					}
				}
				arg = "precision" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.trk_tlm_precision[x] = request->arg(i).toInt();
				}
				arg = "offset" + String(x);
				if (request->argName(i) == arg)
				{
					if (isValidNumber(request->arg(i)))
						config.trk_tlm_offset[x] = request->arg(i).toFloat();
				}
				for (int y = 0; y < 3; y++)
				{
					arg = "eqns" + String(x) + String((char)(y + 'a'));
					if (request->argName(i) == arg)
					{
						if (isValidNumber(request->arg(i)))
							config.trk_tlm_EQNS[x][y] = request->arg(i).toFloat();
					}
				}
			}
		}
		config.trk_en = trakerEn;
		config.trk_smartbeacon = smartEn;
		config.trk_compress = compEn;

		config.trk_gps = posGPS;
		config.trk_loc2rf = pos2RF;
		config.trk_loc2inet = pos2INET;

		config.trk_log = optCST;
		config.trk_altitude = optAlt;
		config.trk_rssi = optBat;
		config.trk_sat = optSat;
		config.trk_timestamp = timeStamp;

		initInterval = true;
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

	String html = "<script type=\"text/javascript\">\n";
	html += "$('form').submit(function (e) {\n";
	html += "e.preventDefault();\n";
	html += "var data = new FormData(e.currentTarget);\n";
	html += "document.getElementById(\"submitTRACKER\").disabled=true;\n";
	html += "$.ajax({\n";
	html += "url: '/tracker',\n";
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
	html += "</script>\n<script type=\"text/javascript\">\n";
	html += "function openWindowSymbol(sel) {\n";
	html += "var i, l, options = [{\n";
	html += "value: 'first',\n";
	html += "text: 'First'\n";
	html += "}, {\n";
	html += "value: 'second',\n";
	html += "text: 'Second'\n";
	html += "}],\n";
	html += "newWindow = window.open(\"/symbol?sel=\"+sel.toString(), null, \"height=400,width=400,status=no,toolbar=no,menubar=no,location=no\");\n";
	html += "}\n";

	html += "function setValue(sel,symbol,table) {\n";
	html += "var txtsymbol=document.getElementById('trackerSymbol');\n";
	html += "var txttable=document.getElementById('trackerTable');\n";
	html += "var imgicon=document.getElementById('trackerImgSymbol');\n";
	html += "if(sel==1){\n";
	html += "txtsymbol=document.getElementById('moveSymbol');\n";
	html += "txttable=document.getElementById('moveTable');\n";
	html += "imgicon= document.getElementById('moveImgSymbol');\n";
	html += "}else if(sel==2){\n";
	html += "txtsymbol=document.getElementById('stopSymbol');\n";
	html += "txttable=document.getElementById('stopTable');\n";
	html += "imgicon= document.getElementById('stopImgSymbol');\n";
	html += "}\n";
	html += "txtsymbol.value = String.fromCharCode(symbol);\n";
	html += "if(table==1){\n txttable.value='/';\n";
	html += "}else if(table==2){\n txttable.value='\\\\';\n}\n";
	html += "imgicon.src = \"http://aprs.dprns.com/symbols/icons/\"+symbol.toString()+'-'+table.toString()+'.png';\n";
	html += "\n}\n";
	html += "function onSmartCheck() {\n";
	html += "if (document.querySelector('#smartBcnEnable').checked) {\n";
	// Checkbox has been checked
	html += "document.getElementById(\"smartbcnGrp\").disabled=false;\n";
	html += "} else {\n";
	// Checkbox has been unchecked
	html += "document.getElementById(\"smartbcnGrp\").disabled=true;\n";
	html += "}\n}\n";

	html += "function selPrecision(idx) {\n";
	html += "var x=0;\n";
	html += "x = document.getElementsByName(\"precision\"+idx)[0].value;\n";
	html += "document.getElementsByName(\"eqns\"+idx+\"b\")[0].value=1/Math.pow(10,x);\n";
	html += "}\n";
	html += "function selOffset(idx) {\n";
	html += "var x=0;\n";
	html += "x = document.getElementsByName(\"offset\"+idx)[0].value;\n";
	html += "document.getElementsByName(\"eqns\"+idx+\"c\")[0].value=x*(-1);\n";
	html += "}\n";
	html += "</script>\n";

	delay(1);
	/************************ tracker Mode **************************/
	html += "<form id='formtracker' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
	// html += "<h2>[TRACKER] Tracker Position Mode</h2>\n";
	html += "<table>\n";
	// html += "<tr>\n";
	// html += "<th width=\"200\"><span><b>Setting</b></span></th>\n";
	// html += "<th><span><b>Value</b></span></th>\n";
	// html += "</tr>\n";
	html += "<th colspan=\"2\"><span><b>[TRACKER] Tracker Position Mode</b></span></th>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Enable:</b></td>\n";
	String trackerEnFlag = "";
	if (config.trk_en)
		trackerEnFlag = "checked";
	html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"trackerEnable\" value=\"OK\" " + trackerEnFlag + "><span class=\"slider round\"></span></label></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Station Callsign:</b></td>\n";
	html += "<td style=\"text-align: left;\"><input maxlength=\"7\" size=\"6\" id=\"myCall\" name=\"myCall\" type=\"text\" value=\"" + htmlEncode(config.trk_mycall) + "\" /></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Station SSID:</b></td>\n";
	html += "<td style=\"text-align: left;\">\n";
	html += "<select name=\"mySSID\" id=\"mySSID\">\n";
	for (uint8_t ssid = 0; ssid <= 15; ssid++)
	{
		if (config.trk_ssid == ssid)
		{
			html += "<option value=\"" + String(ssid) + "\" selected>" + String(ssid) + "</option>\n";
		}
		else
		{
			html += "<option value=\"" + String(ssid) + "\">" + String(ssid) + "</option>\n";
		}
	}
	html += "</select></td>\n";
	html += "</tr>\n";

	html += "<tr>\n";
	html += "<td align=\"right\"><b>Item/Obj Name:</b></td>\n";
	html += "<td style=\"text-align: left;\"><input maxlength=\"9\" size=\"9\" id=\"trackerObject\" name=\"trackerObject\" type=\"text\" value=\"" + htmlEncode(config.trk_item) + "\" /><i> *If not used, leave it blank.In use 3-9 charactor</i></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>PATH:</b></td>\n";
	html += "<td style=\"text-align: left;\">\n";
	html += "<select name=\"trackerPath\" id=\"trackerPath\">\n";
	for (uint8_t pthIdx = 0; pthIdx < PATH_LEN; pthIdx++)
	{
		if (config.trk_path == pthIdx)
		{
			html += "<option value=\"" + String(pthIdx) + "\" selected>" + String(PATH_NAME[pthIdx]) + "</option>\n";
		}
		else
		{
			html += "<option value=\"" + String(pthIdx) + "\">" + String(PATH_NAME[pthIdx]) + "</option>\n";
		}
	}
	html += "</select></td>\n";
	// html += "<td style=\"text-align: left;\"><input maxlength=\"72\" size=\"72\" id=\"trackerPath\" name=\"trackerPath\" type=\"text\" value=\"" + String(config.trk_path) + "\" /></td>\n";
	html += "</tr>\n";

	html += "<tr>\n";
	html += "<td align=\"right\"><b>Text Comment:</b></td>\n";
	html += "<td style=\"text-align: left;\"><input maxlength=\"25\" size=\"30\" id=\"trackerComment\" name=\"trackerComment\" type=\"text\" value=\"" + htmlEncode(config.trk_comment) + "\" /></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
		html += "<td align=\"right\"><b>Text Status:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"50\" size=\"60\" id=\"trkStatus\" name=\"trkStatus\" type=\"text\" value=\"" + htmlEncode(config.trk_status) + "\" />  Interval:<input min=\"0\" max=\"3600\" step=\"1\" name=\"trkSTSInv\" type=\"number\" value=\"" + String(config.trk_sts_interval) + "\" />Sec.</td>\n";
		html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Smart Beacon:</b></td>\n";
	String smartBcnEnFlag = "";
	if (config.trk_smartbeacon)
		smartBcnEnFlag = "checked";
	html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" id=\"smartBcnEnable\" name=\"smartBcnEnable\" onclick=\"onSmartCheck()\" value=\"OK\" " + smartBcnEnFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"><i> *Switch use to smart beacon mode</i></label></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Compress:</b></td>\n";
	String compressEnFlag = "";
	if (config.trk_compress)
		compressEnFlag = "checked";
	html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"compressEnable\" value=\"OK\" " + compressEnFlag + "><span class=\"slider round\"></span></label><label style=\"vertical-align: bottom;font-size: 8pt;\"><i> *Switch compress packet</i></label></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Mic-E Type:</b></td>\n";
	html += "<td style=\"text-align: left;\">\n";
	html += "<select name=\"trkMicEType\" id=\"trkMicEType\">\n";
	for (uint8_t micEIdx = 0; micEIdx < 8; micEIdx++)
	{
		if (config.trk_mice_type == micEIdx)
		{
			html += "<option value=\"" + String(micEIdx) + "\" selected>" + String(MIC_E_MSG[micEIdx]) + "</option>\n";
		}
		else
		{
			html += "<option value=\"" + String(micEIdx) + "\">" + String(MIC_E_MSG[micEIdx]) + "</option>\n";
		}
	}
	html += "</select><label style=\"vertical-align: bottom;font-size: 8pt;\"><i>*Support if Compress is enabled and not use Item/Obj,Time Stamp</i></label></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\"><b>Time Stamp:</b></td>\n";
	String timeStampFlag = "";
	if (config.trk_timestamp)
		timeStampFlag = "checked";
	html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"trackerTimeStamp\" value=\"OK\" " + timeStampFlag + "><span class=\"slider round\"></span></label></td>\n";
	html += "</tr>\n";
	String trackerPos2RFFlag = "";
	String trackerPos2INETFlag = "";
	if (config.trk_loc2rf)
		trackerPos2RFFlag = "checked";
	if (config.trk_loc2inet)
		trackerPos2INETFlag = "checked";
	html += "<tr><td style=\"text-align: right;\"><b>TX Channel:</b></td><td style=\"text-align: left;\"><input type=\"checkbox\" name=\"trackerPos2RF\" value=\"OK\" " + trackerPos2RFFlag + "/>RF <input type=\"checkbox\" name=\"trackerPos2INET\" value=\"OK\" " + trackerPos2INETFlag + "/>Internet </td></tr>\n";
	String trackerOptBatFlag = "";
	String trackerOptSatFlag = "";
	String trackerOptAltFlag = "";
	String trackerOptCSTFlag = "";
	if (config.trk_rssi)
		trackerOptBatFlag = "checked";
	if (config.trk_sat)
		trackerOptSatFlag = "checked";
	if (config.trk_altitude)
		trackerOptAltFlag = "checked";
	if (config.trk_log)
		trackerOptCSTFlag = "checked";
	html += "<tr><td style=\"text-align: right;\"><b>Option:</b></td><td style=\"text-align: left;\">";
	html += "<input type=\"checkbox\" name=\"trackerOptCST\" value=\"OK\" " + trackerOptCSTFlag + "/>Telemetry ";
	html += "<input type=\"checkbox\" name=\"trackerOptAlt\" value=\"OK\" " + trackerOptAltFlag + "/>Altutude ";
	html += "<input type=\"checkbox\" name=\"trackerOptBat\" value=\"OK\" " + trackerOptBatFlag + "/>RSSI Request ";
	// html += "<input type=\"checkbox\" name=\"trackerOptSat\" value=\"OK\" " + trackerOptSatFlag + "/>Satellite";
	html += "</td></tr>\n";

	html += "<tr>";
	html += "<td align=\"right\"><b>POSITION:</b></td>\n";
	html += "<td align=\"center\">\n";
	html += "<table>";
	html += "<tr><td style=\"text-align: right;\">Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"3600\" step=\"1\" id=\"trackerPosInv\" name=\"trackerPosInv\" type=\"number\" value=\"" + String(config.trk_interval) + "\" />Sec.</label></td></tr>";
	String trackerPosFixFlag = "";
	String trackerPosGPSFlag = "";

	if (config.trk_gps)
		trackerPosGPSFlag = "checked=\"checked\"";
	else
		trackerPosFixFlag = "checked=\"checked\"";

	html += "<tr><td style=\"text-align: right;\">Location:</td><td style=\"text-align: left;\"><input type=\"radio\" name=\"trackerPosSel\" value=\"0\" " + trackerPosFixFlag + "/>Fix <input type=\"radio\" name=\"trackerPosSel\" value=\"1\" " + trackerPosGPSFlag + "/>GPS </td></tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\">Symbol Icon:</td>\n";
	String table = "1";
	if (config.trk_symbol[0] == 47)
		table = "1";
	if (config.trk_symbol[0] == 92)
		table = "2";
	html += "<td style=\"text-align: left;\">Table:<input maxlength=\"1\" size=\"1\" id=\"trackerTable\" name=\"trackerTable\" type=\"text\" value=\"" + String(config.trk_symbol[0]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> Symbol:<input maxlength=\"1\" size=\"1\" id=\"trackerSymbol\" name=\"trackerSymbol\" type=\"text\" value=\"" + String(config.trk_symbol[1]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> <img border=\"1\" style=\"vertical-align: middle;\" id=\"trackerImgSymbol\" onclick=\"openWindowSymbol(0);\" src=\"http://aprs.dprns.com/symbols/icons/" + String((int)config.trk_symbol[1]) + "-" + table + ".png\"> <i>*Click icon for select symbol</i></td>\n";
	html += "</tr>\n";
	html += "<tr><td style=\"text-align: right;\">Latitude:</td><td style=\"text-align: left;\"><input min=\"-90\" max=\"90\" step=\"0.00001\" id=\"trackerPosLat\" name=\"trackerPosLat\" type=\"number\" value=\"" + String(config.trk_lat, 5) + "\" />degrees (positive for North, negative for South)</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Longitude:</td><td style=\"text-align: left;\"><input min=\"-180\" max=\"180\" step=\"0.00001\" id=\"trackerPosLon\" name=\"trackerPosLon\" type=\"number\" value=\"" + String(config.trk_lon, 5) + "\" />degrees (positive for East, negative for West)</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Altitude:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"10000\" step=\"0.1\" id=\"trackerPosAlt\" name=\"trackerPosAlt\" type=\"number\" value=\"" + String(config.trk_alt, 2) + "\" /> meter. *Value 0 is not send height</td></tr>\n";
	html += "</table></td>";
	html += "</tr>\n";

	html += "<tr>\n";
	html += "<td align=\"right\"><b>Smart Beacon:</b></td>\n";
	html += "<td align=\"center\">\n";
	if (config.trk_smartbeacon)
		html += "<fieldset id=\"smartbcnGrp\">\n";
	else
		html += "<fieldset id=\"smartbcnGrp\" disabled>\n";
	html += "<legend>Smart beacon configuration</legend>\n<table>";
	html += "<tr>\n";
	html += "<td align=\"right\">Move Symbol:</td>\n";
	table = "1";
	if (config.trk_symmove[0] == 47)
		table = "1";
	if (config.trk_symmove[0] == 92)
		table = "2";
	html += "<td style=\"text-align: left;\">Table:<input maxlength=\"1\" size=\"1\" id=\"moveTable\" name=\"moveTable\" type=\"text\" value=\"" + String(config.trk_symmove[0]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> Symbol:<input maxlength=\"1\" size=\"1\" id=\"moveSymbol\" name=\"moveSymbol\" type=\"text\" value=\"" + String(config.trk_symmove[1]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> <img border=\"1\" style=\"vertical-align: middle;\" id=\"moveImgSymbol\" onclick=\"openWindowSymbol(1);\" src=\"http://aprs.dprns.com/symbols/icons/" + String((int)config.trk_symmove[1]) + "-" + table + ".png\"> <i>*Click icon for select MOVE symbol</i></td>\n";
	html += "</tr>\n";
	html += "<tr>\n";
	html += "<td align=\"right\">Stop Symbol:</td>\n";
	table = "1";
	if (config.trk_symstop[0] == 47)
		table = "1";
	if (config.trk_symstop[0] == 92)
		table = "2";
	html += "<td style=\"text-align: left;\">Table:<input maxlength=\"1\" size=\"1\" id=\"stopTable\" name=\"stopTable\" type=\"text\" value=\"" + String(config.trk_symstop[0]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> Symbol:<input maxlength=\"1\" size=\"1\" id=\"stopSymbol\" name=\"stopSymbol\" type=\"text\" value=\"" + String(config.trk_symstop[1]) + "\" style=\"background-color: rgb(97, 239, 170);\" /> <img border=\"1\" style=\"vertical-align: middle;\" id=\"stopImgSymbol\" onclick=\"openWindowSymbol(2);\" src=\"http://aprs.dprns.com/symbols/icons/" + String((int)config.trk_symstop[1]) + "-" + table + ".png\"> <i>*Click icon for select STOP symbol</i></td>\n";
	html += "</tr>\n";
	html += "<tr><td style=\"text-align: right;\">High Speed:</td><td style=\"text-align: left;\"><input size=\"3\" min=\"10\" max=\"1000\" step=\"1\" id=\"hspeed\" name=\"hspeed\" type=\"number\" value=\"" + String(config.trk_hspeed) + "\" /> km/h</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Low Speed:</td><td style=\"text-align: left;\"><input size=\"3\" min=\"1\" max=\"250\" step=\"1\" id=\"lspeed\" name=\"lspeed\" type=\"number\" value=\"" + String(config.trk_lspeed) + "\" /> km/h</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Slow Interval:</td><td style=\"text-align: left;\"><input size=\"3\" min=\"60\" max=\"3600\" step=\"1\" id=\"slowInterval\" name=\"slowInterval\" type=\"number\" value=\"" + String(config.trk_slowinterval) + "\" /> Sec.</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Max Interval:</td><td style=\"text-align: left;\"><input size=\"3\" min=\"10\" max=\"255\" step=\"1\" id=\"maxInterval\" name=\"maxInterval\" type=\"number\" value=\"" + String(config.trk_maxinterval) + "\" /> Sec.</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Min Interval:</td><td style=\"text-align: left;\"><input size=\"3\" min=\"1\" max=\"100\" step=\"1\" id=\"minInterval\" name=\"minInterval\" type=\"number\" value=\"" + String(config.trk_mininterval) + "\" /> Sec.</td></tr>\n";
	html += "<tr><td style=\"text-align: right;\">Min Angle:</td><td style=\"text-align: left;\"><input size=\"3\" min=\"1\" max=\"359\" step=\"1\" id=\"minAngle\" name=\"minAngle\" type=\"number\" value=\"" + String(config.trk_minangle) + "\" /> Degree.</td></tr>\n";

	html += "</table></fieldset></tr>";

	html += "<tr>\n";
	html += "<td align=\"right\"><b>Telemetry:</b><br />(v=0->8280)</td>\n";
	html += "<td align=\"center\"><table>\n";
	html += "<tr><td style=\"text-align: right;\">Interval:</td><td style=\"text-align: left;\"><input min=\"0\" max=\"1000\" step=\"1\" id=\"trkTlmInv\" name=\"trkTlmInv\" type=\"number\" value=\"" + String(config.trk_tlm_interval) + "\" /> *Number of packets interval,<i>Example: 0 not send,1 send every packet</i></label></td></tr>";
	for (int ax = 0; ax < 5; ax++)
	{
		html += "<tr><td align=\"right\"><b>CH A" + String(ax + 1) + ":</b></td>\n";
		html += "<td align=\"center\">\n";
		html += "<table>";

		html += "<tr><td style=\"text-align: right;\">Sensor:</td>\n";
		html += "<td style=\"text-align: left;\">CH: ";
		html += "<select name=\"sensorCH" + String(ax) + "\" id=\"sensorCH" + String(ax) + "\">\n";
		for (uint8_t idx = 0; idx < 11; idx++)
		{
			if (idx == 0)
			{
				if (config.trk_tlm_sensor[ax] == idx)
				{
					html += "<option value=\"" + String(idx) + "\" selected>NONE</option>\n";
				}
				else
				{
					html += "<option value=\"" + String(idx) + "\">NONE</option>\n";
				}
			}
			else
			{
				if (config.trk_tlm_sensor[ax] == idx)
				{
					html += "<option value=\"" + String(idx) + "\" selected>SENSOR#" + String(idx) + "</option>\n";
				}
				else
				{
					html += "<option value=\"" + String(idx) + "\">SENSOR#" + String(idx) + "</option>\n";
				}
			}
		}
		html += "</select></td>\n";

		html += "<td style=\"text-align: left;\">Name: <input maxlength=\"10\" size=\"8\" name=\"param" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.trk_tlm_PARM[ax]) + "\" /></td>\n";
		html += "<td style=\"text-align: left;\">Unit: <input maxlength=\"8\" size=\"5\" name=\"unit" + String(ax) + "\" type=\"text\" value=\"" + htmlEncode(config.trk_tlm_UNIT[ax]) + "\" /></td>\n";
		html += "<td style=\"text-align: left;\">Precision: <input min=\"0\" max=\"5\" step=\"1\" type=\"number\" style=\"width: 2em\" name=\"precision" + String(ax) + "\" type=\"text\" value=\"" + String(config.trk_tlm_precision[ax]) + "\" onchange=\"selPrecision(" + String(ax) + ")\" /></td></tr>\n";

		html += "<tr><td style=\"text-align: right;\">EQNS:</td><td colspan=\"3\" style=\"text-align: left;\">a:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "a\" type=\"number\" value=\"" + String(config.trk_tlm_EQNS[ax][0], 5) + "\" />  b:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "b\" type=\"number\" value=\"" + String(config.trk_tlm_EQNS[ax][1], 5) + "\" /> c:<input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" name=\"eqns" + String(ax) + "c\" type=\"number\" value=\"" + String(config.trk_tlm_EQNS[ax][2], 5) + "\" /> (av<sup>2</sup>+bv+c) </td>\n";
		html += "<td style=\"text-align: left;\">Offset: <input min=\"-9999\" max=\"9999\" step=\"0.00001\" style=\"width: 5em\" type=\"number\" name=\"offset" + String(ax) + "\" type=\"text\" value=\"" + String(config.trk_tlm_offset[ax], 5) + "\"  onchange=\"selOffset(" + String(ax) + ")\" /></td></tr>\n";
		html += "</table></td>";
		html += "</tr>\n";
	}
	html += "</table></td></tr>\n";
	html += "<tr><td colspan=\"2\" align=\"right\">\n";
	html += "<div><button class=\"button\" type='submit' id='submitTRACKER'  name=\"commitTRACKER\"> Apply Change </button></div>\n";
	html += "<input type=\"hidden\" name=\"commitTRACKER\"/>\n";
	html += csrfField();
	html += "</td></tr></table><br />\n";
	html += "</form><br />";
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

void handle_wireless(AsyncWebServerRequest *request)
{
	if (!request->authenticate(config.http_username, config.http_password))
	{
		return request->requestAuthentication();
	}
	StandByTick = millis() + (config.pwr_stanby_delay * 1000);

	if (request->hasArg("commitWiFiAP"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool wifiAP = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "wifiAP")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						wifiAP = true;
					}
				}
			}

			if (request->argName(i) == "wifi_ssidAP")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wifi_ap_ssid, request->arg(i).c_str(), sizeof(config.wifi_ap_ssid));
				}
			}
			if (request->argName(i) == "wifi_passAP")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.wifi_ap_pass, request->arg(i).c_str(), sizeof(config.wifi_ap_pass));
				}
			}
		}
		if (wifiAP)
		{
			config.wifi_mode |= WIFI_AP_FIX;
		}
		else
		{
			config.wifi_mode &= ~WIFI_AP_FIX;
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
	else if (request->hasArg("commitWiFiClient"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool wifiSTA = false;
		String nameSSID, namePASS;
		for (int n = 0; n < 5; n++)
			config.wifi_sta[n].enable = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "wificlient")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						wifiSTA = true;
					}
				}
			}

			for (int n = 0; n < 5; n++)
			{
				nameSSID = "wifiStation" + String(n);
				if (request->argName(i) == nameSSID)
				{
					if (request->arg(i) != "")
					{
						if (String(request->arg(i)) == "OK")
						{
							config.wifi_sta[n].enable = true;
						}
					}
				}
				nameSSID = "wifi_ssid" + String(n);
				if (request->argName(i) == nameSSID)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.wifi_sta[n].wifi_ssid, request->arg(i).c_str(), sizeof(config.wifi_sta[n].wifi_ssid));
					}
				}
				namePASS = "wifi_pass" + String(n);
				if (request->argName(i) == namePASS)
				{
					if (request->arg(i) != "")
					{
						strlcpy(config.wifi_sta[n].wifi_pass, request->arg(i).c_str(), sizeof(config.wifi_sta[n].wifi_pass));
					}
				}
			}

			if (request->argName(i) == "wifi_pwr")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i))){
						config.wifi_power = (int8_t)request->arg(i).toInt();
						WiFi.setTxPower((wifi_power_t)config.wifi_power);
					}
				}
			}
		}
		if (wifiSTA)
		{
			config.wifi_mode |= WIFI_STA_FIX;
		}
		else
		{
			config.wifi_mode &= ~WIFI_STA_FIX;
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
	else if (request->hasArg("commitBluetooth"))
	{
		if (!csrfValid(request)) { request->send(403, "text/plain", "CSRF validation failed"); return; }
		bool btMaster = false;
		for (uint8_t i = 0; i < request->args(); i++)
		{
			if (request->argName(i) == "btMaster")
			{
				if (request->arg(i) != "")
				{
					if (String(request->arg(i)) == "OK")
					{
						btMaster = true;
					}
				}
			}

			if (request->argName(i) == "bt_name")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.bt_name, request->arg(i).c_str(), sizeof(config.bt_name));
				}
			}
			if (request->argName(i) == "bt_uuid")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.bt_uuid, request->arg(i).c_str(), sizeof(config.bt_uuid));
				}
			}
			if (request->argName(i) == "bt_uuid_rx")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.bt_uuid_rx, request->arg(i).c_str(), sizeof(config.bt_uuid_rx));
				}
			}
			if (request->argName(i) == "bt_uuid_tx")
			{
				if (request->arg(i) != "")
				{
					strlcpy(config.bt_uuid_tx, request->arg(i).c_str(), sizeof(config.bt_uuid_tx));
				}
			}
			if (request->argName(i) == "bt_mode")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.bt_mode = request->arg(i).toInt();
				}
			}
			if (request->argName(i) == "bt_pin")
			{
				if (request->arg(i) != "")
				{
					if (isValidNumber(request->arg(i)))
						config.bt_pin = request->arg(i).toInt();
				}
			}
		}
		config.bt_master = btMaster;
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
		html += "if(e.currentTarget.id===\"formBluetooth\") document.getElementById(\"submitBluetooth\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formWiFiAP\") document.getElementById(\"submitWiFiAP\").disabled=true;\n";
		html += "if(e.currentTarget.id===\"formWiFiClient\") document.getElementById(\"submitWiFiClient\").disabled=true;\n";
		html += "$.ajax({\n";
		html += "url: '/wireless',\n";
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
		/************************ WiFi AP **************************/
		html += "<form id='formWiFiAP' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		// html += "<h2>WiFi Access Point</h2>\n";
		html += "<table>\n";
		// html += "<tr>\n";
		// html += "<th width=\"200\"><span><b>Setting</b></span></th>\n";
		// html += "<th><span><b>Value</b></span></th>\n";
		// html += "</tr>\n";
		html += "<th colspan=\"2\"><span><b>WiFi Access Point</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\" width=\"120\"><b>Enable:</b></td>\n";
		String wifiAPEnFlag = "";
		if (config.wifi_mode & WIFI_AP_FIX)
			wifiAPEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"wifiAP\" value=\"OK\" " + wifiAPEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>WiFi AP SSID:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"32\" maxlength=\"32\" class=\"form-control\" id=\"wifi_ssidAP\" name=\"wifi_ssidAP\" type=\"text\" value=\"" + htmlEncode(config.wifi_ap_ssid) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>WiFi AP PASSWORD:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input size=\"63\" maxlength=\"63\" class=\"form-control\" id=\"wifi_passAP\" name=\"wifi_passAP\" type=\"password\" value=\"" + htmlEncode(config.wifi_ap_pass) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitWiFiAP'  name=\"commitWiFiAP\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitWiFiAP\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
		/************************ WiFi Client **************************/
		html += "<br />\n";
		html += "<form id='formWiFiClient' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		html += "<table>\n";
		html += "<th colspan=\"2\"><span><b>WiFi Multi Station</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>WiFi STA Enable:</b></td>\n";
		String wifiClientEnFlag = "";
		if (config.wifi_mode & WIFI_STA_FIX)
			wifiClientEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"wificlient\" value=\"OK\" " + wifiClientEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>WiFi RF Power:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"wifi_pwr\" id=\"wifi_pwr\">\n";
		for (int i = 0; i < 12; i++)
		{
			if (config.wifi_power == (int8_t)wifiPwr[i][0])
				html += "<option value=\"" + String((int8_t)wifiPwr[i][0]) + "\" selected>" + String(wifiPwr[i][1], 1) + " dBm</option>\n";
			else
				html += "<option value=\"" + String((int8_t)wifiPwr[i][0]) + "\" >" + String(wifiPwr[i][1], 1) + " dBm</option>\n";
		}
		html += "</select>\n";
		html += "</td>\n";
		html += "</tr>\n";
		for (int n = 0; n < 5; n++)
		{
			html += "<tr>\n";
			html += "<td align=\"right\"><b>Station #" + String(n + 1) + ":</b></td>\n";
			html += "<td align=\"center\">\n";
			html += "<fieldset id=\"filterDispGrp" + String(n + 1) + "\">\n";
			html += "<legend>WiFi Station #" + String(n + 1) + "</legend>\n<table style=\"text-align:unset;border-width:0px;background:unset\">";
			html += "<tr style=\"background:unset;\">";
			// html += "<tr>\n";
			html += "<td align=\"right\" width=\"120\"><b>Enable:</b></td>\n";
			String wifiClientEnFlag = "";
			if (config.wifi_sta[n].enable)
				wifiClientEnFlag = "checked";
			html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"wifiStation" + String(n) + "\" value=\"OK\" " + wifiClientEnFlag + "><span class=\"slider round\"></span></label></td>\n";
			html += "</tr>\n";
			html += "<tr>\n";
			html += "<td align=\"right\"><b>WiFi SSID:</b></td>\n";
			html += "<td style=\"text-align: left;\"><input size=\"32\" maxlength=\"32\" name=\"wifi_ssid" + String(n) + "\" type=\"text\" value=\"" + htmlEncode(config.wifi_sta[n].wifi_ssid) + "\" /></td>\n";
			html += "</tr>\n";
			html += "<tr>\n";
			html += "<td align=\"right\"><b>WiFi PASSWORD:</b></td>\n";
			html += "<td style=\"text-align: left;\"><input size=\"63\" maxlength=\"63\" name=\"wifi_pass" + String(n) + "\" type=\"password\" value=\"" + htmlEncode(config.wifi_sta[n].wifi_pass) + "\" /></td>\n";
			html += "</tr>\n";
			html += "</tr></table></fieldset>\n";
			html += "</td></tr>\n";
		}
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitWiFiClient'  name=\"commitWiFiClient\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitWiFiClient\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form><br />";
		/************************ Bluetooth **************************/
#ifdef BLUETOOTH
		html += "<br />\n";
		html += "<form id='formBluetooth' method=\"POST\" action='#' enctype='multipart/form-data'>\n";
		// html += "<h2>Bluetooth Master (BLE)</h2>\n";
		html += "<table>\n";
		// html += "<tr>\n";
		// html += "<th width=\"200\"><span><b>Setting</b></span></th>\n";
		// html += "<th><span><b>Value</b></span></th>\n";
		// html += "</tr>\n";
		html += "<th colspan=\"2\"><span><b>Bluetooth Master (BLE)</b></span></th>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>Enable:</b></td>\n";
		String btEnFlag = "";
		if (config.bt_master)
			btEnFlag = "checked";
		html += "<td style=\"text-align: left;\"><label class=\"switch\"><input type=\"checkbox\" name=\"btMaster\" value=\"OK\" " + btEnFlag + "><span class=\"slider round\"></span></label></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>NAME:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"20\" id=\"bt_name\" name=\"bt_name\" type=\"text\" value=\"" + htmlEncode(config.bt_name) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>PIN:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input min=\"0\" max=\"999999\" id=\"bt_pin\" name=\"bt_pin\" type=\"number\" value=\"" + String(config.bt_pin,DEC) + "\" /></td> <i>*Value 0 is no auth.</i>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>UUID:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"37\" size=\"38\" id=\"bt_uuid\" name=\"bt_uuid\" type=\"text\" value=\"" + htmlEncode(config.bt_uuid) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>UUID RX:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"37\" size=\"38\" id=\"bt_uuid_rx\" name=\"bt_uuid_rx\" type=\"text\" value=\"" + htmlEncode(config.bt_uuid_rx) + "\" /></td>\n";
		html += "</tr>\n";
		html += "<tr>\n";
		html += "<td align=\"right\"><b>UUID TX:</b></td>\n";
		html += "<td style=\"text-align: left;\"><input maxlength=\"37\" size=\"38\" id=\"bt_uuid_tx\" name=\"bt_uuid_tx\" type=\"text\" value=\"" + htmlEncode(config.bt_uuid_tx) + "\" /></td>\n";
		html += "</tr>\n";

		html += "<td align=\"right\"><b>MODE:</b></td>\n";
		html += "<td style=\"text-align: left;\">\n";
		html += "<select name=\"bt_mode\" id=\"bt_mode\">\n";
		String btModeOff = "";
		String btModeTNC2 = "";
		String btModeKISS = "";
		if (config.bt_mode == 1)
		{
			btModeTNC2 = "selected";
		}
		else if (config.bt_mode == 2)
		{
			btModeKISS = "selected";
		}
		else
		{
			btModeOff = "selected";
		}
		html += "<option value=\"0\" " + btModeOff + ">NONE</option>\n";
		html += "<option value=\"1\" " + btModeTNC2 + ">TNC2</option>\n";
		html += "<option value=\"2\" " + btModeKISS + ">KISS</option>\n";
		html += "</select>\n";

		html += "<label style=\"font-size: 8pt;text-align: right;\">*See the following for generating UUIDs: <a href=\"https://www.uuidgenerator.net\" target=\"_blank\">https://www.uuidgenerator.net</a></label></td>\n";
		html += "</tr>\n";
		html += "<tr><td colspan=\"2\" align=\"right\">\n";
		html += "<div><button class=\"button\" type='submit' id='submitBluetooth'  name=\"commitBluetooth\"> Apply Change </button></div>\n";
		html += "<input type=\"hidden\" name=\"commitBluetooth\"/>\n";
		html += csrfField();
		html += "</td></tr></table><br />\n";
		html += "</form>";
#endif
		request->send(200, "text/html", html); // send to someones browser when asked
	}
}
