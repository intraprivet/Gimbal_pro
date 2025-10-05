#include <PIO_DShot.h>

#define PIN 12

BidirDShotX1 *esc;
bool escInitialized = false;  // Флаг инициализации

void SendThrottle(uint16_t value) {
  if (escInitialized && esc) {
    esc->sendThrottle(value);
  }
}

void initializeESC() {
  delay(1000);
  digitalWrite(PIN, LOW);
  delay(500);

  esc = new BidirDShotX1(PIN);
  
  delayMicroseconds(200);
  // арминг нулём ~600 мс
  for (int i = 0; i < 3000; i++) {
    esc->sendThrottle(0);
    delayMicroseconds(200);
  }

  escInitialized = true;
}


void setup() {
  initializeESC();
}

void loop() {
  delayMicroseconds(200);
  SendThrottle(52);  // Подача 5% газа
}
