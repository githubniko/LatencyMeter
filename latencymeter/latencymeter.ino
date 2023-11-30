#define F_CPU 16000000UL //частота работы микроконтроллера



#include "led.h"


Led_5461AS *led;


void setup() {
  Serial.begin(115200);
  
  led = new Led_5461AS();
  
  
  
  //Тестирование индикации
  for (uint32_t i = 0; i < 10000000; i+=100)
  {
    float t = i/1000.0;
    led->Set(t);
    Serial.println(t);
    delay(10);
  }
  //led->Set(121.1);
}

void loop() {
  
  
}

