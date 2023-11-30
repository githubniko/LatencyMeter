#define F_CPU 16000000UL //частота работы микроконтроллера
#define PIN_IN 7
#define PIN_OUT 3
#define PIN_BUTTON 10


#include "led.h"
#include "button.h"

int counter = 0;
uint32_t timer = 0; // Переменная таймера
bool flagStart = false; // когда true, то идет процесс измерения
float startVoltage = 0;

Led_5461AS *led;
Button btnReset(PIN_BUTTON);

void setup() {
  //Serial.begin(115200);\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
  led = new Led_5461AS();

  analogReference(EXTERNAL); // внешнее опорное напряжение 3.3В

  pinMode(PIN_OUT, OUTPUT);
  digitalWrite(PIN_OUT, LOW);  

  delay(1000);
  startVoltage = getVoltage();
  led->Set(startVoltage);
  delay(2000);
}

void loop() {
  // Перезагрузка по клику
  if(btnReset.Click()) {
    asm volatile("jmp 0x00");
  }

  // Стартуем измерения
  if(!flagStart) {
    flagStart = true;
    timer = millis();
    digitalWrite(PIN_OUT, HIGH); // Зажигаем светодиод
  }
  // Ждем, сигнал от фотодатчика
  else {
    float voltage = getVoltage();
    if(voltage > startVoltage+0.5) { // Если сигнал поступил, то 
    float dTime = millis() - timer ; // Считаем задержку
    digitalWrite(PIN_OUT, LOW); // Выкл. светодиод
    
    // Ждем, пока датчик зафиксирует состояние
    do {
      voltage = getVoltage();
    } while(voltage > startVoltage);
    
    flagStart = false;
    led->Set(dTime/1000);
    }
  }
}

/// @brief Получает напряжение на аналоговов входе
/// @return Возвращяет напряжение
float getVoltage()
{
  return (float)(analogRead(PIN_IN) * 3.3) / 1024;
}

