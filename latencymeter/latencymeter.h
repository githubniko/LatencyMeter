#define PIN_IN 7
#define PIN_OUT 3

#include "include/AbstractEventHandler.h"

class LatencyMeter
{
    uint32_t timer = 0; // Переменная таймера
    bool flagStart = false; // когда true, то идет процесс измерения

    public:
    float startVoltage = 0;
    float smaTime = 0; // Среднее время
    float minTime = 0;
    float maxTime = 0;

    TEvent<float> onUpdate;

    public:
    LatencyMeter() {
        analogReference(EXTERNAL); // внешнее опорное напряжение 3.3В

        pinMode(PIN_OUT, OUTPUT);
        digitalWrite(PIN_OUT, HIGH);  

        delay(1000);
        startVoltage = getVoltage()/2;
    }

    void Start() {
        
    }
    void Stop() {}
    void Restart() {}
    
    /// @brief Выполняет измерение
    void Execute() {

        if(!flagStart) {
            flagStart = true;
            timer = millis();
            digitalWrite(PIN_OUT, HIGH); // Зажигаем светодиод
        }
        // Ждем, сигнал от фотодатчика
        else {
            float voltage = getVoltage();
            if(voltage > startVoltage + 0.5f) { // Если сигнал поступил, то 
                float dTime = float(millis() - timer); // Считаем задержку
                if(dTime < minTime || minTime == 0) minTime = dTime;
                if(dTime > maxTime) maxTime = dTime;
                smaTime = (smaTime + dTime) / 2; // Расчет средней
                
                onUpdate(smaTime);

                digitalWrite(PIN_OUT, LOW); // Выкл. светодиод
                // Ждем, пока датчик зафиксирует состояние
                do {
                    voltage = getVoltage();
                } while(voltage > startVoltage);
            } 
            flagStart = false;
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