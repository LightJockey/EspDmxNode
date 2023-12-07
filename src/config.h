#ifndef ESPDMXNODE_CONFIG_H
#define ESPDMXNODE_CONFIG_H

#define CONFIG_VERSION 1
#define CONFIG_JSON_PADDING 128

#include <EEPROM.h>
#include <ArduinoJson.h>

const uint8_t CONFIG_ID[4] PROGMEM = { 'C', 'O', 'N', 'F' };

config_t d_config;
char config_string_buf[sizeof(config_t) + CONFIG_JSON_PADDING + 1];

void config_init()
{
    memset(&d_config, 0, sizeof(d_config));

    memcpy_P(d_config.id, CONFIG_ID, sizeof(d_config.id));
    d_config.version = CONFIG_VERSION;

    // INIT CONFIG
    // Note: only init values that are different from their type's default
    // Misc
    strlcpy(d_config.node_name, default_node_name, sizeof(d_config.node_name));

    // Wifi
    strlcpy(d_config.wifi_ssid, "", sizeof(d_config.wifi_ssid));

    // Network
    d_config.dhcp = 1;
    d_config.ip[0] = 192;
    d_config.ip[1] = 168;
    d_config.ip[2] = 1;
    d_config.ip[3] = 100;
    d_config.netmask[0] = 255;
    d_config.netmask[1] = 255;
    d_config.netmask[2] = 255;
    d_config.netmask[3] = 0;

    // ArtNet
    d_config.artnet_enabled = 1;

    // DMX
    d_config.port_a.enabled = 1;
    d_config.port_a.num_channels = d_config.port_a.type == PORT_DMX ? DMX_UNI_LEN : 0;
    d_config.port_a.brightness = 255;
    d_config.port_a.smoothingthreshold = 25;
    d_config.port_b.num_channels = d_config.port_b.type == PORT_DMX ? DMX_UNI_LEN : 0;
    d_config.port_b.brightness = 255;
    d_config.port_b.smoothingthreshold = 25;

    // MQTT
    d_config.mqtt_broker_port = 1883;
    strlcpy(d_config.mqtt_topic_prefix, "", sizeof(d_config.mqtt_topic_prefix));
    strlcpy(d_config.mqtt_topic_group, "", sizeof(d_config.mqtt_topic_group));

    strlcpy(d_config.mqtt_dmx_scene_a, "", sizeof(d_config.mqtt_dmx_scene_a));
    strlcpy(d_config.mqtt_dmx_scene_b, "", sizeof(d_config.mqtt_dmx_scene_b));
    strlcpy(d_config.mqtt_dmx_scene_c, "", sizeof(d_config.mqtt_dmx_scene_c));
    strlcpy(d_config.mqtt_dmx_scene_d, "", sizeof(d_config.mqtt_dmx_scene_d));
}

char* config_print()
{
    config_string_buf[0] = 0;
    // ALWAYS use the snprintf variant to prevent overflows, ffs
    snprintf_P(config_string_buf, sizeof(config_string_buf), PSTR("[\"%s\",\"%s\",  \"%s\",\"%s\",  \"%s\",\"%u\",\"%s\",\"%s\",\"%s\",  \"%u\",  \"%u\",  \"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",  \"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",\"%u\",  \"%u\",\"%s\",\"%u\",\"%s\",\"%s\",\"%u\",\"%u\",\"%s\",\"%s\",\"%s\",\"%s\"]"),

        config.node_name,
        FIRMWARE_VERSION,

        config.wifi_ssid,
        config.wifi_passphrase,

        WiFi.macAddress().c_str(),
        config.dhcp,
        wifi_mode > 0 ? config.dhcp ? WiFi.localIP().toString().c_str() : config.ip[0] == 0 ? WiFi.localIP().toString().c_str() : printNetAddr(config.ip) : "",
        wifi_mode > 0 ? config.dhcp ? WiFi.subnetMask().toString().c_str() : config.netmask[0] == 0 ? WiFi.subnetMask().toString().c_str() : printNetAddr(config.netmask) : "",
        wifi_mode > 0 ? config.dhcp ? WiFi.gatewayIP().toString().c_str() : config.gateway[0] == 0 ? WiFi.gatewayIP().toString().c_str() : printNetAddr(config.gateway) : "",

        config.artnet_enabled,

        config.e131_enabled,

        config.port_a.enabled,
        config.port_a.type,
        config.port_a.rdm_enabled,
        config.port_a.num_channels,
        config.port_a.universe,
        config.port_a.offset,
        config.port_a.brightness,
        config.port_a.smoothtime,
        config.port_a.smoothingthreshold,

        config.port_b.enabled,
        config.port_b.type,
        config.port_b.rdm_enabled,
        config.port_b.num_channels,
        config.port_b.universe,
        config.port_b.offset,
        config.port_b.brightness,
        config.port_b.smoothtime,
        config.port_b.smoothingthreshold,

        config.mqtt_enabled,
        printNetAddr(config.mqtt_broker_address),
        config.mqtt_broker_port,
        config.mqtt_topic_prefix,
        config.mqtt_topic_group,
        config.mqtt_dmx_mergemode,
        config.mqtt_dmx_fadetime,
        config.mqtt_dmx_scene_a,
        config.mqtt_dmx_scene_b,
        config.mqtt_dmx_scene_c,
        config.mqtt_dmx_scene_d

    );
    return config_string_buf;
}

void config_read()
{
    EEPROM.begin(sizeof(config));
    EEPROM.get(EEPROM_OFFSET, config);
    EEPROM.end();
}

void config_write()
{
    EEPROM.begin(sizeof(config));
    EEPROM.put(EEPROM_OFFSET, config);
    EEPROM.commit();
    EEPROM.end();
    char *config_s = config_print();
    logf_P(LOG_CONFIG, PSTR("Flashed config: %s"), config_s);
}

void config_reset()
{
    config = d_config;
    config_write();
    log_P(LOG_CONFIG, PSTR("Config reset, default config saved"));
}

void config_load()
{
    config_read();

    if (memcmp(config.id, d_config.id, sizeof(config.id)) != 0)
    {
        log_P(LOG_CONFIG, PSTR("No config found in flash"));
        logf_P(LOG_DEBUG, PSTR("default id: %s, flash id: %s"), d_config.id, config.id);
        config_reset();
    }
    else if (config.version != CONFIG_VERSION)
    {
        logf_P(LOG_CONFIG, PSTR("Config v%u found, but we are at v%u. Upgrade / reinitialization needed!"), config.version, CONFIG_VERSION);

        log_P(LOG_CONFIG, PSTR("Cannot update config. Reinitializing ..."));
        config_reset();
    }
    else
    {
        char *config_s = config_print();
        logf_P(LOG_CONFIG, PSTR("Loaded config: %s"), config_s);
    }
}

void config_save()
{
    config_write();
    log_P(LOG_CONFIG, PSTR("New config saved"));
}

bool config_parse(char *data, size_t size)
{
    StaticJsonDocument<sizeof(config_t) + CONFIG_JSON_PADDING> config_json;

    logf_P(LOG_CONFIG, PSTR("Parsing new config json: %s"), data);
    DeserializationError err = deserializeJson(config_json, data, size);
    if (err)
    {
        logf_P(LOG_ERROR, PSTR("Error while parsing json: %s"), err.c_str());
        return false;
    }

    // Misc
    strlcpy(config.node_name, config_json[0] | "", sizeof(config.node_name));

    // Wifi
    strlcpy(config.wifi_ssid, config_json[2] | "", sizeof(config.wifi_ssid));
    strlcpy(config.wifi_passphrase, config_json[3] | "", sizeof(config.wifi_passphrase));

    // Network
    config.dhcp = config_json[5];
    if (!config.dhcp)
    {
        parseNetAddr(config.ip, config_json[6] | "");
        parseNetAddr(config.netmask, config_json[7] | "");
        parseNetAddr(config.gateway, config_json[8] | "");
    }

    // ArtNet
    config.artnet_enabled = config_json[9];

    // e1.31 (sACN)
    config.e131_enabled = config_json[10];

    // PORTS
    config.port_a.enabled = config_json[11];
    if (config.port_a.enabled == 1)
    {
        config.port_a.type = config_json[12];
        config.port_a.rdm_enabled = config_json[13];
        config.port_a.num_channels = config_json[14];
        config.port_a.universe = config_json[15];
        config.port_a.offset = config_json[16];
        config.port_a.brightness = config_json[17];
        config.port_a.smoothtime = config_json[18];
        config.port_a.smoothingthreshold = config_json[19];
    }
    config.port_b.enabled = config_json[20];
    if (config.port_b.enabled == 1)
    {
        config.port_b.type = config_json[21];
        config.port_b.rdm_enabled = config_json[22];
        config.port_b.num_channels = config_json[23];
        config.port_b.universe = config_json[24];
        config.port_b.offset = config_json[25];
        config.port_b.brightness = config_json[26];
        config.port_b.smoothtime = config_json[27];
        config.port_b.smoothingthreshold = config_json[28];
    }

    // MQTT
    config.mqtt_enabled = config_json[29];
    if (config.mqtt_enabled == 1)
    {
        parseNetAddr(config.mqtt_broker_address, config_json[30] | "");
        config.mqtt_broker_port = config_json[31];
        strlcpy(config.mqtt_topic_prefix, config_json[32] | "", sizeof(config.mqtt_topic_prefix));
        strlcpy(config.mqtt_topic_group, config_json[33] | "", sizeof(config.mqtt_topic_group));
        config.mqtt_dmx_mergemode = config_json[34];
        config.mqtt_dmx_fadetime = config_json[35];
        strlcpy(config.mqtt_dmx_scene_a, config_json[36] | "", sizeof(config.mqtt_dmx_scene_a));
        strlcpy(config.mqtt_dmx_scene_b, config_json[37] | "", sizeof(config.mqtt_dmx_scene_b));
        strlcpy(config.mqtt_dmx_scene_c, config_json[38] | "", sizeof(config.mqtt_dmx_scene_c));
        strlcpy(config.mqtt_dmx_scene_d, config_json[39] | "", sizeof(config.mqtt_dmx_scene_d));
    }

    return true;
}

#endif
