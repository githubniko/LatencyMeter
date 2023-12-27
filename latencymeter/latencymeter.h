#define PIN_IN 7
#define PIN_OUT 3

#include "include/AbstractEventHandler.h"

class LatencyMeter
{
    uint32_t _timer = 0;         // Переменная таймера
    bool _flagMeasuring = false; // когда true, то идет процесс измерения
    bool _flagStatus = false;    // Управляет запуском/остановкой процесса измерения
    List<float> _listValue;      // массив измерений

public:
    float startVoltage = 0;
    float medianTime = 0; // Медиана значений
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
        Serial.println("Start");
        startVoltage = 0;
        medianTime = 0;
        smaTime = 0;
        minTime = 0;
        maxTime = 0;
        valueTime = 0;
        count = 0;
        _flagMeasuring = false;
        _listValue.clear();

        onUpdate();
        digitalWrite(PIN_OUT, LOW);
        delay(1000);
        startVoltage = getVoltage() + 0.05f; // / 2;
        // delay(2000);
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
            { // Если сигнал поступил, то
                count++;
                valueTime = float(millis() - _timer); // Считаем задержку
                AddValue(valueTime);

                if (valueTime < minTime || minTime == 0)
                    minTime = valueTime;

                if (valueTime > maxTime)
                    maxTime = valueTime;

                smaTime = smaTime > 0 ? (smaTime + valueTime) / 2 : valueTime; // Расчет средней
                medianTime = Median();

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

    /// @brief Добавляет измерение в сортированный массив
    /// @return void
    void AddValue(float value)
    {
        int size = _listValue.getSize();
        bool flagAdd = false;

        // добавляем первый элемент
        if (size == 0)
        {
            _listValue.add(value);
        }
        else
        {
            // добавляем в середину
            for (int i = 0; i < _listValue.getSize(); i++)
            {
                if (value < _listValue[i])
                {
                    _listValue.addAtIndex(i, value);
                    flagAdd = true;
                    break;
                }
            }

            // добавляем в конец
            if (!flagAdd)
                _listValue.add(value);
        }
        Serial.println(value);
    }

    /// @brief Возвращает медиану. Примечание: данные должны быть отсортированы
    /// @return Значение медианы
    float Median()
    {
        const uint8_t size = _listValue.getSize();
        const float halfSize = size / 2;
        if (size % 2 == 0)
            return (_listValue[halfSize] + _listValue[halfSize - 1]) / 2;

        return _listValue[round(halfSize)];
    }
};