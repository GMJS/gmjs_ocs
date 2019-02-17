/*****************
 * Object interface definition file.
 *
 * This file defines how to operate the a object.
 *
 * @author Fredrik Elestedt
 * @date
 * @history see the git log of the origin repository.
 */
#ifndef BASICOBJECT_H
#define BASICOBJECT_H

#include <SPI.h>

namespace IPOCS {
  class Packet;
}

class BasicObject {
  public:
    void init(String objectName, byte configData[], int configDataLen);
    virtual void handleOrder(IPOCS::Packet* basePacket) = 0;
    virtual void loop() = 0;
    virtual bool hasName(String objectName);
    String name() { return objectName; }
  protected:
    virtual void objectInit(byte configData[], int configDataLen) = 0;
    String objectName;
};

typedef BasicObject* (*initObjectFunction)();

#endif