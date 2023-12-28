#define PIN_IN 7
#define PIN_OUT 3

#include "include/AbstractEventHandler.h"

class LatencyMeter
{
    uint32_t _timer = 0;         // Переменная таймера
    bool _flagMeasuring = false; // когда true, то идет процесс измерения
    bool _flagStatus = false;    // Управляет запуском/остановкой процесса измерения
    List<uint16_t> _listValue;    // массив измерений

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
                valueTime = millis() - _timer; // Считаем задержку
                AddValue(valueTime);

                if (valueTime < minTime || minTime == 0)
                    minTime = valueTime;

                if (valueTime > maxTime)
                    maxTime = valueTime;

                medianTime = round(Median());

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
    void AddValue(uint32_t value)
    {
        int size = _listValue.getSize();
        if (size > 50)
        { // ограничиваем размре, чтобы небыло переполнения памяти
            _listValue.removeFirst();
        }
        _listValue.add(value);
        Serial.println(value);
    }

    /// @brief Возвращает медиану. Примечание: данные должны быть отсортированы
    /// @return Значение медианы
    float Median()
    {
        uint16_t size = _listValue.getSize();
        
        int8_t napravlenie = 0; // задает направление поиска 2ой медианы (для четного ряда). Может принимать -1 или 1
        float median = 0;       // хранит значение первой найденой медианы (для четного ряда)
        float predel = 0;       // хранит верхнюю/нижнюю границу поиска медианы (для четного ряда)
        for (uint16_t i = 0; i < size; i++)
        {
            if (napravlenie != 0)
            { // если первая медиана найдена, то пропускаем значения
                if (napravlenie < 0 && _listValue[i] <= median) // меньше медианы
                    continue;
                if (napravlenie > 0 && _listValue[i] >= median) // больше медианы
                    continue;

                // Проускаем значения, которые выходят за пределы диаппазона
                if (predel != 0)
                {
                    if (napravlenie < 0 && predel < _listValue[i])
                        continue;
                    if (napravlenie > 0 && predel > _listValue[i])
                        continue;
                }
            }
            // цикл поиска медианы
            int m = 0, c = 0; // счетчики бОльших и меньших значений
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
            m = (abs(m) - c) * (m > 0 ? 1 : -1); // это нужно, чтобы определить в какую сторону отнести равные значения ряда

            // для НЕЧЕТНОГО ряда медиана будет одна при m == 0
            if (m == 0)
                return _listValue[i];

            // для ЧЕТНОГО ряда ищем два ближайших кандидата для медианы
            if (abs(m) == 1)
            {
                if (napravlenie == 0)
                { // записываем первую медиан
                    median = _listValue[i];
                    napravlenie = m; // если m = 1, то это верхняя медиана, если m = -1, то это нижняя медиана
                    //  Это помогает отсечь значение вне диапазона для поиска
                    // 2ой медианы (экономит время выполнения программы)
                }
                else
                { // вычисляем среднее значение из 2х соседних медиан
                    return (median + _listValue[i]) / 2.0f;
                }
            }
            // для ЧЕТНОГО ряда: нужно отбрасывать значения, которые не входят в интервал поиска
            else if (napravlenie != 0)
            {
                // Сужаем диаппазон поиска
                //
                if (predel == 0) // для первого значения
                    predel = _listValue[i];

                // Все последующие сравниваются с предыдущим, и если оно сужает поиск, то значение обновляется
                else if (napravlenie < 0 && _listValue[i] < predel)
                    predel = _listValue[i];
                else if (napravlenie > 0 && _listValue[i] > predel)
                    predel = _listValue[i];

            }
        }
        return median;
    }
};