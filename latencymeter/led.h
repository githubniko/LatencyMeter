#define A PORTC1
#define B PORTC5
#define C PORTD6
#define D PORTB0
#define E PORTB1
#define F PORTC2
#define G PORTD5
#define DP PORTD7
#define DIG1 PORTC0
#define DIG2 PORTC3
#define DIG3 PORTC4
#define DIG4 PORTD4

#include <math.h>

class Led_5461AS
{    
    static inline uint8_t count;
    static inline float value; // Хранит текущее число индикации

    public:
        Led_5461AS()
        {
            value = 0;
            count = 1;

            // Устанавливаю порты на выход
            DDRD = DDRD | B11110000;
            DDRB = DDRB | B00111;
            DDRC = DDRC | B00111111;

            // Таймер обновления индикации
            cli(); // отключить глобальные прерывания
            TCCR1A = 0;
            TCCR1B = 0;
            TCCR1B |= (1 << CS11); // делитель 8
            TCCR1B |= (1 << WGM12); // сброс по совпадению
            OCR1A = 2500;         // 10ms, 100Гц*4 = 480Гц, 16 000 000 / 8 * 400Гц * 2 = 2500
            TIMSK1 |= (1 << OCIE1A); // прерывание по переполнению
           
            sei();  // включить глобальные прерывания
        }

        /// @brief Устанавливает значение индикатора
        void Set(float number)
        {
            value = number;
        }

        /// @brief Метод обновления индикации
        static void Update()
        {
            uint8_t digit = 0;
            TurnOff();
            // Защита от переполнения
            if(value > 9999)
                value = 9999;

            
            uint8_t lenDigit = 4; // кол-во сегментов в индиакторе
            uint8_t decimal_pos = FindDecimalPoint(value);
            uint16_t number = int(10000/pow(10, decimal_pos)) * value;
            
            bool dp = (decimal_pos == count && decimal_pos < lenDigit)   ? true : false;  
                  
            switch (count)
            {
            case 1:
                digit = number/1000 % 10;
                Digit(digit, dp);
                PORTC = PORTC &~ (1 << DIG1);
                break;
            case 2:
                digit = number/100 % 10;
                Digit(digit, dp);
                PORTC = PORTC &~ (1 << DIG2);
                break;
            case 3:
                digit = number/10 % 10;
                Digit(digit, dp);
                PORTC = PORTC &~ (1 << DIG3);
                break;
            case 4:
                digit = number % 10;
                Digit(digit, dp);
                PORTD = PORTD &~ (1 << DIG4);
                count = 0;
                break;
            
            default:
                break;
            }
            count++;
        }

    private:
        /// @brief Устанавливает число в порты микроконтроллера
        static void Digit(uint8_t val, bool dp = false)
        {
            DitigOff();

            switch (val)
            {
            case 1:
                PORTC = PORTC | (1 << B);
                PORTD = PORTD | (1 << C);
                break;
            case 2:
                PORTC = PORTC | (1 << A) | (1 << B);
                PORTD = PORTD | (1 << G);
                PORTB = PORTB | (1 << D) | (1 << E);
                break;
            case 3:
                PORTC = PORTC | (1 << A) | (1 << B);
                PORTD = PORTD | (1 << C) | (1 << G);
                PORTB = PORTB | (1 << D);
                break;
            case 4:
                PORTC = PORTC | (1 << B) | (1 << F);
                PORTD = PORTD | (1 << C) | (1 << G);
                break;
            case 5:
                PORTC = PORTC | (1 << A) | (1 << F);
                PORTD = PORTD | (1 << C) | (1 << G);
                PORTB = PORTB | (1 << D);
                break;
            case 6:
                PORTC = PORTC | (1 << A) | (1 << F);
                PORTD = PORTD | (1 << C) | (1 << G);
                PORTB = PORTB | (1 << D) | (1 << E);
                break;
            case 7:
                PORTC = PORTC | (1 << A) | (1 << B);
                PORTD = PORTD | (1 << C);
                break;
            case 8:
                PORTC = PORTC | (1 << A) | (1 << B) | (1 << F);
                PORTD = PORTD | (1 << C) | (1 << G);
                PORTB = PORTB | (1 << D) | (1 << E);
                break;
            case 9:
                PORTC = PORTC | (1 << A) | (1 << B) | (1 << F);
                PORTD = PORTD | (1 << C) | (1 << G);
                PORTB = PORTB | (1 << D);
                break;
            case 0:
                PORTC = PORTC | (1 << A) | (1 << B) | (1 << F);
                PORTD = PORTD | (1 << C);
                PORTB = PORTB | (1 << D) | (1 << E);
                break;
            default:
                break;
            }

            if(dp)
                PORTD = PORTD | (1 << DP);
        }

        /// @brief Сбрасывает значения цифры
        static void DitigOff()
        {
            PORTC = PORTC &~ (1 << A) &~ (1 << B) &~ (1 << F);
            PORTD = PORTD &~ (1 << C) &~ (1 << G) &~ (1 << DP);
            PORTB = PORTB &~ (1 << D) &~ (1 << E);
        }
        /// @brief Выключает все разряды
        static void TurnOff()
        {
            PORTC = PORTC | (1 << DIG1) | (1 << DIG2) | (1 << DIG3);
            PORTD = PORTD | (1 << DIG4);
        }

        /// @brief Возвращает позицию запятой в числе
        static uint8_t FindDecimalPoint(float number)
        {
            for(uint8_t i=0; number != 0; ++i)    {
                if((uint8_t)number == 0) {
                    if(i == 0) i++;
                    return i;
                }
                number /= 10;
            }

            return 1;
        }
};


// Обработчик прерываний
ISR(TIMER1_COMPA_vect)
{
    Led_5461AS::Update();
}
