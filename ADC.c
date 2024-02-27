/************************************************************************************
* Gonzalo Alberto Guajardo Galindo 
* Device: Atmega328P 
* Rev: 1 
* Date: 11/13/22 
* Description: Controls a 7-segment display using an Atmega328P. 
* Reads an analog value from an ADC and displays different patterns on the display.
************************************************************************************/

// Libraries
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h> 

// Constants
#define F_CPU 1600000UL 

// Macros
#define DISPLAY PORTB
#define SCANNING_PORT PORTC
#define THOUSANDS_PIN PINC0 // Thousands
#define THOUSANDS_PIN_ON SCANNING_PORT &=~_BV(THOUSANDS_PIN)
#define THOUSANDS_PIN_OFF SCANNING_PORT |= _BV(THOUSANDS_PIN)
#define HUNDREDS_PIN PINC1  // Hundreds
#define HUNDREDS_PIN_ON SCANNING_PORT &=~_BV(HUNDREDS_PIN)
#define HUNDREDS_PIN_OFF SCANNING_PORT |= _BV(HUNDREDS_PIN)
#define TENS_PIN PINC2      // Tens
#define TENS_PIN_ON SCANNING_PORT &=~_BV(TENS_PIN)
#define TENS_PIN_OFF SCANNING_PORT |= _BV(TENS_PIN)
#define UNITS_PIN PINC3     // Units
#define UNITS_PIN_ON SCANNING_PORT &=~_BV(UNITS_PIN)
#define UNITS_PIN_OFF SCANNING_PORT |= _BV(UNITS_PIN)

// Functions
void init_ports (void); // Initializes ports
void init_timer0(void); // Initializes Timer0
void on_timer0 (void);  // Turns on Timer0
void off_timer0 (void); // Turns off Timer0
void ADC_init (void);   // Initializes ADC
void ADC_on (void);     // Turns on ADC

// Global Variables
volatile uint8_t thousands; // Thousands place
volatile uint8_t hundreds;  // Hundreds place
volatile uint8_t tens;      // Tens place
volatile uint8_t units;     // Units place
volatile uint8_t i = 0;     // Counter variable

// Main
int main(void)
{
    cli(); 
    init_ports();
    init_timer0();
    ADC_init(); 
    sei();
    on_timer0();
    while (1)
    {
        ADC_on();
        if (i == 0)
        {
            DISPLAY = thousands;
            SCANNING_PORT = ~(1<< i);
        }
        else if (i == 1)
        {
            DISPLAY = hundreds;
            SCANNING_PORT = ~(1<< i);
        }
        else if (i == 2)
        {
            DISPLAY = tens;
            SCANNING_PORT = ~(1<< i);
        }
        else if (i == 3)
        {
            DISPLAY = units;
            SCANNING_PORT = ~(1<< i);
        }
        if (i >= 3)
        {
            i = 0;
        }
        else
        {
            i++;
        }
    }
}

// Function definitions
void init_ports(void)
{
    // Outputs
    DDRB |= 0xFF;
    PORTB |= 0X00;
    DDRC |= _BV(THOUSANDS_PIN)|_BV(HUNDREDS_PIN)|_BV(TENS_PIN)|_BV(UNITS_PIN);
}

// Timer
void init_timer0(void)
{
    // CTC Mode
    TCCR0B &= ~_BV(WGM02);  // 0
    TCCR0A |= _BV(WGM01);   // 1
    TCCR0A &= ~_BV(WGM00);  // 0
    // Prescaler 1024
    TCCR0B |= _BV(CS02);
    TCCR0B &=~ _BV(CS01);
    TCCR0B |= _BV(CS00);
    // TOP 0.1 second 
    OCR0A = 156;
    // ENABLE
    TIMSK0 |= _BV(OCIE0A);
}

void on_timer0 (void)
{
    // Reset count
    TCNT0 = 0;
    // Prescaler 1024
    TCCR0B |= _BV(CS02);    // 1
    TCCR0B &= ~_BV(CS01);   // 0
    TCCR0B |= _BV(CS00);    // 1
}

void off_timer0(void)
{
    // STOP
    TCCR0B &= ~_BV(CS02); // 0
    TCCR0B &= ~_BV(CS01); // 0
    TCCR0B &= ~_BV(CS00); // 0
}

ISR (TIMER0_COMPA_vect)
{
    if (i >= 3)
    {
        i = 0;
    }
    else
    {
        i++;
    }
}

// ADC
void ADC_init(void)
{
    // AVCC as reference pin---AVCC reference
    ADMUX &= ~_BV(REFS1);   //1
    ADMUX |= _BV(REFS0);    //0
    // Adjust to 8 bits
    ADMUX |= (1<<ADLAR); // Only using 8 bits out of 10
    // Choose PIN to read ADC5
    ADMUX &=~_BV(MUX3);     //1
    ADMUX |= _BV(MUX2);     //0
    ADMUX &=~_BV(MUX1);     //1
    ADMUX |= _BV(MUX0);     //0
    // Free running
    ADCSRA |= _BV(ADATE);
    // Enable interrupt
    ADCSRA |= _BV(ADIE);
    // 1 MHz clock / 8 = 125 kHz ADC
    ADCSRA &=~ (1<<ADPS0);  //1
    ADCSRA |= (1<<ADPS1);   //0
    ADCSRA |= (1<<ADPS2);   //0
}

void ADC_on(void)
{
    // Turn on ADC
    ADCSRA |= _BV(ADEN);
    _delay_ms(20);
    // Start conversion
    ADCSRA |= _BV(ADSC);
}

ISR(ADC_vect)
{ // ADC signal is handled within the range of 0 to 255
    if (ADCH >= 0 && ADCH <= 50)
    {
        thousands = 0x7C;   // b
        hundreds = 0XF7;    // A
        tens = 0x0E;        // J
        units = 0XF7;       // A
    }
    if (ADCH >= 51 && ADCH <= 100)
    {
        thousands = 0x7C;   // b
        hundreds = 0X06;    // I
        tens = 0xF9;        // E
        units = 0X54;       // n
    }
    if (ADCH >= 101 && ADCH <= 255)
    {
        thousands = 0XF7;   // A
        hundreds = 0X38;    // L
        tens = 0x78;        // t
        units = 0XF7;       // A
    }
}
