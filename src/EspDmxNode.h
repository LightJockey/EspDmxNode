#ifndef ESPDMXNODE_H
#define ESPDMXNODE_H

const char *FIRMWARE_VERSION = "4.0";

#define WIFI_CONNECT_TIMEOUT 30000
#define HTTP_PORT 80
#define DNS_PORT 53
#define EEPROM_OFFSET 0

#include <Arduino.h>
extern "C"
{
    #include "user_interface.h"
}
#include <errno.h>

const char *default_node_name = "EspDmxNode";
const char *default_ota_password = "dmxnodeota";

enum PORT_TYPE {
    PORT_DMX = 0,
    PORT_PIXELS = 1
};
#define DMX_UNI_LEN 512

#include "types.h"
config_t config;
char hostname[sizeof(config.node_name) + 6];

#define DEBUG_LED_PIN D0
#define DEBUG_LED_BLINK_CONNECTED_INTERVAL 500
#define DEBUG_LED_BLINK_DISCONNECTED_INTERVAL 1000
uint8_t DbgLed_State;
uint32_t tDbgLed_lastFlip;

#define RESET_PIN D3 // FLASH button on NodeMCU
#define RESET_TIME 5000
uint8_t resetPinState;
uint16_t t_resetAccumul;

uint32_t tUptime; // In seconds, rolls over in 136 years
uint16_t tUptimeAccumul;
uint32_t tLastFrame;

#define ARTNET_OEM 0x0123   // Artnet OEM Code
#define ESTA_MAN 0x0000     // ESTA Manufacturer Code
#define ESTA_DEV 0xEE000000 // RDM Device ID (used with Man Code to make 48bit UID)

#include "espArtNetRDM.h"

esp8266ArtNetRDM artRDM;
uint8_t artnet_portA[2];
uint8_t artnet_portB[2];
uint32_t artnet_tLastReceived;
uint16_t artnet_framesPerSecond;
void artnet_init();
void artnet_stop();
void artnet_handle_dmx(uint8_t, uint8_t, uint16_t, bool);
void artnet_handle_rdm(uint8_t, uint8_t, rdm_data *);
void artnet_handle_tod_request(uint8_t, uint8_t);
void artnet_handle_tod_flush(uint8_t, uint8_t);
void artnet_decode_uni(uint16_t fullUni, uint8_t *net, uint8_t *subnet, uint8_t *uni)
{
    *net = (fullUni >> 8) & 0x7F;
    *subnet = (fullUni >> 4) & 0xF;
    *uni = fullUni & 0xF;
}

#define DMXA_DIR_PIN D5
#define DMXB_DIR_PIN D6

#include "espDMX_RDM.h"
#include <NeoPixelBus.h>

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart0800KbpsMethod> *pixelsA = NULL;
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> *pixelsB = NULL;

uint8_t out_numPorts;
uint16_t out_poolSize;
uint8_t *out_poolPrev;
uint8_t *out_poolCur;
uint8_t *out_poolMerged;
uint8_t *out_poolScene;
bool *out_poolPropIsInScene;
uint8_t *out_poolFadeStartValues;
float_t out_curDimmerScale;
uint8_t out_curSceneIdx;
uint32_t out_tLastFrameReceived;
uint32_t out_tLastFadeStart;
float_t out_a_brightness_scale;
float_t out_b_brightness_scale;
bool out_isTestingOutput;

void out_setup();
void out_set_uni(uint8_t port, uint8_t *data, uint16_t numChans);
void out_merge(bool isLoop);
void out_send();
void out_clear();
void out_stop();
void out_test();
void out_set_dimmer(uint8_t value);
void out_set_scene(uint8_t sceneIdx);
void out_reset_fade();

void dmx_handle_rdm_portA(rdm_data *data);
void dmx_handle_rdm_portB(rdm_data *data);
void dmx_handle_tod_portA();
void dmx_handle_tod_portB();

inline void out_set_channel(uint16_t addr, uint8_t value)
{
    memset(out_poolMerged + addr, value, 1);
}
inline bool isOutputEnabled()
{
    return out_numPorts > 0 && !out_isTestingOutput;
}
inline bool isRdmEnabled()
{
    return config.port_a.rdm_enabled || config.port_b.rdm_enabled;
}
inline bool isValidUni(uint16_t uni)
{
    return (config.port_a.enabled && uni == config.port_a.universe) ||
           (config.port_b.enabled && uni == config.port_b.universe);
}
inline bool isValidDmxPort(uint8_t port)
{
    bool isEnabled = port == 0 ? config.port_a.enabled : config.port_b.enabled;
    return port < out_numPorts && isEnabled;
}

void restart()
{
    artRDM.end();
    out_stop();
    yield();
    ESP.restart();
}

char *status_print()
{
    static char buf[256];
    buf[0] = 0;
    snprintf_P(buf, sizeof(buf), PSTR("[\"%s\", \"%s %s\", \"%lu\", \"%lu\", \"%u\"]"),
               FIRMWARE_VERSION,
               __DATE__, __TIME__,
               tUptime,
               system_get_free_heap_size(),
               (config.artnet_enabled ? artnet_framesPerSecond : 0));
    return buf;
}

#include "utils.h"
#include "log.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "web.h"

#endif
