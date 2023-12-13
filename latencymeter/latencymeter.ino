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
float smaTime = 0; // Среднее время
float minTime = 0;
float maxTime = 0;

Led_5461AS *led;
Button btnReset(PIN_BUTTON);

void setup() {
  Serial.begin(115200);
  
  led = new Led_5461AS();

  analogReference(EXTERNAL); // внешнее опорное напряжение 3.3В

  pinMode(PIN_OUT, OUTPUT);
  digitalWrite(PIN_OUT, HIGH);  

  delay(1000);
  startVoltage = getVoltage()/2;
  led->Set(startVoltage);
  
  //led->Set("...1.2");
  //led->Set(2.70f);
  
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
    if(voltage > startVoltage + 0.5f) { // Если сигнал поступил, то 
      float dTime = float(millis() - timer); // Считаем задержку
      if(dTime < minTime || minTime == 0) minTime = dTime;
      if(dTime > maxTime) maxTime = dTime;
      smaTime = (smaTime + dTime) / 2; // Расчет средней
      led->Set((int)smaTime);

      digitalWrite(PIN_OUT, LOW); // Выкл. светодиод
      // Ждем, пока датчик зафиксирует состояние
      do {
        voltage = getVoltage();
      } while(voltage > startVoltage);
      
      flagStart = false;
    }
  }
}

/// @brief Получает напряжение на аналоговов входе
/// @return Возвращяет напряжение
float getVoltage()
{
  return (float)(analogRead(PIN_IN) * 3.3) / 1024;
}
