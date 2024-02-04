#pragma once
#include <Config.h>

class Communication
{
  public:
  // Wifi
  WiFiUDP tcUdp;
  WiFiUDP tmUdp;

  // Config needs to be copied to this variable in beginWiFi()
  // for us to be able to access config values in WiFi events
  Config::WiFi_Config wifi_config;

  bool connecetedToWiFi = false;
  bool remoteIpKnown = false;
  unsigned long lastUdpReceivedMillis = 0;

  void WiFiCheckConnection();

  void beginWiFi(Config::WiFi_Config &wifi_config);

  void WifiSendData(String &data);
};
