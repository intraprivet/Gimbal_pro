#include <PIO_DShot.h>

#define PIN 12

BidirDShotX1 *esc;

void initializeESC() {
  // Эмулируем два "провала" сигнала на пине, как будто пин дергается в LOW
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);
  delay(500);  // Краткий LOW

  // Теперь запускаем регулярную подачу DShot-пакетов с нулевым газом
  esc = new BidirDShotX1(PIN);
  for (int i = 0; i < 5000; i++) {
    esc->sendThrottle(0);  // Подаём 0% газа
    delayMicroseconds(200);
  }
}

void setup() {
  initializeESC();
}

void loop() {
  delayMicroseconds(200);
  esc->sendThrottle(100);  // Подача 5% газа (100 из 2000)
}
