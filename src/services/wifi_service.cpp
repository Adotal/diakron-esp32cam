#include "wifi_service.h"
WifiService::WifiService(const char* ssid, const char* password)
    : ssid(ssid), password(password)
{}

void WifiService::init(IPAddress local_IP, IPAddress gateway, IPAddress subnet, IPAddress primaryDNS, IPAddress secondaryDNS)
{
    this->local_IP = local_IP;
    this->gateway = gateway;
    this->subnet = subnet;
    this->primaryDNS = primaryDNS;
    this->secondaryDNS = secondaryDNS;

    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA Failed to configure");
    }

    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Logger::info("Connecting to WiFi ..");
    Serial.print("[");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('=');
        delay(500);
    }
    Serial.println("]");
    String ipInfo = "Connected to WiFi with IP[" + WiFi.localIP().toString() + "]";
    Logger::info(ipInfo.c_str());
}