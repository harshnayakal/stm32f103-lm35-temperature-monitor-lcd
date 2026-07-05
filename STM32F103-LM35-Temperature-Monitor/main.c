/*---------------------------------------------------------
  Program     : ADC Interfacing with STM32F103C8T6
  Device      : STM32F103C8T6
  Toolchain   : Keil uVision
  Header      : stm32f10x.h

  Description :
  Reads LM35 output which is connected to PA0 / ADC1 Channel 0
  and display it on LCD.

  ADC Resolution:
      12-bit

  ADC Digital Range:
      0 to 4095

  Analog Voltage Range:
      0V to 3.3V

  Hardware Connections:
      LM35 o/p -> PA0
  LCD:
      Connected in 8-bit mode
			LCD Data  : PB8 - PB15
			RS        : PB0
			EN        : PB1
			RW        : GND
---------------------------------------------------------*/

#include <stm32f10x.h>

void delay_us(unsigned int);   // Function prototype: delay in microseconds
void delay_ms(unsigned int);   // Function prototype: delay in milliseconds
void delay_s(unsigned int);    // Function prototype: delay in seconds

#define LCD_RS_PIN    0		//PB0
#define LCD_EN_PIN    1		//PB1


//HD44780 Commands 
// Basic Commands
#define CLEAR_LCD              0x01
#define RET_CUR_HOME           0x02

// Entry Mode Commands
#define ENTRY_CUR_RIGHT        0x06
#define ENTRY_CUR_LEFT         0x04

#define ENTRY_DISP_SHIFT_R     0x05
#define ENTRY_DISP_SHIFT_L     0x07

// Display Control Commands
#define DSP_OFF                0x08
#define DSP_ON_CUR_OFF         0x0C
#define DSP_ON_CUR_ON          0x0E
#define DSP_ON_CUR_BLINK       0x0F

// Cursor Shift Commands
#define CURSOR_SHIFT_LEFT      0x10
#define CURSOR_SHIFT_RIGHT     0x14

// Display Shift Commands
#define DISPLAY_SHIFT_LEFT     0x18
#define DISPLAY_SHIFT_RIGHT    0x1C

// Function Set Commands
#define MODE_8BIT_1LINE        0x30
#define MODE_4BIT_1LINE        0x20

#define MODE_8BIT_2LINE        0x38
#define MODE_4BIT_2LINE        0x28

// DDRAM Address Commands
#define GOTO_LINE1_POS0        0x80
#define GOTO_LINE2_POS0        0xC0
#define GOTO_LINE3_POS0        0x94
#define GOTO_LINE4_POS0        0xD4

// CGRAM Address Command
#define GOTO_CGRAM_START       0x40

void LCD_Command(unsigned char);
void LCD_Char(unsigned char);
void LCD_String(unsigned char *);
void LCD_Init(void);
void LCD_SetCursor(unsigned char, unsigned char);
void LCD_UInt(unsigned int);
void LCD_Float(float, unsigned char);

//ADC function declarations
void ADC1_Init(void);
uint16_t ADC1_Read(void);


uint16_t adcVal;	// Stores 12-bit ADC result, Range: 0 to 4095
float eAR;				//Stores calculated analog voltage, Range: 0.0v to 3.3v



int main(void)
{

	LCD_Init();	//Initialize LCD module.
  ADC1_Init();	//Initialize ADC1 peripheral.
	LCD_Command(GOTO_LINE1_POS0);	//Move cursor to: Line 1 Position 0
	LCD_String("ADC test ");	//Display startup message.
	delay_ms(500);		// Delay for visibility.

  while(1)
  {
        /*
          Read ADC value from PA0 / ADC1_CH0.

          adcVal range:
              0    -> 0V
              4095 -> 3.3V
        */
		LCD_Command(CLEAR_LCD);	//Clear LCD display.
    adcVal = ADC1_Read();		//Read ADC value
		/*
		 Convert ADC digital value into analog voltage.
		 Formula:

							ADC_Value ? Vref
			Voltage = ----------------
										4095

     Here: Vref = 3.3V
		*/
		eAR = ((float)adcVal*3.3f)/4095.0f;	
		
		LCD_Command(GOTO_LINE1_POS0);	// Move cursor to: Line 1 Position 0
		LCD_String("Room Temp : ");		// Display label
		LCD_Command(GOTO_LINE2_POS0);	// Move cursor to: Line 2 Position 0
		LCD_Float(eAR*100,2);					// Display room temp with 2 decimal points.
		LCD_Char(0xDF);								// Display degrees symbol 
		LCD_Char('C');								// Display temperature unit.
		delay_ms(1000);								// Delay before next update.
  }
}

/*---------------------------------------------------------
Function Name : ADC1_Init

Description : 
Initializes ADC1 peripheral for: PA0 -> ADC1 Channel 0

Steps:
      1. Enable clocks
      2. Configure PA0 as analog input
			3. Configure ADC Clock
      4. Power ON ADC
      5. Calibrate ADC
      6. Configure sample time
---------------------------------------------------------*/
void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;   // Enable GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;   // Enable ADC1 clock

    /*
    PA0 as Analog Input
    PA0 = ADC1 Channel 0
	    
  	PA0 uses CRL bits 3:0
    MODE = 00
    CNF  = 00
    Binary = 0000
    */
    GPIOA->CRL &= ~(0xF << (0 * 4));
	
		/*
		Configure ADC Clock

		ADC clock must not exceed 14 MHz.

		ADCPRE bits:
				00 -> PCLK2/2
				01 -> PCLK2/4
				10 -> PCLK2/6
				11 -> PCLK2/8

		Here:
				ADCCLK = 72MHz / 6 = 12MHz
		*/

		RCC->CFGR &= ~(3 << 14);
		RCC->CFGR |=  (2 << 14);

    /*
			Power ON ADC1 peripheral.
      ADON bit = ADC_CR2 bit 0
    */
    ADC1->CR2 |= ADC_CR2_ADON;
	
		// Small delay for ADC stabilization.
    delay_ms(2);

    /*
			Start ADC Calibration
      Calibration improves ADC conversion accuracy.
    */
		// Set CAL bit to start calibration.
    ADC1->CR2 |= ADC_CR2_CAL;
		
		//  Wait until calibration completes.
    // Hardware clears CAL bit automatically.
    while(ADC1->CR2 & ADC_CR2_CAL);

    /*
			Configure Sampling Time

      Channel 0 sample time bits:
          SMPR2[2:0]

      111 = 239.5 ADC clock cycles

      Larger sample time improves stability
      and accuracy.
    */
    ADC1->SMPR2 |= (7 << (0 * 3));
}


/*---------------------------------------------------------
  Function Name : ADC1_Read
  Description :
  Performs one ADC conversion on Channel 0 and returns 
  12-bit ADC result.
  Returns :
      0 to 4095
-----------------------------------------------------------*/
uint16_t ADC1_Read(void)
{
    /*
      Select ADC Channel 0 as first conversion
      SQR3 register controls first conversion.
    */
    ADC1->SQR3 = 0;

    /*
      Start ADC conversion.

      In STM32F103:
          Second write to ADON bit
          starts conversion.
    */
    ADC1->CR2 |= ADC_CR2_ADON;

    /*
      Wait until conversion complete
			EOC = End Of Conversion flag.
    */
    while(!(ADC1->SR & ADC_SR_EOC));

    /*
      Return ADC conversion result.

      Reading DR automatically clears EOC flag.
    */
    return ADC1->DR;
}

// Send command to LCD
void LCD_Command(unsigned char cmd)
{
    GPIOB->BRR = (1 << LCD_RS_PIN);    // RS = 0 for command

    GPIOB->ODR &= 0x00FF;            // Clear PB8-PB15
    GPIOB->ODR |= cmd<<8;            // Send command byte

    GPIOB->BSRR = (1 << LCD_EN_PIN);   // EN = 1
    delay_us(2);
    GPIOB->BRR  = (1 << LCD_EN_PIN);   // EN = 0
    delay_ms(2);
}

// Send character to LCD
void LCD_Char(unsigned char data)
{
    GPIOB->BSRR = (1 << LCD_RS_PIN);   // RS = 1 for data

    GPIOB->ODR &= 0x00FF;   // Clear PB8-PB15 without affecting other bits
    GPIOB->ODR |= data<<8;	// Send data byte to PB8-PB15

    GPIOB->BSRR = (1 << LCD_EN_PIN);   // EN = 1
    delay_us(2);
    GPIOB->BRR  = (1 << LCD_EN_PIN);   // EN = 0
    delay_ms(2);
}

// Send string to LCD 
void LCD_String(unsigned char *str)
{
    while(*str)
        LCD_Char(*str++);
}

// LCD initialization 
void LCD_Init(void)
{
	  // Enable GPIOB clocks 
    RCC->APB2ENR |= (1 << 3);   // GPIOB clock enable

   
    // PB0, PB1 as output push-pull, 2 MHz
    // MODE = 10, CNF = 00
    GPIOB->CRL &= ~((0xF << 0) | (0xF << 4));
    GPIOB->CRL |=  ((0x2 << 0) | (0x2 << 4));

    // PB8-PB15 as output push-pull, 2 MHz
    // MODE = 10, CNF = 00 --> 0010 = 2
    GPIOB->CRH = 0x22222222;
	
		delay_ms(16);
    LCD_Command(0x30);
    delay_ms(6);
		LCD_Command(0x30);
    delay_ms(1);
    LCD_Command(0x30);
    delay_ms(1);
    LCD_Command(0x38);//set 8-bit mode of operation with 2 lines
											// and 5X7 character font 
		LCD_Command(0x10);//Turning OFF the display
		LCD_Command(0x01);//clearing the display
    LCD_Command(0x06);//Shifting the cursor to the right side
                      //after writing each byte onto the display
		LCD_Command(0x0f);//Enabling the display with blinking cursor
}


// Move cursor
void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;

    if(row == 0)
        address = 0x80 + col;
    else
        address = 0xC0 + col;

    LCD_Command(address);
}

// Display unsigned integer
void LCD_UInt(unsigned int num)
{
	int8_t i=0;
	unsigned char buffer[10];
	
	if(num==0)
	{
		LCD_Char('0');
	}
  else
	{
		while(num>0)
		{
			buffer[i++]=(num%10)+48;
			num/=10;
		}
		for(--i;i>=0;i--)
		{
			LCD_Char(buffer[i]);
		}
	}
}

// Display float with required decimal places
void LCD_Float(float num, unsigned char decimals)
{
	 unsigned int n,i;
	 if(num<0.0)
	 {	 
		 LCD_Char('-');
		 num=-num;
	 }	 
   n=num;
   LCD_UInt(n);
   LCD_Char('.');
	 for(i=0;i<decimals;i++)
	 {
		num=(num-n) * 10;
		n=num;
    LCD_Char(n+48);		 
	 }
}

/* Delay for microseconds */
void delay_us(unsigned int dlyUS)
{
	dlyUS *= 12;		// Scale the input value.
	                		// Each loop iteration consumes ~6 CPU cycles.
	                		// 6 cycles ? 12 iterations = 72 cycles = 1 ?s (for 72 MHz CPU).
	while(dlyUS--)  // Decrement loop counter until it reaches 0.
		__NOP();      // "No Operation" ? 1 CPU cycle instruction, used to stabilize timing.
}

/* Delay for milliseconds */
void delay_ms(unsigned int dlyMS)
{
	dlyMS *= 12000;	// Each ms = 1,000 ?s, so 12 ? 1,000 = 12,000 iterations per ms.
	               		 // Each iteration ~6 cycles ? 6 ? 12,000 = 72,000 cycles = 1 ms.
	while(dlyMS--)  // Loop for the scaled number of counts
		__NOP();      // NOP stabilizes timing and prevents optimization removal.
}

/* Delay for seconds */
void delay_s(unsigned int dlyS)
{
	dlyS *= 12000000;	// Each second = 1,000 ms
	                 		 // 12,000 (for 1 ms) ? 1,000 = 12,000,000 iterations per second.

	while(dlyS--)     // Decrement the counter until zero
		__NOP();        // Consume 1 cycle, maintain timing.
}


