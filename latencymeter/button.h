#include "include/AbstractEventHandler.h"

class Button
{

public:
  TEvent<> onClick;
  TEvent<> onClickLong;
  TEvent<> onClickLongDown;
  TEvent<> onClickLongPulse;
  TEvent<> onKeyDown;
  TEvent<> onKeyUp;

public:
  Button(byte pin)
  {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
  }
  /// @brief Основной цикл класса
  void Execute()
  {
    bool btnState = !digitalRead(_pin);

    

    if (!btnState && _flag) // кнопка отпущена
    {
      uint32_t time = millis() - _timer;

      if (time >= 500) // Отпускание после долгого нажания
      {
        onKeyUp();
        onClickLongDown();
        _flag = false; 
        _flagEvent = false; 
        return;
      }
      if (time >= 30 && !_flagEvent) // Одинарно нажатие
      {
        onKeyUp();
        onClick();
        _flag = false;
        _flagEvent = false;
        return;
      }
    }

    if(btnState && _flag) // Удерживание кнопки
    {
      uint32_t time = millis() - _timer;
      if (time >= 500)
      {
        if(_flagEvent)
          onClickLongPulse();
        else 
          onClickLong();

        _flagEvent = true;
        _timer = millis();       
        return;
      }
    }

    if (btnState && !_flag && !_flagEvent) // Нажатие кнопки
    {
      onKeyDown();
      _flag = true;
      _timer = millis();
    }
  }

private:
  byte _pin;
  uint32_t _timer = 0;
  bool _flag = false; // флаг кнопка нажата
  bool _flagEvent = false; // флаг Произошло событие, ждать исходное состояние кнопки
};