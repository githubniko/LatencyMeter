#define PIN_IN 7
#define PIN_OUT 3
#define PIN_OUTT 2

#include "include/AbstractEventHandler.h"

class LatencyMeter
{
    uint32_t _timer = 0;         // Переменная таймера
    bool _flagMeasuring = false; // когда true, то идет процесс измерения
    bool _flagStatus = false;    // Управляет запуском/остановкой процесса измерения
    List<uint16_t> _listValue;   // массив измерений
    byte _pinOut = PIN_OUT, _pinOutt = PIN_OUTT;

public:
    float startVoltage = 0;
    uint16_t medianTime = 0; // Медиана значений
    uint16_t minTime = 0;
    uint16_t maxTime = 0;
    uint16_t valueTime = 0;
    uint16_t count = 0; // Кол-во измерений

    TEvent<> onUpdate;

public:
    LatencyMeter()
    {
        analogReference(EXTERNAL); // внешнее опорное напряжение 3.3В
        pinMode(PIN_OUT, OUTPUT);
        pinMode(PIN_OUTT, OUTPUT);
    }

    void Start()
    {
        Serial.println("Start");
        startVoltage = 0;
        medianTime = 0;
        minTime = 0;
        maxTime = 0;
        valueTime = 0;
        count = 0;
        _flagMeasuring = false;
        _listValue.clear();

        onUpdate();
        minTime = 32767;
        ledSwitch(0);
        delay(1000);
        startVoltage = getVoltage() + 0.05f; // / 2;
        // delay(2000);
        _flagStatus = true;

        ledSwitch(0); // Выкл. светодиод
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
            _timer = micros();
            ledSwitch(1); // Зажигаем светодиод
        }
        // Ждем, сигнал от фотодатчика
        else
        {
            float voltage = getVoltage();
            if (voltage > startVoltage + 0.5f)
            { // Если сигнал поступил, то
                count++;
                valueTime = (micros() - _timer) / 1000; // Считаем задержку
                AddValue(valueTime);

                if (valueTime < minTime)
                    minTime = valueTime;

                if (valueTime > maxTime)
                    maxTime = valueTime;

                medianTime = round(median());

                onUpdate();

                ledSwitch(0); // Выкл. светодиод
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

    /// @brief Добавляет измерение в сортированный массив
    /// @return void
    void AddValue(uint32_t value)
    {
        int size = _listValue.getSize();
        if (size > 50)
        { // ограничиваем размeр, чтобы не было переполнения памяти
            _listValue.removeFirst();
        }
        _listValue.add(value);
        Serial.println(value);
    }

    /// @brief Возвращает медиану. Примечание: данные должны быть отсортированы
    /// @return Значение медианы
    float median()
    {
        uint16_t size = _listValue.getSize();

        float median1 = 0;           // хранит значение первой найденой медианы (для четного ряда)
        int limLow = 0, limHigh = 0; // верхний нижний предел
        for (uint16_t i = 0; i < size; i++)
        {
            // Пропускамем, если число вышло за установленные пределы поиска
            if (limLow != 0 && limLow >= _listValue[i])
                continue;
            if (limHigh != 0 && limHigh <= _listValue[i])
                continue;
            // цикл поиска медианы
            int m = 0, // счетчики бОльших и меньших значений
                c = 0; // счетчик повторяющихся значений
            for (uint16_t j = 0; j < size; j++)
            {
                if (i != j) // пропускаем самого себя
                {
                    if (_listValue[i] < _listValue[j])
                        m--;
                    else if (_listValue[i] > _listValue[j])
                        m++;
                    else
                        c++;
                }
            }
            m = c > abs(m) ? 0 : (abs(m) - c) * (m > 0 ? 1 : -1); // это нужно, чтобы определить в какую сторону отнести равные значения ряда

            // для НЕЧЕТНОГО ряда медиана будет одна при m == 0
            if (m == 0)
                return _listValue[i];

            // для ЧЕТНОГО ряда ищем два ближайших кандидата для медианы
            if (abs(m) == 1)
            {
                if (median1 != 0 && (limLow == median1 || limHigh == median1))
                    return (median1 + _listValue[i]) / 2.0f; // вычисляем среднее значение из 2х соседних медиан
                else
                    median1 = _listValue[i]; // первая медиана
            }
            // Выставляем верхний и нижний предел ряда
            if (m < 0)
            {
                if (limLow == 0 || limLow < _listValue[i])
                    limLow = _listValue[i];
            }
            else if(m > 0)
            {
                if (limHigh == 0 || limHigh > _listValue[i])
                    limHigh = _listValue[i];
            }
        }
        return median1;
    }
    /// @brief Переключает светодиод
    /// @param in
    void ledSwitch(bool in)
    {
        digitalWrite(_pinOutt, !in);
        digitalWrite(_pinOut, in);
    }
};
