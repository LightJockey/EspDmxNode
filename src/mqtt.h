#ifndef ESPDMXNODE_MQTT_H
#define ESPDMXNODE_MQTT_H

#include <AsyncMqttClient.h>

AsyncMqttClient mqttClient;

bool mqtt_connected;
char mqtt_topic_state[sizeof(config.mqtt_topic_prefix) + 1 + sizeof(config.mqtt_topic_group) + 6]; // 48 + 1 + 16 + 6 ("/state")
char mqtt_topic_dimmer[sizeof(config.mqtt_topic_prefix) + 1 + sizeof(config.mqtt_topic_group) + 7]; // 48 + 1 + 16 + 7 ("/dimmer")
char mqtt_topic_scene[sizeof(config.mqtt_topic_prefix) + 1 + 6]; // 48 + 1 + 6 ("/scene")

void mqtt_subscribe(char *topic)
{
    logf_P(LOG_MQTT, PSTR("Subscribing to topic \"%s\" ..."), topic);
    mqttClient.subscribe(topic, 0);
}
void mqtt_connect()
{
    if (!config.mqtt_enabled ||
        (config.mqtt_broker_address[0] == 0 && config.mqtt_broker_address[1] == 0 && config.mqtt_broker_address[2] == 0 && config.mqtt_broker_address[3] == 0))
        return;

    if (mqtt_connected)
        return;

    snprintf_P(mqtt_topic_state, sizeof(mqtt_topic_state), PSTR("%s/%s%s"), config.mqtt_topic_prefix, config.mqtt_topic_group, API_PATH_STATE);
    snprintf_P(mqtt_topic_dimmer, sizeof(mqtt_topic_dimmer), PSTR("%s/%s%s"), config.mqtt_topic_prefix, config.mqtt_topic_group, API_PATH_DIMMER);
    snprintf_P(mqtt_topic_scene, sizeof(mqtt_topic_scene), PSTR("%s%s"), config.mqtt_topic_prefix, API_PATH_SCENE);

    mqttClient.onConnect([](bool sessionPresent) {
        mqtt_connected = true;
        logf_P(LOG_MQTT, PSTR("Connected to broker %s:%u, session present: %d"), printNetAddr(config.mqtt_broker_address), config.mqtt_broker_port, sessionPresent);

        mqtt_subscribe(mqtt_topic_state);
        mqtt_subscribe(mqtt_topic_dimmer);
        mqtt_subscribe(mqtt_topic_scene);
    });
    mqttClient.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
        if (mqttClient.connected())
            mqttClient.disconnect(true);
        mqtt_connected = false;
        logf_P(LOG_MQTT, PSTR("Client disconnected (%u)"), reason);
        //mqtt_connect();
    });
    mqttClient.onSubscribe([](uint16_t packetId, uint8_t qos) {
        logf_P(LOG_MQTT, PSTR("Sub acknowledged. packetId: %u, qos: %u"), packetId, qos);
    });

    logf_P(LOG_MQTT, PSTR("Connecting to broker at %s:%u ..."), printNetAddr(config.mqtt_broker_address), config.mqtt_broker_port);
    mqttClient.setServer(printNetAddr(config.mqtt_broker_address), config.mqtt_broker_port);
    mqttClient.setClientId(hostname);
    mqttClient.connect();
}

#endif
