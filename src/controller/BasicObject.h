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

#include <stdint.h>

namespace IPOCS {
  class Packet;
}

class BasicObject {
  public:
    void init(const char objectName[], const uint8_t* configData, int configDataLen);
    virtual void handleOrder(IPOCS::Packet* basePacket) = 0;
    virtual void loop() = 0;
    virtual bool hasName(const char* const objectName);
    void sendStatus();
  protected:
    virtual IPOCS::Packet* getStatusPacket() = 0;
    virtual void objectInit(const uint8_t* configData, int configDataLen) = 0;
    char* objectName;
};

typedef BasicObject* (*initObjectFunction)();

#endif
