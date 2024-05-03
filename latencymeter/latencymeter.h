#define PIN_IN 7     // фотодатчик
#define PIN_OUT 2    // светодиод 1
#define PIN_OUT2 3   // светодиод 2
#define INTERVAL 500 // интервал переключения светодиода, мс
#define VOLTAGE_REF 3.33f

#include "include/AbstractEventHandler.h"

class LatencyMeter
{
    uint32_t _timer = 0, _timer2 = 0, _timer3 = 0, _timer4 = 0; // Переменная таймера
    bool _flagMeasuring = false;                                // когда true, то идет процесс измерения
    bool _flagStatus = false;                                   // Управляет запуском/остановкой процесса измерения
    List<uint16_t> _listValue;                                  // массив измерений
    byte _pinOut = PIN_OUT,                                     // красный
        _pinOut2 = PIN_OUT2;                                    // синий
    bool _sw = 1;                                               // флаг переключения светодиода
    uint32_t _interval = INTERVAL;                              // интервал переключения светодиода
    float _spread = 0;                                          // разброс измерений

public:
    float lowVoltage = 0;
    float highVoltage = VOLTAGE_REF;
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
        pinMode(_pinOut, OUTPUT);
        pinMode(_pinOut2, OUTPUT);
    }

    void Start()
    {
        Serial.println("Start");
        lowVoltage = 0;
        highVoltage = VOLTAGE_REF;
        medianTime = 0;
        minTime = 0;
        maxTime = 0;
        valueTime = 0;
        count = 0;
        _listValue.clear();

        // Проверка, какой из цветов дает высокий, а какой - низкий уровни
        float low = 32767, high = 0;
        ledSwitch(_sw);
        delay(2000);

        ledSwitch(!_sw);
        delay(500);
        high = getVoltage();

        ledSwitch(_sw);
        delay(500);
        low = getVoltage();

        if (low > high)
        { // меняем местами цвета
            _sw = !_sw;
        }

        // Вычисляем разброс измерений
        low = 32767, high = 0;
        for (int i = 0; i < 500; i++)
        {
            float voltage = getVoltage();
            if (voltage < low)
                low = voltage;
            if (voltage > high)
                high = voltage;
            // Serial.print(voltage);
            // Serial.print(",");
            // Serial.println("");
            delay(1);
        }
        _spread = high - low;

        onUpdate();
        minTime = 32767;

        _flagStatus = true;
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
        if (!_flagStatus) // Ждем Старт
            return;

        // Блок переключает светодиод с периодичностью _interval
        // Тут нужно подобрать интервал переключения так, чтобы автоэкспозиция не менялась
        uint32_t ms = micros() - _timer2;
        if (ms >= _interval * 1000 * 2)
        {
            _sw = !_sw;
            ledSwitch(_sw);
            _timer2 = micros();
            if (_sw)
                _timer = micros(); // сохраняем время начала измерения
        }

        // Блок производящий измерения на фотодатчике
        if (_sw) // Ждем, начало измерения
        {
            // Интервал изменений на фотодатчике
            ms = (micros() - _timer3) / 1000;
            if (ms > _interval * 2)
            {
                _flagMeasuring = true;
                lowVoltage = 0;
                highVoltage = 3.3f;
                _timer3 = micros(); // Обнуляем таймер каждый _interval миллисекунд
            }
            else
            {
                float voltage = getVoltage(); // сохраняем зн. напряжения каждую миллисекунду

                // Авто-обновляем экстремумов
                ms = (micros() - _timer4) / 1000;
                if (ms > _interval * 2)
                {
                    if (lowVoltage > voltage || lowVoltage == 0)
                        lowVoltage = voltage;

                    if (highVoltage < voltage || highVoltage == VOLTAGE_REF)
                        highVoltage = voltage;
                    _timer4 = micros();
                }

                float delta = (highVoltage - lowVoltage);

                // Защита от низкой разницы между верх/нижн измерениями
                if (delta < 0.5f)
                {
                    valueTime = 0;
                    return;
                }

                // if (ms % 1000 == 0)
                // {
                //     Serial.print("Low: ");
                //     Serial.print(lowVoltage);
                //     Serial.print(" High: ");
                //     Serial.print(highVoltage);
                //     Serial.print(" _spread: ");
                //     Serial.print(_spread);
                //     Serial.print(" delta: ");
                //     Serial.println(delta);
                // }

                if (voltage > lowVoltage + _spread + 0.2f && _flagMeasuring)
                {
                    _flagMeasuring = false;
                    valueTime = (micros() - _timer) / 1000;
                    AddValue(valueTime);
                    if (valueTime < minTime)
                        minTime = valueTime;

                    if (valueTime > maxTime) {
                        maxTime = valueTime;
                        _interval = maxTime*2; // сокращаем интервал проверки
                        }
                    //_timer3 = _timer3 + (valueTime - maxTime)*1000; // синхронизации кадров и вспышки
                    medianTime = round(median());
                    onUpdate();
                    Serial.println(valueTime);
                }
            }

            /*if (voltage > highVoltage)
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
                _flagMeasuring = false;
            }*/
        }
    }

private:
    /// @brief Получает напряжение на аналоговов входе
    /// @return Возвращяет напряжение
    float getVoltage()
    {
        return (float)(analogRead(PIN_IN) * 3.3) / 1024;
    }
    /// @brief Переключает светодиод
    /// @param in
    void ledSwitch(bool in)
    {
        digitalWrite(_pinOut2, !in);
        digitalWrite(_pinOut, in);
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
        // Serial.println(value);
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
            else if (m > 0)
            {
                if (limHigh == 0 || limHigh > _listValue[i])
                    limHigh = _listValue[i];
            }
        }
        return median1;
    }
};
