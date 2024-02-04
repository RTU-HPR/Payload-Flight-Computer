#include "Communication.h"

void Communication::beginWiFi(Config::WiFi_Config &wifi_config)
{
  // Copy the config
  this->wifi_config = wifi_config;

  // WiFi.setTxPower(WIFI_POWER_19_5dBm); // Set WiFi RF power output to highest level
  WiFi.begin(this->wifi_config.ssid, this->wifi_config.pass);

  Serial.println("Connecting to " + String(wifi_config.ssid) + " network ...");
}

void Communication::WiFiCheckConnection()
{
  int status = WiFi.status();
  if (status == WL_CONNECTED && connecetedToWiFi)
  {
    // All good, nothing to do here
    return;
  }
  else if (status == WL_CONNECTED && !connecetedToWiFi)
  {
    // No previous connection, but now connected
    // WiFi
    Serial.println("Connected to WiFi");
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
    Serial.print("Pico IP address: ");
    Serial.println(WiFi.localIP());

    // UDP
    tcUdp.begin(wifi_config.tcPort);
    tmUdp.begin(wifi_config.tmPort);
    Serial.println("UDP ports opened");

    connecetedToWiFi = true;
    return;
  }
  else if (connecetedToWiFi)
  {
    // Lost connection, but was previously connected
    connecetedToWiFi = false;
    Serial.print("WiFi lost connection...Reconnecting");

    // Stop UDP and WiFI
    tmUdp.stop();
    tcUdp.stop();
    WiFi.disconnect();
  }

  if (WiFi.begin(wifi_config.ssid, wifi_config.pass) == WL_CONNECTED)
  {
    // Try to connect, if not connected
    // WiFi
    Serial.println("Connected to WiFi");
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
    Serial.print("Pico IP address: ");
    Serial.println(WiFi.localIP());

    // UDP
    tcUdp.begin(wifi_config.tcPort);
    tmUdp.begin(wifi_config.tmPort);
    Serial.println("UDP ports opened");

    connecetedToWiFi = true;
  }
}

void Communication::WifiSendData(String &data)
{
  WiFiCheckConnection();
  
  // If connected to WiFi, check for any messages from UDP
  if (connecetedToWiFi)
  {
    tmUdp.beginPacket(tcUdp.remoteIP(), wifi_config.tmPort);
    tmUdp.print(data);
    tmUdp.endPacket();
  }
}