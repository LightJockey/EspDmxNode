#ifndef ESPDMXNODE_WEB_H
#define ESPDMXNODE_WEB_H

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Include gzipped web app
#include "web_app/dist/index.html.gz.h"

const char CONTENTTYPE_HTML[] = "text/html";
const char CONTENTTYPE_TEXT[] = "text/plain";
const char CONTENTTYPE_JSON[] = "application/json";

const int HTTPSTATUS_OK = 200;
const int HTTPSTATUS_NOTFOUND = 404;
const int HTTPSTATUS_ERROR = 500;

AsyncWebServer webServer(HTTP_PORT);

// https://github.com/me-no-dev/ESPAsyncWebServer/pull/391
class OneParamRewrite : public AsyncWebRewrite
{
protected:
    String _urlPrefix;
    int _paramIndex;
    String _paramsBackup;

public:
    OneParamRewrite(const char *from, const char *to) : AsyncWebRewrite(from, to)
    {
        _paramIndex = _from.indexOf('{');

        if (_paramIndex >= 0 && _from.endsWith("}"))
        {
            _urlPrefix = _from.substring(0, _paramIndex);
            int index = _params.indexOf('{');
            if (index >= 0)
                _params = _params.substring(0, index);
        }
        else
            _urlPrefix = _from;

        _paramsBackup = _params;
    }

    bool match(AsyncWebServerRequest *request) override
    {
        if (request->url().startsWith(_urlPrefix))
        {
            if (_paramIndex >= 0)
                _params = _paramsBackup + request->url().substring(_paramIndex);
            else
                _params = _paramsBackup;
            return true;
        }
        else
            return false;
    }
};

bool web_check_ota_auth(AsyncWebServerRequest *request)
{
    AsyncWebHeader *authHeader = request->getHeader(PSTR("Auth"));
    if (authHeader == NULL ||
        strcmp(authHeader->value().c_str(), default_ota_password) != 0)
    {
        return false;
    }
    return true;
}

void web_init()
{
    #pragma region NODE ENDPOINTS
    auto indexHandler = ([](AsyncWebServerRequest *request) {
        // Client still has a valid cached version
        if (request->header("If-Modified-Since").equals(WEBAPP_LASTMODIFIED))
            request->send(304);
        // Send the gzipped webapp from PROGMEM
        else
        {
            AsyncWebServerResponse *response = request->beginResponse_P(HTTPSTATUS_OK, CONTENTTYPE_HTML, WEBAPP_COMPRESSED, WEBAPP_COMPRESSED_LEN);
            response->addHeader("Last-Modified", WEBAPP_LASTMODIFIED);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        }
    });
    webServer.on(PSTR("/"), HTTP_GET, indexHandler);
    webServer.on(PSTR("/status"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTPSTATUS_OK, CONTENTTYPE_JSON, status_print());
    });
    webServer.on(PSTR("/logs"), HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream(CONTENTTYPE_TEXT);
        uint8_t logIdx = logEntries < LOG_BUFFER_ENTRIES ? 0 : curLogIdx;
        while (true)
        {
            response->println(logBuffer[logIdx]);
            logIdx = (logIdx + 1) % LOG_BUFFER_ENTRIES;
            if (logIdx == curLogIdx)
            break;
        }
        request->send(response);
    });
    webServer.on(PSTR("/settings"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTPSTATUS_OK, CONTENTTYPE_JSON, config_print());
    });
    webServer.on(PSTR("/settings/save"), HTTP_POST, [](AsyncWebServerRequest *request) {
            // Do nothing, see: https://github.com/me-no-dev/ESPAsyncWebServer/issues/195#issuecomment-379458163
        }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (index == 0)
            config_string_buf[0] = 0;

            strncpy(config_string_buf + index, (char *)data, len);

            if (index + len == total)
            {
                if (config_parse(config_string_buf, total))
                {
                    config_save();

                    request->send(HTTPSTATUS_OK);

                    if (wifi_ap_started)
                        WiFi.softAPdisconnect();

                    restart();
                }
                else
                    request->send(HTTPSTATUS_ERROR);
            }
        }
    );
    webServer.on(PSTR("/settings/reset"), HTTP_GET, [](AsyncWebServerRequest *request) {
        config_reset();
        request->send(HTTPSTATUS_OK);
        restart();
    });
    webServer.on(PSTR("/networks"), HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(2048);
        JsonArray arr = doc.to<JsonArray>();

        int16_t status = WiFi.scanComplete();
        if (status == WIFI_SCAN_FAILED)
            WiFi.scanNetworks(true);
        else if (status >= 0)
        {
            for (int i = 0; i < status; i++)
            {
                JsonObject netObj = arr.createNestedObject();
                netObj[F("ssid")] = WiFi.SSID(i);
                netObj[F("rssi")] = WiFi.RSSI(i);
            }
            
            WiFi.scanDelete();

            AsyncResponseStream *response = request->beginResponseStream(CONTENTTYPE_JSON);
            serializeJson(arr, *response);
            request->send(response);
        }
    });
    webServer.on(PSTR("/testoutput"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTPSTATUS_OK);
        out_isTestingOutput = true;
    });
    webServer.on(PSTR("/restart"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTPSTATUS_OK);
        restart();
    });
    webServer.on(PSTR("/update"), HTTP_POST, [](AsyncWebServerRequest *request) {
            if (!web_check_ota_auth(request))
            {
                request->send(401, CONTENTTYPE_TEXT, PSTR("Access Denied"));
                return;
            }
            if (Update.hasError())
            {
                char buf[128];
                snprintf_P(buf, sizeof(buf), PSTR("Update failed: %s"), Update.getError());
                request->send(HTTPSTATUS_ERROR, CONTENTTYPE_TEXT, buf);
            }
            else
                request->send(HTTPSTATUS_OK, CONTENTTYPE_TEXT, PSTR("Update successful! Restarting ..."));
        }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!web_check_ota_auth(request))
                return;

            if (!index)
            {
                watchdog_disable();
                Update.runAsync(true);

                logf_P(LOG_INFO, PSTR("Starting update from file \"%s\" ..."), filename.c_str());
                
                if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
                    log(LOG_INFO, (char *)Update.getError());
            }
            if (!Update.hasError())
            {
                if (Update.write(data, len) != len)
                    log(LOG_INFO, (char *)Update.getError());
            }
            if (final)
            {
                if (Update.end(true))
                    logf_P(LOG_INFO, PSTR("Successfully written %u bytes"), index + len);
                else
                {
                    log(LOG_INFO, (char *)Update.getError());
                    watchdog_enable();
                }
            }
    });
#pragma endregion

    #pragma region HTTP API ENDPOINTS
    //////////////////////////////
    // /state
    //////////////////////////////
    webServer.on(API_PATH_STATE, HTTP_GET, [](AsyncWebServerRequest *request) {
        if (out_curDimmerScale > 0)
            request->send(HTTPSTATUS_OK, CONTENTTYPE_TEXT, PSTR("ON"));
        else
            request->send(HTTPSTATUS_OK, CONTENTTYPE_TEXT, PSTR("OFF"));
    });
    webServer.on(API_PATH_STATE, HTTP_POST | HTTP_PUT, [](AsyncWebServerRequest *request) { }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (api_set_state((char *)data, total))
            request->send(HTTPSTATUS_OK);
        else
            request->send(HTTPSTATUS_ERROR);
    });

    //////////////////////////////
    // /dimmer
    //////////////////////////////
    webServer.on(API_PATH_DIMMER, HTTP_GET, [](AsyncWebServerRequest *request) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%u", (uint8_t)(out_curDimmerScale * UINT8_MAX));
        request->send(HTTPSTATUS_OK, CONTENTTYPE_TEXT, buf);
    });
    webServer.on(API_PATH_DIMMER, HTTP_POST | HTTP_PUT, [](AsyncWebServerRequest *request) { }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (api_set_dimmer((char *)data, total))
            request->send(HTTPSTATUS_OK);
        else
            request->send(HTTPSTATUS_ERROR);
    });
    
    //////////////////////////////
    // /scene
    //////////////////////////////
    webServer.on(API_PATH_SCENE, HTTP_GET, [](AsyncWebServerRequest *request) {
        char buf[2];
        snprintf(buf, sizeof(buf), "%u", out_curSceneIdx);
        request->send(HTTPSTATUS_OK, CONTENTTYPE_TEXT, buf);
    });
    webServer.on(API_PATH_SCENE, HTTP_POST | HTTP_PUT, [](AsyncWebServerRequest *request) { }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (api_set_scene((char *)data, total))
            request->send(HTTPSTATUS_OK);
        else
            request->send(HTTPSTATUS_ERROR);
    });

    //////////////////////////////
    // /dmx
    //////////////////////////////
    webServer.addRewrite(new OneParamRewrite("/dmx/{ch}", "/dmx?ch={ch}"));
    webServer.on(API_PATH_DMX, HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!isOutputEnabled())
        {
            request->send(HTTPSTATUS_ERROR);
            return;
        }
        // print out individual channel value
        if (request->hasParam("ch"))
        {
            AsyncWebParameter *chParam = request->getParam("ch");
            int channel = str2int(chParam->value().c_str(), chParam->value().length()) - 1;
            if (channel < 0 || channel > out_poolSize - 1)
                request->send(HTTPSTATUS_NOTFOUND);
            else
                request->send(HTTPSTATUS_OK, CONTENTTYPE_TEXT, out_print_channel(channel));
        }
        // print out all channel values (as raw bytes)
        else
            request->send_P(HTTPSTATUS_OK, CONTENTTYPE_TEXT, out_poolMerged, out_poolSize);
    });
    webServer.on(API_PATH_DMX, HTTP_POST | HTTP_PUT, [](AsyncWebServerRequest *request) { }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!isOutputEnabled())
        {
            request->send(HTTPSTATUS_ERROR);
            return;
        }
        // set individual channel value
        if (request->hasParam("ch") && total <= 3)
        {
            AsyncWebParameter *chParam = request->getParam("ch");
            int channel = str2int(chParam->value().c_str(), chParam->value().length()) - 1;
            if (channel < 0 || channel > out_poolSize - 1)
                request->send(HTTPSTATUS_NOTFOUND);
            else
            {
                int value = str2int((char *)data, total);
                if (value < 0 || value > UINT8_MAX)
                {
                    request->send(HTTPSTATUS_ERROR);
                    return;
                }
                out_set_channel(channel, value);
                request->send(HTTPSTATUS_OK);
            }
        }
        else
            request->send(HTTPSTATUS_ERROR);
    });
    #pragma endregion

    webServer.onNotFound(wifi_ap_started ? indexHandler : [](AsyncWebServerRequest *request) { request->send(HTTPSTATUS_NOTFOUND); });

    // Allow CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    webServer.begin();

    logf_P(LOG_INFO, PSTR("Started async webserver on TCP %u"), HTTP_PORT);
}

#endif
