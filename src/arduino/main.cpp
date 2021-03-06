#include "../controller/EspConnection.h"
#include "../controller/ObjectStore.h"
#include "../IPC/Message.h"
#include "../IPOCS/Message.h"
#include "../LedControl.h"
#include <avr/wdt.h>

#ifdef HAVE_HWSERIAL1
#define PRINT_TIME 50
unsigned long lastPrint = 0;
#endif

extern const uint8_t LED_PIN = LED_BUILTIN;

void setup(void) {
  MCUSR = ~(1 << WDRF); // allow us to disable WD
  wdt_disable();
  #ifdef HAVE_HWSERIAL1
  Serial1.begin(115200);
  Serial1.print('r');
  Serial1.flush();
  lastPrint = millis();
  #endif
  ard::EspConnection::instance().begin();
  LedControl::instance().begin();
  LedControl::instance().setState(LedState::BLINK, 500);
}

void loop() {
  LedControl::instance().loop();
  ard::EspConnection::instance().loop();
  ObjectStore::getInstance().loop();
#ifdef HAVE_HWSERIAL1
  // Make sure we're not working too fast for Arduino
  unsigned long now = millis();
  if ((now - lastPrint) >= PRINT_TIME) {
    lastPrint = now;
    Serial1.print('.');
    Serial1.flush();
  }
#endif
}
