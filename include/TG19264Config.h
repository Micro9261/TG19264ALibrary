#ifndef __TG19264A_CONFIG__
#define __TG19264A_CONFIG__

//Configuration File
/*
Connection between display and MCU is defined by:
DATA - 8 bit bidirectional port (all pins in 1 port), D0 ~ D7 in order: Pin0 -> D0, Pin1 -> D1 etc
RS_PIN - 1 bit output port
RW_PIN - 1 bit output port
E_PIN - 1 bit output port
CS1_PIN - 1 bit output port
CS2_PIN - 1 bit output port
CS3_PIN  - 1 bit output port
RES_PIN - 1 bit output port
*/

#ifdef ATmega
/*
Section with configuration for ATmega MCU
(suitable for all MCUs which requires only Port & Direction Registers to operate)
X_PORT <- defines Port Register for given pin/s
X_DDR <- defines Data Direction Register for given pin/s
X_NUM <- defines pin number in 1 bit operations
X_READ <- defines Read Port Register for given pin/s (used by RS and DATA)
OUTPUT <- value to set port as output
INPUT <- value to set port as input
OUTPUT_8BIT <-value to set 8 bits as output (used by DATA)
PULLUP_8BIT <- value to set 8 bits as pullup [input] (used by DATA)
INPUT_8BIT <-value to set 8 bits as input (used by DATA)
*/

//Include suitable header file with MCU register here
#include <xc.h>

//Data Port
#define DATA_PORT	PORTA
#define DATA_DDR	DDRA
#define DATA_READ	PINA

//RS_PIN
#define RS_PIN_PORT	PORTC
#define RS_PIN_DDR	DDRC
#define RS_PIN_NUM	4
#define RS_PIN_READ PINC

//RW_PIN
#define RW_PIN_PORT	PORTC
#define RW_PIN_DDR	DDRC
#define RW_PIN_NUM	5

//E_PIN
#define E_PIN_PORT	PORTC
#define E_PIN_DDR	DDRC
#define E_PIN_NUM	3

//CS1_PIN
#define CS1_PIN_PORT  PORTD
#define CS1_PIN_DDR	  DDRD
#define CS1_PIN_NUM	  6

//CS2_PIN
#define CS2_PIN_PORT  PORTC
#define CS2_PIN_DDR	  DDRC
#define CS2_PIN_NUM	  6

//CS3_PIN
#define CS3_PIN_PORT  PORTC
#define CS3_PIN_DDR   DDRC
#define CS3_PIN_NUM   7

//RES_PIN
#define RES_PIN_PORT  PORTC
#define RES_PIN_DDR   DDRC
#define RES_PIN_NUM   2

//Pin states
#define OUTPUT 1
#define INPUT 0

//DATA states
#define OUTPUT_8BIT 0xFF
#define INPUT_8BIT 0x0
#define PULLUP_8BIT 0xFF

#endif // ATmega

/*
Delay configuration:
Add suitable header file with delays function and define clock freq if needed
*/
#define F_CPU 16000000UL
#include <util/delay.h>

#endif //__TG19264A_CONFIG__