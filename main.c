#include <msp430.h>
#include <inttypes.h>
#include<stdio.h>

#define CMD         0
#define DATA        1

#define Sensor      BIT0      
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

void delay(uint16_t t)
{
    uint16_t i;
    for(i=t; i > 0; i--)
        __delay_cycles(100);
}

void pulseEN(void)
{
    LCD_OUT |= EN;
    delay(1);
    LCD_OUT &= ~EN;
    delay(1);
}


void lcd_write(uint8_t value, uint8_t mode)
{
    if(mode == CMD)
        LCD_OUT &= ~RS;  
    else
        LCD_OUT |= RS;          

    LCD_OUT = ((LCD_OUT & 0x0F) | (value & 0xF0));    
    pulseEN();
    delay(1);

    LCD_OUT = ((LCD_OUT & 0x0F) | ((value << 4) & 0xF0));       
    pulseEN();
    delay(1);
}

void lcd_print(char *s)
{
    while(*s)
    {
        lcd_write(*s, DATA);
        s++;
    }
}


void lcd_setCursor(uint8_t row, uint8_t col)
{
    const uint8_t row_offsets[] = { 0x00, 0x40};
    lcd_write(0x80 | (col + row_offsets[row]), CMD);
    delay(1);
}


void lcd_printNumber(unsigned int num)
{
    char buf[3];                        
    char *str = &buf[2];                

    *str = '\0';                      

    do
    {
        unsigned long m = num;         
        num /= 10;              
        char c = (m - 10 * num) + '0';  
        *--str = c;                    
    } while(num);

    lcd_print(str);
}

void lcd_init()
{
    LCD_DIR |= (D4+D5+D6+D7+RS+EN);
    LCD_OUT &= ~(D4+D5+D6+D7+RS+EN);

    delay(150);                     
    lcd_write(0x33, CMD);          
    delay(50);                     
    lcd_write(0x32, CMD);           
    delay(1);                     
    lcd_write(0x28, CMD);          
    delay(1);

    lcd_write(0x0C, CMD);          
    delay(1);

    lcd_write(0x01, CMD);          
    delay(20);

    lcd_write(0x06, CMD);           
    delay(1);

    lcd_setCursor(0,0);            
}

void register_settings_for_ADC10()
{
    ADC10AE0 |= AIN;                            
    ADC10CTL1 = INCH_0;                        
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;  
}

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;                    

    lcd_init();                                    
    lcd_write(0x01, CMD);                  
    delay(20);

    register_settings_for_ADC10();
    int count=0;

    while(1)
    {
        ADC10CTL0 |= ENC + ADC10SC;    

        P2DIR &= ~Sensor;

        P2DIR |= led;

        P2DIR |= led1;
        P2DIR |= led2;

        while(ADC10CTL1 & ADC10BUSY);          

        float adc_value = 0;

                adc_value = (ADC10MEM) * (99.99) / (1023.00);   

                int int_part = adc_value;                                  
                int decimal_part = (adc_value - (float)int_part) * 10.0 ;  




                if(int_part>90){
                    count = count-1;
                    lcd_setCursor(1,9);
                                    lcd_print("Out");
                    lcd_setCursor(1,13);
                                    lcd_printNumber(count);          
               else{
                   lcd_setCursor(0,0);
                                   lcd_print("");}

                lcd_setCursor(0,1);
                lcd_print("Visitors");
                lcd_setCursor(0,13);
                lcd_printNumber(count);           
                delay(6000);




            if((P2IN & Sensor)) 
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


        else

        {
            lcd_write(0x01, CMD);
            delay(20);
            lcd_setCursor(1,0);
            lcd_print(" ");
            P2OUT |= led;
            delay(5000);

        }
      
        delay(6000);

        if(count>0){
            P2OUT |= led2;
        }
        else{
            P2OUT &=~ led2;
        }

    }
}
// code by -@bugruster
