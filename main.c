#include <msp430.h>
#include <inttypes.h>
#include<stdio.h>

#define CMD         0
#define DATA        1

#define Sensor      BIT0        // Channel A0
#define AIN         BIT0

#define LCD_OUT     P1OUT
#define LCD_DIR     P1DIR
#define D4          BIT4
#define D5          BIT5
#define D6          BIT6
#define D7          BIT7
#define RS          BIT2
#define EN          BIT3

#define led          BIT1
#define led1         BIT2
#define led2         BIT3
#define led3         BIT4



/**
 *@brief Delay function for producing delay in 0.1 ms increments
 *@param t milliseconds to be delayed
 *@return void
 **/
void delay(uint16_t t)
{
    uint16_t i;
    for(i=t; i > 0; i--)
        __delay_cycles(100);
}

/**
 *@brief Function to pulse EN pin after data is written
 *@return void
 **/
void pulseEN(void)
{
    LCD_OUT |= EN;
    delay(1);
    LCD_OUT &= ~EN;
    delay(1);
}

/**
 *@brief Function to write data/command to LCD
 *@param value Value to be written to LED
 *@param mode Mode -> Command or Data
 *@return void
 **/
void lcd_write(uint8_t value, uint8_t mode)
{
    if(mode == CMD)
        LCD_OUT &= ~RS;             // Set RS -> LOW for Command mode
    else
        LCD_OUT |= RS;              // Set RS -> HIGH for Data mode

    LCD_OUT = ((LCD_OUT & 0x0F) | (value & 0xF0));              // Write high nibble first
    pulseEN();
    delay(1);

    LCD_OUT = ((LCD_OUT & 0x0F) | ((value << 4) & 0xF0));       // Write low nibble next
    pulseEN();
    delay(1);
}

/**
 *@brief Function to print a string on LCD
 *@param *s pointer to the character to be written.
 *@return void
 **/
void lcd_print(char *s)
{
    while(*s)
    {
        lcd_write(*s, DATA);
        s++;
    }
}

/**
 *@brief Function to move cursor to desired position on LCD
 *@param row Row Cursor of the LCD
 *@param col Column Cursor of the LCD
 *@return void
 **/
void lcd_setCursor(uint8_t row, uint8_t col)
{
    const uint8_t row_offsets[] = { 0x00, 0x40};
    lcd_write(0x80 | (col + row_offsets[row]), CMD);
    delay(1);
}

/**
 *@brief Function to change numeric value into it's corresponding char array
 *@param num Number which has to be displayed
 *@return void
 **/
void lcd_printNumber(unsigned int num)
{
    char buf[3];                        // Creating a array of size 3
    char *str = &buf[2];                // Initializing pointer to end of the array

    *str = '\0';                        // storing null pointer at end of string

    do
    {
        unsigned long m = num;          // Storing number in variable m
        num /= 10;                      // Dividing number by 10
        char c = (m - 10 * num) + '0';  // Finding least place value and adding it to get character value of digit
        *--str = c;                     // Decrementing pointer value and storing character at that character
    } while(num);

    lcd_print(str);
}

void lcd_init()
{
    LCD_DIR |= (D4+D5+D6+D7+RS+EN);
    LCD_OUT &= ~(D4+D5+D6+D7+RS+EN);

    delay(150);                     // Wait for power up ( 15ms )
    lcd_write(0x33, CMD);           // Initialization Sequence 1
    delay(50);                      // Wait ( 4.1 ms )
    lcd_write(0x32, CMD);           // Initialization Sequence 2
    delay(1);                       // Wait ( 100 us )

    // All subsequent commands take 40 us to execute, except clear & cursor return (1.64 ms)

    lcd_write(0x28, CMD);           // 4 bit mode, 2 line
    delay(1);

    lcd_write(0x0C, CMD);           // Display ON, Cursor OFF, Blink OFF
    delay(1);

    lcd_write(0x01, CMD);           // Clear screen
    delay(20);

    lcd_write(0x06, CMD);           // Auto Increment Cursor
    delay(1);

    lcd_setCursor(0,0);             // Goto Row 1 Column 1
}

/**
 * @brief
 * These settings are wrt enabling ADC10 on Lunchbox
 **/

void register_settings_for_ADC10()
{
    ADC10AE0 |= AIN;                            // P1.0 ADC option select
    ADC10CTL1 = INCH_0;                         // ADC Channel -> 1 (P1.0)
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;  // Ref -> Vcc, 64 CLK S&H , ADC - ON
}


//@brief entry point for the code/
void main(void) {
    WDTCTL = WDTPW + WDTHOLD;                       //! Stop Watchdog (Not recommended for code in production and devices working in field)
    //P1DIR &= ~BIT0;


    lcd_init();                                     // Initializing LCD
    lcd_write(0x01, CMD);                   // Clear screen
    delay(20);

    register_settings_for_ADC10();
    int count=0;

    while(1)
    {
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

        P2DIR &= ~Sensor;

        P2DIR |= led;

        P2DIR |= led1;
        P2DIR |= led2;

        while(ADC10CTL1 & ADC10BUSY);           // Wait for conversion to end

        float adc_value = 0;

                adc_value = (ADC10MEM) * (99.99) / (1023.00);    // mapping 10-bit conversion result of ADC to corresponding voltage

                int int_part = adc_value;                                   // Integer part of calculated ADC value
                int decimal_part = (adc_value - (float)int_part) * 10.0 ;  // Decimal part of calculated ADC value




                if(int_part>90){
                    count = count-1;
                    lcd_setCursor(1,9);
                                    lcd_print("Out");
                    lcd_setCursor(1,13);
                                    lcd_printNumber(count);              // Displaying integer part of ADC value
                                    //lcd_print(".");
                                    //lcd_printNumber(decimal_part);          // Displaying decimal part of ADC value
                                    delay(6000); }
               else{
                   lcd_setCursor(0,0);
                                   lcd_print("");}

                lcd_setCursor(0,1);
                lcd_print("Visitors");
                lcd_setCursor(0,13);
                lcd_printNumber(count);              // Displaying integer part of ADC value
                //lcd_print(".");
                //lcd_printNumber(decimal_part);          // Displaying decimal part of ADC value
                delay(6000);




            if((P2IN & Sensor)) // If SW is Pressed
                {
                    lcd_write(0x01, CMD);
                    delay(20);
                    count = count+1;
                    lcd_setCursor(1,0);
                            lcd_print("IN");
                    lcd_setCursor(1,3);
                    lcd_printNumber(count);
                    P2OUT &=~ led;// Toggle LED
                    delay(5000);

                }

        //if(P1IN == 0x01) //  IF IR IS HIGH

       // {

           // lcd_setCursor(0,0);
           // lcd_print("IR is high");

       //delay(5000);//GIVE SOME DELAY

       // }

        else

        {
            lcd_write(0x01, CMD);
            delay(20);
            lcd_setCursor(1,0);
            lcd_print(" ");
            P2OUT |= led;
            delay(5000);

        }
       // float adc_value = 0;

        //adc_value = (ADC10MEM) * (3.30) / (1023.00);    // mapping 10-bit conversion result of ADC to corresponding voltage

        //int int_part = adc_value;                                   // Integer part of calculated ADC value
        //int decimal_part = (adc_value - (float)int_part) * 10.0 ;  // Decimal part of calculated ADC value/




        //lcd_setCursor(1,3);
        //lcd_printNumber(int_part);              // Displaying integer part of ADC value
        //lcd_print(".");
        //lcd_printNumber(decimal_part);          // Displaying decimal part of ADC value
        //lcd_setCursor(1,7);
        //lcd_print("V");
        delay(6000);

        if(count>0){
            P2OUT |= led2;
        }
        else{
            P2OUT &=~ led2;
        }

    }
}
