#include "ArduinoConnection.h"
#include "ServerConnection.h"
#include "Configuration.h"
#include "http.h"
#include "../IPC/Message.h"
#include "../IPOCS/Message.h"

void onPacketReceived(const uint8_t* buffer, size_t size);

esp::ArduinoConnection& esp::ArduinoConnection::instance() {
    static esp::ArduinoConnection conn;
    return conn;
}

void esp::ArduinoConnection::begin() {
    pinMode(LED_BUILTIN, OUTPUT);
    this->packetSerial = new SLIPPacketSerial();
    this->packetSerial->begin(115200);
    this->packetSerial->setPacketHandler(&onPacketReceived);
}

void esp::ArduinoConnection::loop() {
    this->packetSerial->update();
}

void onPacketReceived(const uint8_t* buffer, size_t size)
{
  if (size < 4) {
    return;
  }
  if (!IPC::Message::verifyChecksum(buffer)) {
    esp::Http::instance().log("Message CRC didn't match");
  }
  // Make a temporary buffer.
  IPC::Message* ipcMsg = IPC::Message::create(buffer);
  switch(ipcMsg->RT_TYPE) {
    case IPC::STARTED: {
      IPC::Message* ipcSiteData = IPC::Message::create();
      ipcSiteData->RT_TYPE = IPC::SITEDATA;
      ipcSiteData->setPayload();
      if (Configuration::verifyCrc()) {
        uint8_t siteData[200];
        uint8_t siteDataLength = Configuration::getSD(siteData, 200);
        ipcSiteData->setPayload(siteData, siteDataLength);
      } else {
        digitalWrite(LED_BUILTIN, LOW);
      }
      esp::ArduinoConnection::instance().send(ipcSiteData);
      delete ipcSiteData;
      break; }
    case IPC::SITEDATA: {
      // Not handled in ESP
      break; }
    case IPC::IPOCS: {
      IPOCS::Message* ipocsMsg = IPOCS::Message::create(ipcMsg->pld);
      esp::ServerConnection::instance().send(ipocsMsg);
      delete ipocsMsg;
      break; }
    case IPC::RESTART: {
      break; }
    case IPC::IPONG: {
      break; }
    case IPC::IPING: {
      break; }
  }
  delete ipcMsg;
}

void esp::ArduinoConnection::send(IPC::Message* msg) {
    uint8_t buffer[msg->RL_MESSAGE + 2];
    size_t msgSize = msg->serialize(buffer);
    this->packetSerial->send(buffer, msgSize);
}

void esp::ArduinoConnection::log(const String& msg) {
    IPC::Message* ipc = IPC::Message::create();
    ipc->RT_TYPE = IPC::ILOG;
    ipc->setPayload();
    ipc->setPayload((uint8_t*)msg.c_str(), msg.length() + 1);
    esp::ArduinoConnection::instance().send(ipc);
    delete ipc;
}