#include "ESPDMXNode.h"

/*
=============
=== SETUP ===
=============
*/
void setup()
{
    ESP.wdtDisable();

#ifdef DEBUG
    Serial.begin(115200);
#endif

    pinMode(RESET_PIN, INPUT);
    pinMode(DEBUG_LED_PIN, OUTPUT);

    logf_P(LOG_INFO, PSTR("Starting EspDmxNode v%s (build date: %s, %s) (mac: %s)"), FIRMWARE_VERSION, __DATE__, __TIME__, WiFi.macAddress().c_str());

    config_init();
    config_load();

    wifi_setup();
    wifi_init();
    // STATION mode
    if (wifi_mode > 0)
    {
        if (config.mqtt_enabled)
        {
            mqttClient.onMessage([](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
                logf_P(LOG_MQTT, PSTR("Pub msg received. topic: %s, qos: %u, dup: %u, retain: %u, len: %u, index: %u, total: %u, payload: \"%*.*s\""),
                        topic,
                        properties.qos,
                        properties.dup,
                        properties.retain,
                        len,
                        index,
                        total,
                        len,
                        len,
                        payload);

                // mqtt_topic_state
                if (strcmp(topic, mqtt_topic_state) == 0)
                    api_set_state(payload, len);
                // mqtt_topic_dimmer
                else if (strcmp(topic, mqtt_topic_dimmer) == 0)
                    api_set_dimmer(payload, len);
                // mqtt_topic_scene
                else if (strcmp(topic, mqtt_topic_scene) == 0)
                    api_set_scene(payload, len);
            });
        }

        // NOTE: using PROGMEM strings inside OTA callbacks crashes the ESP
        ArduinoOTA.onStart([]() {
            artnet_stop();
            out_stop();

            ESP.wdtDisable();
            yield();
            log(LOG_OTA, (char *)"Starting fw update");
        });
        ArduinoOTA.onEnd([]() {
            log(LOG_OTA, (char *)"Done!");
            restart();
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            logf(LOG_OTA, "Progress: %u", progress / (total / 100));
        });
        ArduinoOTA.onError([](ota_error_t err) {
            char errReason[8];
            if (err == OTA_AUTH_ERROR) strlcpy(errReason, "auth", sizeof(errReason));
            else if (err == OTA_BEGIN_ERROR) strlcpy(errReason, "begin", sizeof(errReason));
            else if (err == OTA_CONNECT_ERROR) strlcpy(errReason, "connect", sizeof(errReason));
            else if (err == OTA_RECEIVE_ERROR) strlcpy(errReason, "receive", sizeof(errReason));
            else if (err == OTA_END_ERROR) strlcpy(errReason, "end", sizeof(errReason));
            logf(LOG_OTA, "Error: %s fail", errReason);
        });
        ArduinoOTA.setHostname(hostname);
        ArduinoOTA.setPassword(default_ota_password);
        ArduinoOTA.begin();
    }

    web_init();

    out_setup();

    // Reenable the watchdog with a 4s timeout
    ESP.wdtEnable(WDTO_4S);

    logf_P(LOG_INFO, PSTR("EspDmxNode \"%s\" started"), hostname);
}

/*
==================
=== MAIN  LOOP ===
==================
*/
void loop()
{
    //ESP.wdtFeed();

    wifi_handle();

    // Handle ArtNet
    if (wifi_connected && config.artnet_enabled)
        artRDM.handler();

    // Handle DMX
    if (out_numPorts > 0)
    {
        if (out_isTestingOutput)
            out_test();
        else
        {
            out_merge(true);
            out_send();
        }
    }

    // Handle physical reset button
    resetPinState = digitalRead(RESET_PIN);
    if (resetPinState == LOW && tLastFrame > 0)
    {
        t_resetAccumul += millis() - tLastFrame;
        if (t_resetAccumul > (uint16_t)RESET_TIME)
        {
            config_reset();
            restart();
        }
    }
    else
        t_resetAccumul = 0;

    // Blink the onboard LED
    uint16_t tDbgLed_delay = wifi_connected ? DEBUG_LED_BLINK_CONNECTED_INTERVAL : DEBUG_LED_BLINK_DISCONNECTED_INTERVAL;
    if (millis() - tDbgLed_lastFlip > tDbgLed_delay)
    {
        DbgLed_State = DbgLed_State == LOW ? HIGH : LOW;
        digitalWrite(DEBUG_LED_PIN, DbgLed_State);
        tDbgLed_lastFlip = millis();
    }

    // Increment the uptime counter
    tUptimeAccumul += millis() - tLastFrame;
    while (tUptimeAccumul >= 1000)
    {
        tUptimeAccumul -= 1000;
        tUptime++;
    }
    tLastFrame = millis();
}

/*
======================
=== ARTNET METHODS ===
======================
*/
void artnet_init()
{
    if (!config.artnet_enabled)
        return;

    logf_P(LOG_ARTNET, PSTR("Starting ArtNet node (ip: %s, netmask: %s, name: %s)"), WiFi.localIP().toString().c_str(), WiFi.subnetMask().toString().c_str(), config.node_name);
    artRDM.init(WiFi.localIP(), WiFi.subnetMask(), (bool)config.dhcp, config.node_name, config.node_name, ARTNET_OEM, ESTA_MAN, mac_address);

    //artRDM.setFirmwareVersion(FIRMWARE_VERSION);

    uint8_t net, subnet, uni;
    if (config.port_a.enabled)
    {
        artnet_decode_uni(config.port_a.universe, &net, &subnet, &uni);
        artnet_portA[0] = artRDM.addGroup(net, subnet);
        artnet_portA[1] = artRDM.addPort(artnet_portA[0], 0, uni);
    }
    if (config.port_b.enabled)
    {
        artnet_decode_uni(config.port_b.universe, &net, &subnet, &uni);
        artnet_portB[0] = artRDM.addGroup(net, subnet);
        artnet_portB[1] = artRDM.addPort(artnet_portB[0], 1, uni);
    }

    artRDM.setArtDMXCallback(artnet_handle_dmx);

    if (isRdmEnabled())
    {
        artRDM.setArtRDMCallback(artnet_handle_rdm);
        artRDM.setTODRequestCallback(artnet_handle_tod_request);
        artRDM.setTODFlushCallback(artnet_handle_tod_flush);
    }
    //artRDM.setArtSyncCallback(syncHandle);
    //artRDM.setArtIPCallback(ipHandle);
    //artRDM.setArtAddressCallback(addressHandle);

    artRDM.begin();
}

void artnet_stop()
{
    if (!config.artnet_enabled)
        return;

    log_P(LOG_ARTNET, PSTR("Stopping ArtNet ..."));
    artRDM.end();
}

void artnet_handle_dmx(uint8_t group, uint8_t port, uint16_t numChans, bool syncEnabled)
{
    if (!isOutputEnabled())
        return;

    if (artnet_portA[0] == group && artnet_portA[1] == port)
        out_set_uni(0, artRDM.getDMX(group, port), numChans);
    else if (artnet_portB[0] == group && artnet_portB[1] == port)
        out_set_uni(1, artRDM.getDMX(group, port), numChans);

    uint16_t artnet_tDelta = (millis() - artnet_tLastReceived);
    artnet_framesPerSecond = artnet_tDelta > 0 ? 1000 / artnet_tDelta : 0;
    artnet_tLastReceived = millis();
}
void artnet_handle_rdm(uint8_t group, uint8_t port, rdm_data *data)
{
    if (!isRdmEnabled())
        return;

    if (artnet_portA[0] == group && artnet_portA[1] == port)
    {
        if (config.port_a.rdm_enabled)
            dmxA.rdmSendCommand(data);
    }
    else if (artnet_portB[0] == group && artnet_portB[1] == port)
    {
        if (config.port_b.rdm_enabled)
            dmxB.rdmSendCommand(data);
    }
}
void artnet_handle_tod_request(uint8_t group, uint8_t port)
{
    if (!isRdmEnabled())
        return;

    if (artnet_portA[0] == group && artnet_portA[1] == port)
    {
        if (config.port_a.rdm_enabled)
            dmx_handle_tod_portA();
    }

    else if (artnet_portB[0] == group && artnet_portB[1] == port)
    {
        if (config.port_b.rdm_enabled)
            dmx_handle_tod_portB();
    }
}
void artnet_handle_tod_flush(uint8_t group, uint8_t port)
{
    if (!isRdmEnabled())
        return;

    if (artnet_portA[0] == group && artnet_portA[1] == port)
    {
        if (config.port_a.rdm_enabled)
            dmxA.rdmDiscovery();
    }
    else if (artnet_portB[0] == group && artnet_portB[1] == port)
    {
        if (config.port_b.rdm_enabled)
            dmxB.rdmDiscovery();
    }
}

/*
======================
===   DMX METHODS  ===
======================
*/
void out_setup()
{
    out_numPorts = 0;
    if (config.port_a.enabled)
        out_numPorts = 1;
    if (config.port_b.enabled)
        out_numPorts = 2;

    if (!isOutputEnabled())
        return;

    out_poolSize = sizeof(uint8_t) * DMX_UNI_LEN * out_numPorts;
    out_poolPrev = (uint8_t *)os_zalloc(out_poolSize);
    out_poolCur = (uint8_t *)os_zalloc(out_poolSize);
    out_poolScene = (uint8_t *)os_zalloc(out_poolSize);
    out_poolMerged = (uint8_t *)os_zalloc(out_poolSize);
    out_poolFadeStartValues = (uint8_t *)os_zalloc(out_poolSize);
    out_poolPropIsInScene = (bool *)os_zalloc(out_poolSize);

    for (uint8_t portIdx = 0; portIdx < out_numPorts; portIdx++)
    {
#if DEBUG   // Serial monitor and port A share the same GPIO1 (TX) pin
        if (portIdx == 0)
            continue;
#endif

        bool portEnabled = portIdx == 0 ? config.port_a.enabled : config.port_b.enabled;
        if (!portEnabled)
            continue;

        uint8_t portType = portIdx == 0 ? config.port_a.type : config.port_b.type;
        bool rdmEnabled = portIdx == 0 ? config.port_a.rdm_enabled : config.port_b.rdm_enabled;
        uint16_t numChannels = portIdx == 0 ? config.port_a.num_channels : config.port_b.num_channels;
        uint16_t universe = portIdx == 0 ? config.port_a.universe : config.port_b.universe;
        uint16_t offset = portIdx == 0 ? config.port_a.offset : config.port_b.offset;

        char portLetter = 'A' + portIdx;

        if (portType == PORT_DMX)
        {
            espDMX *dmxPort = portIdx == 0 ? &dmxA : &dmxB;

            dmxPort->chanUpdate(numChannels);

            if (rdmEnabled)
            {
                dmxPort->rdmEnable(ESTA_MAN, ESTA_DEV);
                dmxPort->rdmSetCallBack(dmx_handle_rdm_portA);
                dmxPort->todSetCallBack(dmx_handle_tod_portA);
            }

            uint8_t *dmxBufferStart = out_poolMerged + (portIdx == 1 ? DMX_UNI_LEN : 0);
            if (rdmEnabled)
                dmxPort->begin(DMXA_DIR_PIN, dmxBufferStart);
            else
                dmxPort->begin(dmxBufferStart);

            logf_P(LOG_OUTPUT, PSTR("Port %c started as DMX (channels: %u, rdm: %u, universe: %u)"), portLetter, numChannels, rdmEnabled, universe);
        }
        else if (portType == PORT_PIXELS)
        {
            if (portIdx == 0)
            {
                pixelsA = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart0800KbpsMethod>(numChannels);
                pixelsA->Begin();
            }
            else
            {
                pixelsB = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod>(numChannels);
                pixelsB->Begin();
            }

            logf_P(LOG_OUTPUT, PSTR("Port %c started as PIXELS (pixels: %u, universe: %u, offset: %u)"), portLetter, numChannels, universe, offset);
        }
    }

    // Send a first frame out so that we clear the gibberish sent out by the bootloader?
    out_clear();

    out_a_brightness_scale = (float_t)config.port_a.brightness / (float_t)255;
    out_b_brightness_scale = (float_t)config.port_b.brightness / (float_t)255;
    out_curDimmerScale = 1.0F;
}

void out_set_uni(uint8_t port, uint8_t *data, uint16_t numChans)
{
    if (!isOutputEnabled())
        return;

    uint16_t poolChOffset = DMX_UNI_LEN * port;
    memcpy(out_poolPrev + poolChOffset, out_poolCur + poolChOffset, _min(numChans, DMX_UNI_LEN));
    memcpy(out_poolCur + poolChOffset, data, _min(numChans, DMX_UNI_LEN) /*_min(dmx_poolSize - poolChOffset, DMX_UNI_LEN)*/);

    out_tLastFrameReceived = millis();

    //dmx_merge(false);
}

void out_merge(bool isLoop)
{
    if (!isOutputEnabled())
        return;

    uint32_t dmx_tCurFadeStep = millis() - out_tLastFadeStart;
    bool dmx_isFading = config.mqtt_dmx_fadetime > 0 && dmx_tCurFadeStep < config.mqtt_dmx_fadetime;

    uint8_t startValue;
    uint8_t targetValue;
    for (uint16_t i = 0; i < out_poolSize; i++)
    {
        if (out_poolPropIsInScene[i])
        {
            startValue = out_poolCur[i];
            targetValue = out_poolScene[i];
        }
        else
        {
            startValue = out_poolFadeStartValues[i];

            // Handle frame interpolation.
            // If the delta between prev and cur frame is lower than threshold, lerp prev towards cur at the configured time rate; otherwise snap to cur value.
            // "smoothTime" should be set at the known interval between ArtNet frames.
            bool isPortA = i < DMX_UNI_LEN ? true : false;
            uint8_t smoothTime = isPortA ? config.port_a.smoothtime : config.port_b.smoothtime;

            if (smoothTime > 0)
            {
                int8_t frameDelta = out_poolPrev[i] - out_poolCur[i];
                uint8_t frameDeltaAbs = abs(frameDelta);
                uint8_t deltaThreshold = isPortA ? config.port_a.smoothingthreshold : config.port_b.smoothingthreshold;

                if (frameDeltaAbs < deltaThreshold)
                {
                    uint32_t dmx_tSmoothingStep = _min(millis() - out_tLastFrameReceived, smoothTime);
                    targetValue = lerp(out_poolPrev[i], out_poolCur[i], dmx_tSmoothingStep, smoothTime);
                }
                else
                    targetValue = out_poolCur[i];
            }
            else
                targetValue = out_poolCur[i];
        }
        
        // Handle global brightness
        float_t brightnessScale = i < DMX_UNI_LEN ? out_a_brightness_scale : out_b_brightness_scale;
        targetValue = scale(targetValue, brightnessScale);
        
        // Handle MQTT dimming
        targetValue = scale(targetValue, out_curDimmerScale);

        if (dmx_isFading)
            out_poolMerged[i] = lerp(startValue, targetValue, dmx_tCurFadeStep, config.mqtt_dmx_fadetime);
        else
            out_poolMerged[i] = targetValue;
    }
}

void out_send()
{
    if (config.port_a.enabled)
    {
#ifndef DEBUG
        if (config.port_a.type == PORT_DMX)
        {
            dmxA.chanUpdate(config.port_a.num_channels);
            dmxA.handler();
        }
        else if (config.port_a.type == PORT_PIXELS)
        {
            uint8_t *pixelsBuf = pixelsA->Pixels();
            uint8_t *pixelsAddr = out_poolMerged + config.port_a.offset;
            memcpy(pixelsBuf, pixelsAddr, 3 * config.port_a.num_channels);
            pixelsA->Dirty();
            pixelsA->Show();
        }
#endif
    }
    if (config.port_b.enabled)
    {
        if (config.port_b.type == PORT_DMX)
        {
            dmxB.chanUpdate(config.port_b.num_channels);
            dmxB.handler();
        }
        else if (config.port_b.type == PORT_PIXELS)
        {
            uint8_t *pixelsBuf = pixelsB->Pixels();
            uint8_t *pixelsAddr = config.port_a.universe != config.port_b.universe ? out_poolMerged + DMX_UNI_LEN + config.port_b.offset : out_poolMerged + config.port_b.offset;
            memcpy(pixelsBuf, pixelsAddr, 3 * config.port_b.num_channels);
            pixelsB->Dirty();
            pixelsB->Show();
        }
    }
}

void out_clear()
{
    if (config.port_a.enabled)
    {
#ifndef DEBUG
        if (config.port_a.type == PORT_DMX)
        {
            dmxA.clearChans();
            dmxA.chanUpdate(config.port_a.num_channels);
            dmxA.handler();
        }
        else if (config.port_a.type == PORT_PIXELS)
        {
            uint8_t *pixelsBuf = pixelsA->Pixels();
            memset(pixelsBuf, 0, 3 * config.port_a.num_channels);
            pixelsA->Dirty();
            pixelsA->Show();
        }
#endif
    }
    if (config.port_b.enabled)
    {
        if (config.port_a.type == PORT_DMX)
        {
            dmxB.clearChans();
            dmxB.chanUpdate(config.port_b.num_channels);
            dmxB.handler();
        }
        else if (config.port_b.type == PORT_PIXELS)
        {
            uint8_t *pixelsBuf = pixelsB->Pixels();
            memset(pixelsBuf, 0, 3 * config.port_b.num_channels);
            pixelsB->Dirty();
            pixelsB->Show();
        }
    }
}

void out_stop()
{
    if (out_numPorts == 0)
        return;
    
    log_P(LOG_OUTPUT, PSTR("Stopping output ..."));

    if (config.port_a.enabled)
    {
#ifndef DEBUG
        if (config.port_a.type == PORT_DMX)
            dmxA.end();
        else if (config.port_a.type == PORT_PIXELS)
        {
            delete pixelsA;
            pixelsA = NULL;
        }
#endif
    }
    if (config.port_b.enabled)
    {
        if (config.port_b.type == PORT_DMX)
            dmxB.end();
        else if (config.port_b.type == PORT_PIXELS)
        {
            delete pixelsB;
            pixelsB = NULL;
        }
    }
}

void out_test()
{
    if (out_numPorts == 0)
    {
        out_isTestingOutput = false;
        return;
    }

    out_clear();

    uint8_t port = 0;
    while (port < out_numPorts)
    {
        if (!isValidDmxPort(port))
            continue;
        uint8_t port_type = port == 0 ? config.port_a.type : config.port_b.type;
        uint16_t num_channels = port == 0 ? config.port_a.num_channels : config.port_b.num_channels;
        for (uint16_t chIdx = 0; chIdx < num_channels; chIdx++)
        {
            if (port_type == PORT_DMX)
            {
                out_set_channel(DMX_UNI_LEN * port + chIdx, 255);
#ifndef DEBUG
                dmxA.chanUpdate(DMX_UNI_LEN);
#endif
                dmxB.chanUpdate(DMX_UNI_LEN);
                if (port == 0)
                {
#ifndef DEBUG
                    dmxA.handler();
#endif
                }
                else
                    dmxB.handler();
            }
            else if (port_type == PORT_PIXELS)
            {
                if (port == 0)
                {
#ifndef DEBUG
                    pixelsA->SetPixelColor(chIdx, RgbColor(255, 255, 255));
                    pixelsA->Show();
#endif
                }
                else
                {
                    pixelsB->SetPixelColor(chIdx, RgbColor(255, 255, 255));
                    pixelsB->Show();
                }
            }

            delay(100);
            ESP.wdtFeed();
        }
        port++;
    }

    out_clear();

    out_isTestingOutput = false;
}

void out_set_dimmer(uint8_t value)
{
    if (!isOutputEnabled())
        return;

    out_curDimmerScale = (float_t)value / (float_t)255;

    if (!isOutputEnabled())
        return;

    out_reset_fade();
}

void out_set_scene(uint8_t sceneIdx)
{
    if (!isOutputEnabled() ||
        sceneIdx == out_curSceneIdx ||
        sceneIdx < 1 || sceneIdx > NUM_SCENES)
        return;

    // Flag all channels to their default "unassigned" state, so that we can see it while merging
    for (uint16_t i = 0; i < out_poolSize; i++)
        out_poolPropIsInScene[i] = false;

    if (sceneIdx > 0)
    {
        char *sceneTemplate = 0;
        switch (sceneIdx)
        {
            case 1:
                sceneTemplate = config.mqtt_dmx_scene_a;
                break;
            case 2:
                sceneTemplate = config.mqtt_dmx_scene_b;
                break;
            case 3:
                sceneTemplate = config.mqtt_dmx_scene_c;
                break;
            case 4:
                sceneTemplate = config.mqtt_dmx_scene_d;
                break;
        }

        sceneTemplate = deblank(sceneTemplate);

        char *tok;
        tok = strtok(sceneTemplate, ",");

        while (tok != NULL)
        {
            uint16_t addr = strtoul(tok, NULL, 0);
            uint8_t value = strtoul(strchr(tok, '=') + 1, NULL, 0);

            if (addr > 0 && addr <= out_poolSize)
            {
                out_poolPropIsInScene[addr - 1] = true;
                out_poolScene[addr - 1] = value;
            }
            logf_P(LOG_DEBUG, PSTR("[Scene] Setting channel %u at value %u"), addr, value);
            tok = strtok(0, ",");
        }
    }

    out_curSceneIdx = sceneIdx;

    out_reset_fade();
    //dmx_merge(false);
}

void out_reset_fade()
{
    if (!isOutputEnabled() ||
        config.mqtt_dmx_fadetime == 0)
        return;

    memcpy(out_poolFadeStartValues, out_poolMerged, out_poolSize);
    out_tLastFadeStart = millis();
}

void dmx_handle_rdm_portA(rdm_data *data)
{
    artRDM.rdmResponse(data, artnet_portA[0], artnet_portA[1]);
}
void dmx_handle_rdm_portB(rdm_data *data)
{
    artRDM.rdmResponse(data, artnet_portB[0], artnet_portB[1]);
}
void dmx_handle_tod_portA()
{
#ifndef DEBUG
    artRDM.artTODData(artnet_portA[0], artnet_portA[1], dmxA.todMan(), dmxA.todDev(), dmxA.todCount(), dmxA.todStatus());
#endif    
}
void dmx_handle_tod_portB()
{
    artRDM.artTODData(artnet_portB[0], artnet_portB[1], dmxB.todMan(), dmxB.todDev(), dmxB.todCount(), dmxB.todStatus());
}
