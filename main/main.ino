#include <PIO_DShot.h>

#define PIN 10

BidirDShotX1 *esc;

void initializeESC() {
  uint32_t start = millis();
  while (millis() - start < 500) {
    esc->sendThrottle(0);     // Подаём 0% газа для инициализации
    delayMicroseconds(200);   // DShot600 задержка
  }
}

void setup() {
  esc = new BidirDShotX1(PIN);  
  initializeESC();              
}

void loop() {
  delayMicroseconds(200);      // Периодическая задержка
  esc->sendThrottle(75);      // Подача 5% газа (100 из 2000)
}

