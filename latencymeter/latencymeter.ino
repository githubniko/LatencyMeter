#define F_CPU 16000000UL // частота работы микроконтроллера

#define PIN_BUTTON 10

#include "Led_5461AS.h"
#include "Button.h"
#include "LatencyMeter.h"

Led_5461AS *led;
LatencyMeter *latencyMeter;
Button button1(PIN_BUTTON);

uint8_t flagTypeDataOut = 0; // 0 - задержка рела-тайм, 1 - минимальная, 2 - максимальная, 3 - средняя, 4 - кол-во измерений
uint8_t icount = 0;

class EventHandler
{
public:
  void OnUpdateData()
  {
    char buf[33];
    char str[10];

    switch (flagTypeDataOut)
    {
    case 1:
      dtostrf(latencyMeter->minTime, 3, 0, buf);
      strcpy(str, "L");
      break;
    case 2:
      dtostrf(latencyMeter->maxTime, 3, 0, buf);
      strcpy(str, "H");
      break;
    case 3:
      dtostrf(latencyMeter->smaTime, 3, 0, buf);
      strcpy(str, "^");
      break;
    case 4:
      itoa(icount++, buf, 10);
      strcpy(str, "n");
      break;
    case 0:
    default:
      dtostrf(latencyMeter->valueTime, 4, 0, buf);
      strcpy(str, "");
      break;
    }
    strcat(str, buf);
    Serial.println(latencyMeter->valueTime);
    led->Set(str); // вывод на экран
  }
  void OnBtnClick()
  {
    flagTypeDataOut++;
    if (flagTypeDataOut > 4)
      flagTypeDataOut = 0;
    
    OnUpdateData();
  }
  void OnBtnReset()
  {
    icount=0;
    latencyMeter->Start();
    // asm volatile("jmp 0x00"); // Перезагрузка
  }
};
EventHandler eventHandler;

void setup()
{
  Serial.begin(115200);

  led = new Led_5461AS();
  latencyMeter = new LatencyMeter();
  latencyMeter->onUpdate += METHOD_HANDLER(eventHandler, EventHandler::OnUpdateData);

  button1.onClick += METHOD_HANDLER(eventHandler, EventHandler::OnBtnClick);
  button1.onClickLong += METHOD_HANDLER(eventHandler, EventHandler::OnBtnReset);

  latencyMeter->Start();
}

void loop()
{
  button1.Execute();
  latencyMeter->Execute();
}
