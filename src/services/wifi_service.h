#include <WiFi.h>
#include <IPAddress.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "../communication/logger.h"
class WifiService
{
private:
    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;
    const char* ssid;
    const char* password;
public:
    WifiService(const char* ssid, const char* password);
    void init(IPAddress local_IP, IPAddress gateway, IPAddress subnet, IPAddress primaryDNS, IPAddress secondaryDNS);

};
