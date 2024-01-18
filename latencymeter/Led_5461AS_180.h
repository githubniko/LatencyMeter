#pragma once

#define A PORTD5    // D5
#define B PORTB1    // D9
#define C PORTC2    // A2
#define D PORTC4    // A4
#define E PORTC5    // A5
#define F PORTD6    // D6
#define G PORTC1    // A1
#define DP PORTC3   // A3
#define DIG1 PORTD4 // D4
#define DIG2 PORTD7 // D7
#define DIG3 PORTB0 // D8
#define DIG4 PORTC0 // A0

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
        DDRB = DDRB | B00011;
        DDRC = DDRC | B00111111;
        DDRD = DDRD | B11110000;

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
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C);
            break;
        case '2':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << D) | (1 << E) | (1 << G);
            PORTD = PORTD | (1 << A);
            break;
        case '3':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << G);
            PORTD = PORTD | (1 << A);
            break;
        case '4':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C) | (1 << G);
            PORTD = PORTD | (1 << F);
            break;
        case '5':
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << G);
            PORTD = PORTD | (1 << A) | (1 << F);
            break;
        case '6':
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << E) | (1 << G);
            PORTD = PORTD | (1 << A) | (1 << F);
            break;
        case '7':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C);
            PORTD = PORTD | (1 << A);
            break;
        case '8':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << E) | (1 << G);
            PORTD = PORTD | (1 << A) | (1 << F);
            break;
        case '9':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << G);
            PORTD = PORTD | (1 << A) | (1 << F);
            break;
        case '0':
            PORTB = PORTB | (1 << B);
            PORTD = PORTD | (1 << A) | (1 << F);
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << E);
            break;
        case 'H':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C) | (1 << E) | (1 << G);
            PORTD = PORTD | (1 << F);
            break;
        case 'h':
            PORTC = PORTC | (1 << C) | (1 << E) | (1 << G);
            PORTD = PORTD | (1 << F);
            break;
        case 'L':
            PORTC = PORTC | (1 << D) | (1 << E);
            PORTD = PORTD | (1 << F);
            break;
        case 'l':
            PORTC = PORTC | (1 << E);
            PORTD = PORTD | (1 << F);
            break;
        case ' ':
            PORTB = PORTB & ~(1 << B);
            PORTC = PORTC & ~(1 << C) & ~(1 << D) & ~(1 << E) & ~(1 << G) & ~(1 << DP);
            PORTD = PORTD & ~(1 << A) & ~(1 << F);
            break;
        case '-':
            PORTC = PORTC | (1 << G);
            break;
        case 'U':
        case 'u':
            PORTB = PORTB | (1 << B);
            PORTC = PORTC | (1 << C) | (1 << D) | (1 << E);
            PORTD = PORTD | (1 << F);
            break;
        case '^':
            PORTD = PORTD | (1 << A) | (1 << F);
            break;
        case 'n':
            PORTC = PORTC | (1 << C) | (1 << E) | (1 << G);
            break;
        default:
            break;
        }

        if (dp)
            PORTC = PORTC | (1 << DP);
    }
    /// @brief Сбрасывает значения цифры
    static void DigitOff()
    {
        PORTB = PORTB & ~(1 << B);
        PORTC = PORTC & ~(1 << C) & ~(1 << D) & ~(1 << E) & ~(1 << G) & ~(1 << DP);
        PORTD = PORTD & ~(1 << A) & ~(1 << F);
    }
    /// @brief Выключает все разряды
    static void TurnOff()
    {
        PORTB = PORTB | (1 << DIG3);
        PORTC = PORTC | (1 << DIG4);
        PORTD = PORTD | (1 << DIG1) | (1 << DIG2);
    }
    /// @brief Включает разряд c порядковым номером.Слева направо
    static void TurnOnNumber(uint8_t num)
    {
        switch (num)
        {
        case 1:
            PORTD = PORTD & ~(1 << DIG1);
            break;
        case 2:
            PORTD = PORTD & ~(1 << DIG2);
            break;
        case 3:
            PORTB = PORTB & ~(1 << DIG3);
            break;
        case 4:
            PORTC = PORTC & ~(1 << DIG4);
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
