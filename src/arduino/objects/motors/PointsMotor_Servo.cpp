#include <SPI.h>

#include "PointsMotorStore.h"
#include "PointsMotor_Servo.h"
#include "../../../IPOCS/Packets/ThrowPointsPacket.h"
#include "../../log.h"

#define NO_POSITION_INPUT 0
const int servoRightVal = 20;
const int servoLeftVal = 150;
const int pointStepTime = 20;
// Left state is less than or equal to this value.
const int StateLeft = 399;
// Moving/Out Of Control state is less than or equal to this value (Out Of Control after 10s)
const int StateMoving = 799;
// Right state is less than or equal to this value.
const int StateRight = 1023;

PointsMotor_Servo::PointsMotor_Servo()
{
}

int PointsMotor_Servo::objectInit(const uint8_t configData[])
{
  this->lastRun = millis();
  this->setPos = servoRightVal;
  this->curPos = servoRightVal;

  this->object.attach(configData[1] + 1);
  switch (configData[3]) {
    case 1: this->posInput = A0; break;
    case 2: this->posInput = A1; break;
    case 3: this->posInput = A2; break;
    case 4: this->posInput = A3; break;
    case 5: this->posInput = A4; break;
    case 6: this->posInput = A5; break;
    case 7: this->posInput = A6; break;
    case 8: this->posInput = A7; break;
#ifdef PIN_A8
    case 9: this->posInput = A8; break;
#endif
#ifdef PIN_A9
    case 10: this->posInput = A9; break;
#endif
#ifdef PIN_A10
    case 11: this->posInput = A10; break;
#endif
#ifdef PIN_A11
    case 12: this->posInput = A11; break;
#endif
#ifdef PIN_A12
    case 13: this->posInput = A12; break;
#endif
#ifdef PIN_A13
    case 14: this->posInput = A13; break;
#endif
#ifdef PIN_A14
    case 15: this->posInput = A14; break;
#endif
#ifdef PIN_A15
    case 16: this->posInput = A15; break;
#endif
    default: this->posInput = NO_POSITION_INPUT; break;
  }
  return 3;
}

void PointsMotor_Servo::handleOrder(IPOCS::Packet* basePacket)
{
  if (basePacket->RNID_PACKET == 10)
  {
    IPOCS::ThrowPointsPacket* packet = (IPOCS::ThrowPointsPacket*)basePacket;
    switch (packet->RQ_POINTS_COMMAND)
    {
      case IPOCS::ThrowPointsPacket::DIVERT_RIGHT: this->setPos = servoRightVal; break;
      case IPOCS::ThrowPointsPacket::DIVERT_LEFT: this->setPos = servoLeftVal; break;
      default:
        // TODO: Send error about invalid value
        break;
    }
  }
}

void PointsMotor_Servo::loop()
{
  if (millis() - this->lastRun > pointStepTime) {
    this->lastRun = millis();
    // For each object, handle it.
    if (this->setPos == this->curPos)
      return;
    // Else determine which direction to move.
    int direction = this->setPos > this->curPos ? +1 : -1;
    // And then move
    this->curPos += direction;
    this->object.write(this->curPos);

    // And possibly notify on reaching end position (obsolete if we have an input to look at)
  }
}

IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE PointsMotor_Servo::getState()
{
  IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE pos = IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::OUT_OF_CONTROL;
  if (this->posInput == NO_POSITION_INPUT) {
    if (this->curPos == this->setPos) {
      pos = this->setPos == servoLeftVal ? IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::LEFT : IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::RIGHT;
    }
    else pos = IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::MOVING;
    // TODO Add 10s timeout
  } else {
    int posValue = analogRead(this->posInput);
    if (posValue < StateLeft) {
      pos = IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::LEFT;
    } else if (posValue < StateMoving) {
      // TODO Add 10s timeout
      pos = IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::MOVING;
    } else if (posValue < StateRight) {
      pos = IPOCS::PointsStatusPacket::E_RQ_POINTS_STATE::RIGHT;
    }
  }
  return pos;
}

static PointsMotor* createPointsMotor_Servo()
{
  return new PointsMotor_Servo();
}

__attribute__((constructor))
static void initialize_pointsmotor_servo() {
  PointsMotorStore::getInstance().registerType(2, &createPointsMotor_Servo);
}