#ifndef ESPDMXNODE_WEB_H
#define ESPDMXNODE_WEB_H

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Include gzipped web app
#include "web_app/dist/index.html.gz.h"

const char HTMLCONTENT_NOTFOUND[] PROGMEM = R"=====(
<h1>Not Found</h1>
)=====";

const char CONTENTTYPE_HTML[] = "text/html";
const char CONTENTTYPE_TEXT[] = "text/plain";
const char CONTENTTYPE_JSON[] = "application/json";
const int HTMLSTATUS_OK = 200;
const int HTMLSTATUS_ERROR = 500;

AsyncWebServer webServer(HTTP_PORT);

void web_init()
{
    webServer.on(PSTR("/"), HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->header("If-Modified-Since").equals(WEBAPP_LASTMODIFIED))
            request->send(304);
        else {
            // Send the gzipped webapp from PROGMEM
            AsyncWebServerResponse *response = request->beginResponse_P(HTMLSTATUS_OK, CONTENTTYPE_HTML, WEBAPP_COMPRESSED, WEBAPP_COMPRESSED_LEN);
            response->addHeader("Last-Modified", WEBAPP_LASTMODIFIED);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        }
    });
    webServer.on(PSTR("/status"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTMLSTATUS_OK, CONTENTTYPE_JSON, status_print());
    });
    webServer.on(PSTR("/logs"), HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream(CONTENTTYPE_TEXT);
        uint8_t logIdx = logEntries < LOG_BUFFER_ENTRIES ? 0 : curLogIdx;
        while (true) {
            response->println(logBuffer[logIdx]);
            logIdx = (logIdx + 1) % LOG_BUFFER_ENTRIES;
            if (logIdx == curLogIdx)
            break;
        }
        request->send(response);
    });
    webServer.on(PSTR("/settings"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTMLSTATUS_OK, CONTENTTYPE_JSON, config_print());
    });
    webServer.on(PSTR("/settings/save"), HTTP_POST, [](AsyncWebServerRequest *request) {
            // Do nothing, see: https://github.com/me-no-dev/ESPAsyncWebServer/issues/195#issuecomment-379458163
        }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (index == 0)
            config_string_buf[0] = 0;

            strncpy(config_string_buf + index, (char *)data, len);

            if (index + len == total) {
            if (config_parse(config_string_buf, total)) {
                config_save();

                request->send(HTMLSTATUS_OK);

                if (wifi_ap_started)
                    WiFi.softAPdisconnect();

                restart();
            }
            else
                request->send(HTMLSTATUS_ERROR);
            }
        }
    );
    webServer.on(PSTR("/settings/reset"), HTTP_GET, [](AsyncWebServerRequest *request) {
        config_reset();
        request->send(HTMLSTATUS_OK);
        restart();
    });
    webServer.on(PSTR("/channels"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(HTMLSTATUS_OK, CONTENTTYPE_TEXT, out_poolMerged, out_poolSize);
    });
    webServer.on(PSTR("/testoutput"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTMLSTATUS_OK);
        out_isTestingOutput = true;
    });
    webServer.on(PSTR("/restart"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTMLSTATUS_OK);
        restart();
    });
    webServer.onNotFound([](AsyncWebServerRequest *request) { request->send_P(404, CONTENTTYPE_HTML, HTMLCONTENT_NOTFOUND); });
    webServer.begin();
    logf_P(LOG_INFO, PSTR("Started async webserver on TCP %u"), HTTP_PORT);
}

#endif
