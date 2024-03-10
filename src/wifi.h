#ifndef ESPDMXNODE_WIFI_H
#define ESPDMXNODE_WIFI_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>

const char *setup_ap_ssid = "EspDmxNode-";

uint8_t mac_address[6];

WiFiEventHandler wifiGotIpHandler, wifiConnectedHandler, wifiDisconnectedHandler;
uint8_t wifi_mode;
bool wifi_connected;
bool wifi_ap_started;

DNSServer dnsServer;

void wifi_connect(uint8_t mode)
{
    if (wifi_connected)
        return;
    
    // The ESP SDK can save wifi credentials by itself, disable this feature since we handle config ourselves
    WiFi.persistent(false);
    // Disconnect and delete SDK config in EEPROM
    WiFi.disconnect(true);

    // Disable any form of modem sleep state
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    // Set radio mode
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    // 1 - B
    // 2 - G
    // 3 - N
    //WiFi.setOutputPower(2.5);

    // Initialize in SoftAP mode when ssid not found in config
    if (mode == 0)
    {
        WiFi.mode(WIFI_AP);
        log_P(LOG_WIFI, PSTR("Starting in SoftAP mode"));

        IPAddress softAPIP = IPAddress(192, 168, 1, 1);
        WiFi.softAPConfig(softAPIP, softAPIP, IPAddress(255, 255, 255, 0));
        wifi_ap_started = WiFi.softAP(hostname);

        if (wifi_ap_started)
        {
            logf_P(LOG_WIFI, PSTR("Running in SoftAP mode (ssid: \"%s\", ip: %s)"), hostname, WiFi.softAPIP().toString().c_str());

            // Start a DNS server to redirect all requests to our IP when in SoftAP mode
            dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
            bool success = dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
            if (success)
                logf_P(LOG_WIFI, PSTR("Started DNS server on UDP %u (captive portal redirect)"), DNS_PORT);
            else
                log_P(LOG_ERROR, PSTR("Failed to start DNS server!"));
        }
        else
            log_P(LOG_ERROR, PSTR("Failed to start in SoftAP mode!"));
    }
    // If an ssid is found in the config then Initialize in STATION mode and connect to it
    else
    {
        WiFi.mode(WIFI_STA);
        wifi_station_set_hostname(hostname);

        logf_P(LOG_WIFI, PSTR("Running in STATION mode. Connecting to AP \"%s\" ..."), config.wifi_ssid);

        WiFi.begin(config.wifi_ssid, config.wifi_passphrase);

        if (!config.dhcp)
        {
            // Set static IP if DHCP is not enabled in config
            bool res = WiFi.config(IPAddress(config.ip[0], config.ip[1], config.ip[2], config.ip[3]),                      // ip
                                   IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3]),  // gateway
                                   IPAddress(config.netmask[0], config.netmask[1], config.netmask[2], config.netmask[3]),  // subnet
                                   IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3])); // dns1
            if (!res)
            {
                logf_P(LOG_WIFI, PSTR("Could not set static IP settings! Restarting in SoftAP mode ..."));
                wifi_mode = 0;
                wifi_connect(wifi_mode);
            }
        }
    }
}
void wifi_init()
{
    wifi_mode = 0;
    // Choose if we must initialize in STATION or SoftAP mode
    if (strlen(config.wifi_ssid) > 0)
        wifi_mode = 1;

    if ((wifi_mode == 0 && !wifi_ap_started) ||
        (wifi_mode == 1 && WiFi.status() != WL_CONNECTED && !wifi_connected))
    {
        log_P(LOG_WIFI, PSTR("Disconnected, will now try reconnecting until we succeed"));
        wifi_connect(wifi_mode);
    }
}
void wifi_setup()
{
    WiFi.macAddress(mac_address);
    snprintf_P(hostname, sizeof(hostname), PSTR("%s-%02X%02X"), config.node_name, mac_address[4], mac_address[5]);
    logf_P(LOG_DEBUG, PSTR("Hostname: %s"), hostname);

    wifiGotIpHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
        // Is STATION mode and connected
        if (wifi_mode > 0 && WiFi.status() == WL_CONNECTED && !wifi_connected)
        {
            wifi_connected = true; // Setting this flag because I'm not sure if this callback gets fired on DHCP renews as well

            logf_P(LOG_WIFI, PSTR("Got net settings (dhcp: %u, ip: %s, netmask: %s, gateway: %s, dns: %s)"),
                config.dhcp,
                WiFi.localIP().toString().c_str(),
                WiFi.subnetMask().toString().c_str(),
                WiFi.gatewayIP().toString().c_str(),
                WiFi.dnsIP().toString().c_str());

            artnet_init();

            if (mqttClient.connected())
                mqttClient.disconnect(true);

            mqtt_connected = false;
            mqtt_connect();
        }
    });
    wifiConnectedHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& event) {
        logf_P(LOG_WIFI, PSTR("Connected to AP \"%s\", channel %i"), WiFi.SSID().c_str(), WiFi.channel());
    });
    wifiDisconnectedHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
        wifi_connected = false;
        wifi_init();
    });
}
void wifi_handle()
{
    // STATION mode
    if (wifi_mode > 0 && wifi_connected)
        ArduinoOTA.handle();
    // SoftAP mode
    else if (wifi_ap_started)
        dnsServer.processNextRequest();
}

#endif