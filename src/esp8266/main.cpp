#include "http.h"
#include "Configuration.h"
#include "ServerConnection.h"
#include "../IPC/Message.h"
#include "../IPOCS/Message.h"
#include "ArduinoConnection.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <list>

#define MIN_LOOP_TIME 100
#define PING_TIME 1000
#define WIFI_CONNECT 10000

std::list<WiFiEventHandler> wifiHandlers;

unsigned long connectionInitiated = 0;
int attemptNo = 0;
unsigned long lastPing = 0;

int onStationModeConnected(const WiFiEventStationModeConnected& change) {
  return 0;
}

int onStationModeDisconnected(const WiFiEventStationModeDisconnected& change) {
  esp::ServerConnection::instance().disconnect();
  connectionInitiated = millis();
  return 0;
}

int onStationModeGotIP(const WiFiEventStationModeGotIP& change) {
  return 0;
}

int onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& change) {
  return 0;
}

int onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& change) {
  return 0;
}

int onSoftAPModeProbeRequestReceived(const WiFiEventSoftAPModeProbeRequestReceived& change) {
  return 0;
}

void setup(void)
{
  char name[20];
  sprintf(name, "ipocs_%d", Configuration::getUnitID());
  MDNS.begin(name);
  esp::Http::instance().setup();
  esp::ArduinoConnection::instance().begin();
  //WiFi.onStationModeConnected(onStationModeConnected);
  wifiHandlers.push_back(WiFi.onStationModeDisconnected(onStationModeDisconnected));
  //wifiHandlers.push_back(WiFi.onStationModeGotIP(onStationModeGotIP));
  //WiFi.onSoftAPModeStationConnected(onSoftAPModeStationConnected);
  //WiFi.onSoftAPModeStationDisconnected(onSoftAPModeStationDisconnected);
  lastPing = millis();
  connectionInitiated = millis() - WIFI_CONNECT;
}

void loop(void)
{
  MDNS.update();
  int wiFiStatus = WiFi.status();
  if (wiFiStatus != WL_CONNECTED)
  {
    if ((millis() - connectionInitiated) >= WIFI_CONNECT)
    {
      MDNS.notifyAPChange();
      connectionInitiated = millis();
      if (attemptNo < 2)
      {
        char ssid[33];
        Configuration::getSSID(ssid);
        char password[61];
        Configuration::getPassword(password);
        esp::Http::instance().log("Connecting to SSID: " + String(ssid));
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        attemptNo++;
      }
      else if (attemptNo < 3)
      {
        esp::Http::instance().log(String("Starting SoftAP"));
        WiFi.mode(WIFI_AP);
        char name[20];
        sprintf(name, "IPOCS_%04X", Configuration::getUnitID());
        WiFi.softAP(name);
        attemptNo = 6;
      }
    }
  }
  else
  {
    if (attemptNo == 6 && connectionInitiated != 0 && (millis() - connectionInitiated) <= WIFI_CONNECT)
    {
      attemptNo = 0;
      connectionInitiated = 0;
      esp::Http::instance().log(String("IP address: ") + WiFi.localIP().toString());
      MDNS.notifyAPChange();
    }
  }
  esp::Http::instance().loop();
  esp::ServerConnection::instance().loop(wifi_station_get_connect_status() == STATION_GOT_IP);
  esp::ArduinoConnection::instance().loop();

  unsigned long loopEnd = millis();
  if ((loopEnd - lastPing) >= PING_TIME) {
    lastPing = loopEnd;
    IPC::Message* ipcPing = IPC::Message::create();
    ipcPing->RT_TYPE = IPC::IPING;
    ipcPing->setPayload();
    esp::ArduinoConnection::instance().send(ipcPing);
    delete ipcPing;
  }
}