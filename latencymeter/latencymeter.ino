#define F_CPU 16000000UL // частота работы микроконтроллера

#define PIN_BUTTON 10

#include "led.h"
#include "button.h"
#include "latencymeter.h"

Led_5461AS *led;
LatencyMeter *latencyMeter;
Button btnReset(PIN_BUTTON);

class EventHandler
{
public:
  void OnUpdate(float value)
  {
    led->Set(value);
  }
};
EventHandler eventHandler;

void setup()
{
  Serial.begin(115200);

  led = new Led_5461AS();
  latencyMeter = new LatencyMeter();
  latencyMeter->onUpdate += METHOD_HANDLER(eventHandler, EventHandler::OnUpdate);

  led->Set(latencyMeter->startVoltage);

  // led->Set("...1.2");
  // led->Set(2.70f);

  delay(2000);
}

void loop()
{

  // Перезагрузка по клику
  if (btnReset.Click())
  {
    // asm volatile("jmp 0x00");
    latencyMeter->Stop();
  }

  latencyMeter->Execute();
}
