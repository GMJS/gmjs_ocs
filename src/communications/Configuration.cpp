#include "Configuration.h"

#include <EEPROM.h>
#include <uCRC16Lib.h>

void Configuration::getUnitName(char unitName[])
{
  uint16_t value = (EEPROM.read(0) << 8) + EEPROM.read(1);
  if (value > 30) {
    #ifdef ESP8266
    sprintf(unitName, "ipocs_%06x", ESP.getChipId());
    #else
    sprintf(unitName, "ipocs_%06x", (uint32_t)ESP.getEfuseMac());
    #endif
    return;
  }
  size_t i;
  for (i = 0; i < value; i++) {
    unitName[i] = EEPROM.read(990 + i);
    if (unitName[i] == 0x00) {
      break;
    }
  }
  unitName[value] = 0x00;
  if (i+1 != value || unitName[i] != 0x00) {
    #ifdef ESP8266
    sprintf(unitName, "ipocs_%06x", ESP.getChipId());
    #else
    sprintf(unitName, "ipocs_%06x", (uint32_t)ESP.getEfuseMac());
    #endif
  }
}

void Configuration::setUnitName(const char unitName[])
{
  uint8_t unitID = strlen(unitName) + 1;
  EEPROM.write(0, unitID >> 8);
  EEPROM.write(1, unitID & 0xFF);
  for (int i = 0; i < unitID; i++) {
    EEPROM.write(990 + i, unitName[i]);
  }
  EEPROM.write(990 + unitID, 0x00);
  EEPROM.commit();
}

size_t Configuration::getSSID(char ssid[]) {
  for (uint16_t i = 0; i < 33; i++) {
    ssid[i] = EEPROM.read(i + 2);
  }
  // Force null termination for uninitialized cards
  if (ssid[0] == 0xFF) {
    ssid[0] = 0x00;
  }
  ssid[32] = 0x00; // Force null termination just in case
  return strlen(ssid);
}

void Configuration::setSSID(const char ssid[])
{
  for (uint16_t i = 0; i < strlen(ssid); i++) {
    EEPROM.write(i + 2, ssid[i]);
  }
  EEPROM.write(strlen(ssid) + 2 , 0);
  EEPROM.commit();
}

size_t Configuration::getPassword(char pwd[])
{
  for (uint16_t i = 0; i < 61; i++) {
    pwd[i] = EEPROM.read(i + 36);
  }
  // Force null termination for uninitialized cards
  if (pwd[0] == 0xFF) {
    pwd[0] = 0x00;
  }
  pwd[60] = 0x00; // Force null termination just in case
  return strlen(pwd);
}

void Configuration::setPassword(const char pwd[])
{
  for (uint16_t i = 0; i < strlen(pwd); i++) {
    EEPROM.write(i + 36, pwd[i]);
  }
  EEPROM.write(strlen(pwd) + 36, 0);
  EEPROM.commit();
}

uint8_t Configuration::getSD(uint8_t data[], int dataLength)
{
  uint8_t storedLength = EEPROM.read(100);
  // For unitialized memory, the value is 0xFF. If that occurs - return 0 and nothing will be loaded.
  if (storedLength == 0xFF)
    storedLength = 0x00;
  for (int i = 0; i < storedLength; i++)
  {
    data[i] = EEPROM.read(101 + i);
  }
  return storedLength;
}

void Configuration::setSD(const uint8_t data[], int dataLength)
{
  EEPROM.write(100, dataLength);
  for (int i = 0; i < dataLength; i++)
  {
    EEPROM.write(101 + i, data[i]);
  }
  uint16_t crc = uCRC16Lib::calculate(data, dataLength);
  EEPROM.write(98, crc >> 8);
  EEPROM.write(99, crc & 0xFF);
  EEPROM.commit();
}

uint16_t Configuration::getSiteDataCrc()
{
  return (EEPROM.read(98) << 8) + EEPROM.read(99);
}

bool Configuration::verifyCrc()
{
  uint16_t storedCrc = Configuration::getSiteDataCrc();

  uint8_t data[100];
  uint8_t sdLength = Configuration::getSD(data, 0x100);
  uint16_t calculatedCrc = uCRC16Lib::calculate(data, sdLength);
  return (sdLength != 0 && calculatedCrc == storedCrc);
}
