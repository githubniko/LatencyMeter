#define PIN_IN 7
#define PIN_OUT 3

#include "include/AbstractEventHandler.h"

class LatencyMeter
{
    uint32_t _timer = 0;         // Переменная таймера
    bool _flagMeasuring = false; // когда true, то идет процесс измерения
    bool _flagStatus = false;    // Управляет запуском/остановкой процесса измерения

public:
    float startVoltage = 0;
    float smaTime = 0; // Среднее время
    float minTime = 0;
    float maxTime = 0;
    float valueTime = 0;
    uint8_t count = 0; // Кол-во измерений

    TEvent<> onUpdate;

public:
    LatencyMeter()
    {
        analogReference(EXTERNAL); // внешнее опорное напряжение 3.3В
        pinMode(PIN_OUT, OUTPUT);
    }

    void Start()
    {
        startVoltage = 0;
        smaTime = 0;
        minTime = 0;
        maxTime = 0;
        valueTime = 0;
        _flagMeasuring = false;

        onUpdate();
        digitalWrite(PIN_OUT, LOW);
        delay(1000);
        startVoltage = getVoltage();// / 2;
        //delay(2000);
        _flagStatus = true;
        
        digitalWrite(PIN_OUT, LOW); // Выкл. светодиод
    }
    void Stop() { _flagStatus = false; }
    void Restart()
    {
        Stop();
        Start();
    }

    /// @brief Выполняет измерение
    void Execute()
    {
        if (!_flagStatus)
            return;

        if (!_flagMeasuring)
        {
            // Ждем, пока датчик зафиксирует состояние выкл. светодиода
            if (getVoltage() > startVoltage)
                return;

            _flagMeasuring = true;
            _timer = millis();
            digitalWrite(PIN_OUT, HIGH); // Зажигаем светодиод
        }
        // Ждем, сигнал от фотодатчика
        else
        {
            float voltage = getVoltage();
            if (voltage > startVoltage + 0.5f)
            {                                         // Если сигнал поступил, то
                count++;
                valueTime = float(millis() - _timer); // Считаем задержку

                if (valueTime < minTime || minTime == 0)
                    minTime = valueTime;

                if (valueTime > maxTime)
                    maxTime = valueTime;

                smaTime = smaTime > 0 ? (smaTime + valueTime) / 2 : valueTime; // Расчет средней
                
                onUpdate();

                digitalWrite(PIN_OUT, LOW); // Выкл. светодиод
                _flagMeasuring = false;
            }
        }
    }

private:
    /// @brief Получает напряжение на аналоговов входе
    /// @return Возвращяет напряжение
    float getVoltage()
    {
        return (float)(analogRead(PIN_IN) * 3.3) / 1024;
    }
};