#define F_CPU 16000000UL // частота работы микроконтроллера

#define PIN_BUTTON 10

#include "Led_5461AS.h"
#include "Button.h"
#include "LatencyMeter.h"

Led_5461AS *led;
LatencyMeter *latencyMeter;
Button button1(PIN_BUTTON);

class EventHandler
{
public:
  void OnUpdate(float value)
  {
    led->Set(value);
  }
  void OnBtnClick()
  {
      Serial.println("OnBtnClick");
  }
  void OnBtnReset()
  {
    Serial.println("onClickLong");
    latencyMeter->Start();
    //asm volatile("jmp 0x00"); // Перезагрузка
    //led->Set("L");
  }
};
EventHandler eventHandler;

void setup()
{
  Serial.begin(115200);

  led = new Led_5461AS();
  latencyMeter = new LatencyMeter();
  latencyMeter->onUpdate += METHOD_HANDLER(eventHandler, EventHandler::OnUpdate);

  button1.onClick     += METHOD_HANDLER(eventHandler, EventHandler::OnBtnClick);
  button1.onClickLong += METHOD_HANDLER(eventHandler, EventHandler::OnBtnReset);

  latencyMeter->Start();
}

void loop()
{
  button1.Execute();
  latencyMeter->Execute();
}
