#include "Configuration.h"

#include <EEPROM.h>
#include <util/crc16.h>

void Configuration::getMAC(byte MAC[6])
{
  MAC[0] = EEPROM.read(2);
  MAC[1] = EEPROM.read(3);
  MAC[2] = EEPROM.read(4);
  MAC[3] = EEPROM.read(5);
  MAC[4] = EEPROM.read(6);
  MAC[5] = EEPROM.read(7);
}

void Configuration::setMAC(byte newMAC[6])
{
  for (int i = 2; i < 8; i++)
  {
    EEPROM.write(i, newMAC[i - 2]);
  }
}

String Configuration::getSSID()
{
  byte ssid[6];
  Configuration::getMAC(ssid);
  String s_ssid;
  for (int i = 0; i < 6; i++)
    s_ssid += String((char)ssid[i]);
  return s_ssid;
}

void Configuration::setSSID(String ssid)
{
  byte bssid[7];
  ssid.getBytes(bssid, 7);
  Configuration::setMAC(bssid);
}

String Configuration::getPassword()
{
  String pwd;
  for (int i = 0; i < 8; i++)
    pwd += String((char)EEPROM.read(24 + i));
  return pwd;
}

void Configuration::setPassword(String pwd)
{
  byte bpwd[9];
  pwd.getBytes(bpwd, 9);
  for (int i = 0; i < 8; i++)
  {
    EEPROM.write(24 + i, bpwd[i]);
  }
}

unsigned int Configuration::getUnitID()
{
  unsigned int value = (EEPROM.read(0) << 8) + EEPROM.read(1);
  return value;
}

void Configuration::setUnitID(unsigned int unitID)
{
  EEPROM.write(0, unitID >> 8);
  EEPROM.write(1, unitID & 0xFF);
}

byte Configuration::getSD(byte data[], int dataLength)
{
  byte storedLength = EEPROM.read(32);
  // For unitialized memory, the value is 0xFF. If that occurs - return 0 and nothing will be loaded.
  if (storedLength == 0xFF)
    storedLength = 0x00;
  for (int i = 0; i < storedLength; i++)
  {
    data[i] = EEPROM.read(33 + i);
  }
  return storedLength;
}

uint16_t checkcrc(uint8_t* data, uint16_t len)
{
  uint16_t crc = 0xFFFF, i;
  for (i = 0; i < len; i++)
    crc = _crc_ccitt_update(crc, data[i]);
  return crc; // must be 0
}

void Configuration::setSD(byte data[], int dataLength)
{
  EEPROM.write(32, dataLength);
  for (int i = 0; i < dataLength; i++)
  {
    EEPROM.write(33 + i, data[i]);
  }

  uint16_t crc = checkcrc(data, dataLength);
  EEPROM.write(8, crc >> 8);
  EEPROM.write(9, crc & 0xFF);
}

uint16_t Configuration::getSiteDataCrc()
{
  return (EEPROM.read(8) << 8) + EEPROM.read(9);
}

bool Configuration::verifyCrc()
{
  uint16_t storedCrc = Configuration::getSiteDataCrc();

  uint8_t data[100];
  uint8_t sdLength = Configuration::getSD(data, 0x100);
  uint16_t calculatedCrc = checkcrc(data, sdLength);
  return (sdLength != 0 && calculatedCrc == storedCrc);
}
