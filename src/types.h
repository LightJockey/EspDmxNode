#ifndef ESPDMXNODE_TYPES_H
#define ESPDMXNODE_TYPES_H

typedef struct {
    uint8_t			enabled;
    uint8_t			type;
    uint8_t			rdm_enabled;
    uint16_t		num_channels;
    uint16_t		universe;
    uint16_t		offset;
	uint8_t			brightness;
    uint8_t         smoothtime;
    uint8_t         smoothingthreshold;
} __attribute__((packed)) port_config_t;
typedef struct {
    // Header
    uint8_t			id[4];          				// Config structure signature
    uint8_t			version;        				// Config structure version
    uint8_t			reserved;       				// Reserved for future use

    // Misc
    char			node_name[32];  				// [0]

    // Wifi
    char			wifi_ssid[32];       			// [2]
    char			wifi_passphrase[64]; 			// [3]

    // Network
    uint8_t			dhcp;							// [5]
    uint8_t			ip[4];							// [6]
    uint8_t			netmask[4];						// [7]
    uint8_t			gateway[4];						// [8]

    // ArtNet
    uint8_t			artnet_enabled;					// [9]

    // e1.31 (sACN)
    uint8_t			e131_enabled;					// [10]

    // DMX
    port_config_t	port_a;							// [11 - 16]
    port_config_t	port_b;							// [17 - 22]

    // MQTT
    uint8_t			mqtt_enabled;					// [23]
    uint8_t			mqtt_broker_address[4];			// [24]
    uint16_t		mqtt_broker_port;				// [25]
    char			mqtt_topic_prefix[48];			// [26]
    char			mqtt_topic_group[16];			// [27]
    uint8_t			mqtt_dmx_mergemode;				// [28]
    uint16_t		mqtt_dmx_fadetime;				// [29]
    char			mqtt_dmx_scene_a[128];			// [30]
    char			mqtt_dmx_scene_b[128];			// [31]
    char			mqtt_dmx_scene_c[128];			// [32]
    char			mqtt_dmx_scene_d[128];			// [33]

} __attribute__((packed)) config_t;

#endif
