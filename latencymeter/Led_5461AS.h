#pragma once

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
    static inline uint8_t offset; // Смещение. Нужно для точки или запятой
    static inline char *value;    // Хранит текущее число индикации

public:
    static inline uint8_t Length; // Размер строки

public:
    Led_5461AS()
    {
        value = "0";
        count = 1;
        offset = 0;
        Length = strlen(value);

        // Устанавливаю порты на выход
        DDRD = DDRD | B11110000;
        DDRB = DDRB | B00011;
        DDRC = DDRC | B00111111;

        // Таймер обновления индикации
        cli(); // отключить глобальные прерывания
        TCCR1A = 0;
        TCCR1B = 0;
        TCCR1B |= (1 << CS11);   // делитель 8
        TCCR1B |= (1 << WGM12);  // сброс по совпадению
        OCR1A = 2500;            // 10ms, 100Гц*4 = 480Гц, 16 000 000 / 8 * 400Гц * 2 = 2500
        TIMSK1 |= (1 << OCIE1A); // прерывание по переполнению

        sei(); // включить глобальные прерывания
    }

    /// @brief Устанавливает значение индикатора
    void Set(float number)
    {
        if (number > 9999)
            number = 9999;

        uint8_t pos = FindDecimalPoint(number);
        char buf[33];
        dtostrf(number, 4, 4 - pos, buf);
        Set(buf);
    }

    /// @brief Устанавливает значение индикатора
    void Set(int number)
    {
        if (number > 9999)
            number = 9999;

        char buf[33];
        itoa(number, buf, 10);
        Set(buf);
    }

    /// @brief Устанавливает значение индикатора
    void Set(const char *str)
    {
        strcpy(value, str);
        Length = strlen(value);
    }

    /// @brief Метод обновления индикации
    static void Update()
    {
        bool dp = false;
        char symbol;
        TurnOff();

        int i = Length - 4 + count - 1;
        symbol = i < 0 ? ' ' : value[i - offset];

        if (symbol == '.' || symbol == ',')
        {
            dp = true;
            offset++;
            symbol = i - offset < 0 ? ' ' : value[i - offset];
        }

        Digit(symbol, dp);
        TurnOnNumber(count);

        count--;

        if (count == 0)
        {
            count = 4;
            offset = 0;
        }
    }

private:
    /// @brief Устанавливает число на индикаторе
    static void Digit(char val, bool dp = false)
    {
        DigitOff();

        switch (val)
        {
        case '1':
            PORTC = PORTC | (1 << B);
            PORTD = PORTD | (1 << C);
            break;
        case '2':
            PORTC = PORTC | (1 << A) | (1 << B);
            PORTD = PORTD | (1 << G);
            PORTB = PORTB | (1 << D) | (1 << E);
            break;
        case '3':
            PORTC = PORTC | (1 << A) | (1 << B);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << D);
            break;
        case '4':
            PORTC = PORTC | (1 << B) | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            break;
        case '5':
            PORTC = PORTC | (1 << A) | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << D);
            break;
        case '6':
            PORTC = PORTC | (1 << A) | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << D) | (1 << E);
            break;
        case '7':
            PORTC = PORTC | (1 << A) | (1 << B);
            PORTD = PORTD | (1 << C);
            break;
        case '8':
            PORTC = PORTC | (1 << A) | (1 << B) | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << D) | (1 << E);
            break;
        case '9':
            PORTC = PORTC | (1 << A) | (1 << B) | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << D);
            break;
        case '0':
            PORTC = PORTC | (1 << A) | (1 << B) | (1 << F);
            PORTD = PORTD | (1 << C);
            PORTB = PORTB | (1 << D) | (1 << E);
            break;
        case 'H':
            PORTC = PORTC | (1 << B) | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << E);
            break;
        case 'h':
            PORTC = PORTC | (1 << F);
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << E);
            break;
        case 'L':
            PORTC = PORTC | (1 << F);
            PORTB = PORTB | (1 << D) | (1 << E);
            break;
        case 'l':
            PORTC = PORTC | (1 << F);
            PORTB = PORTB | (1 << E);
            break;
        case ' ':
            PORTC = PORTC & ~(1 << A) & ~(1 << B) & ~(1 << F);
            PORTD = PORTD & ~(1 << C) & ~(1 << G) & ~(1 << DP);
            PORTB = PORTB & ~(1 << D) & ~(1 << E);
            break;
        case '-':
            PORTD = PORTD | (1 << G);
            break;
        case 'U':
        case 'u':
            PORTC = PORTC | (1 << B) | (1 << F);
            PORTD = PORTD | (1 << C);
            PORTB = PORTB | (1 << D) | (1 << E);
            break;
        case '^':
            PORTC = PORTC | (1 << A) | (1 << F);
            break;
        case 'n':
            PORTD = PORTD | (1 << C) | (1 << G);
            PORTB = PORTB | (1 << E);
            break;
        default:
            break;
        }

        if (dp)
            PORTD = PORTD | (1 << DP);
    }
    /// @brief Сбрасывает значения цифры
    static void DigitOff()
    {
        PORTC = PORTC & ~(1 << A) & ~(1 << B) & ~(1 << F);
        PORTD = PORTD & ~(1 << C) & ~(1 << G) & ~(1 << DP);
        PORTB = PORTB & ~(1 << D) & ~(1 << E);
    }
    /// @brief Выключает все разряды
    static void TurnOff()
    {
        PORTC = PORTC | (1 << DIG1) | (1 << DIG2) | (1 << DIG3);
        PORTD = PORTD | (1 << DIG4);
    }
    /// @brief Включает разряд c порядковым номером.Слева направо
    static void TurnOnNumber(uint8_t num)
    {
        switch (num)
        {
        case 1:
            PORTC = PORTC & ~(1 << DIG1);
            break;
        case 2:
            PORTC = PORTC & ~(1 << DIG2);
            break;
        case 3:
            PORTC = PORTC & ~(1 << DIG3);
            break;
        case 4:
            PORTD = PORTD & ~(1 << DIG4);
            break;

        default:

            break;
        }
    }

    /// @brief Возвращает позицию запятой в числе
    static uint8_t FindDecimalPoint(float number)
    {
        for (uint8_t i = 0; number != 0; ++i)
        {
            if ((uint8_t)number == 0)
            {
                if (i == 0)
                    i++;
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
