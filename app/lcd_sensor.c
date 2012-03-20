#include "arm_comm.h"
#include "board.h"
#include "sys.h"

//_lcdPressedCallback = NULL;
int _stage = 0;

#pragma optimize = none
void delay(int value)
{
  for(int delay = 0; delay < value; delay++); 
}

char getXLpullup(void)
{
  PINMODE1_bit.P0_25 = 0; //pull-up resistor
  FIO0DIR_bit.P0_25 = 0; 
  delay(200);
  return FIO0PIN_bit.P0_25;
}

void initAdc(void)
{
  PINSEL1_bit.P0_23 = 1; //function 1 - adc
  PINSEL1_bit.P0_24 = 1; //function 1 - adc

  AD0CR_bit.CLKDIV = SYS_GetFpclk(ADC_PCLK_OFFSET)/4500000 - 1;    //(72 MHz / 4.5 MHz)  - 1
  AD0CR_bit.BURST = 0;
  AD0CR_bit.CLKS = 0;
  AD0CR_bit.PDN = 1;
}

void setYUasAdc(void)
{
  PINSEL1_bit.P0_23 = 1; //function 1 - adc  
  AD0CR_bit.SEL = 1;
}

void setXRasAdc(void)
{
  PINSEL1_bit.P0_24 = 1; //function 1 - adc
  AD0CR_bit.SEL = 2;
}

int getYUadc(void)
{
  AD0CR_bit.START = 1;
  while(!ADDR0_bit.DONE);
  return ADDR0_bit.RESULT;
}

int getXRadc(void)
{
  AD0CR_bit.START = 1;
  while(!ADDR1_bit.DONE);
  return ADDR1_bit.RESULT;
}

void setXLout(char value)
{
  FIO0DIR_bit.P0_25 = 1;
  FIO0PIN_bit.P0_25 = value;
}

char getXL(void)
{
  FIO0DIR_bit.P0_25 = 0;
  PINMODE1_bit.P0_25 = 2; //no pull-up resistor  
  return FIO0PIN_bit.P0_25;
}

char getXR(void)
{
  PINSEL1_bit.P0_24 = 0; //function 0 - gpio
  FIO0DIR_bit.P0_24 = 0;
  PINMODE1_bit.P0_24 = 2; //no pull-up resistor
  return FIO0PIN_bit.P0_24;
}

void setXRout(char value)
{
  PINSEL1_bit.P0_24 = 0; //function 0 - gpio
  FIO0DIR_bit.P0_24 = 1;
  FIO0PIN_bit.P0_24 = value;
}

char getYD(void)
{
 // PINSEL7_bit.P3_28 = 0; 
  FIO0DIR_bit.P0_26 = 0;
  PINMODE1_bit.P0_26 = 2; //no pull-up resistor
  return FIO0PIN_bit.P0_26;
}

void setYDout(char value)
{
  FIO0DIR_bit.P0_26 = 1;
  FIO0PIN_bit.P0_26 = value;
}

void setYUout(char value)
{
  PINSEL1_bit.P0_23 = 0; //function 0 - gpio
  FIO0DIR_bit.P0_23 = 1;
  FIO0PIN_bit.P0_23 = value; 
}

//-------------------------------

char getScreenPressed(void)
{
  setYUout(0);
  getXR();
  getYD();
  return !getXLpullup();
}

void setState1(void)
{
  setXLout(0);
  setXRout(1);
  getYD(); //set UD to high-impedance state
  setYUasAdc();
}

void setState2(void)
{
  setYDout(0);
  setYUout(1);
  getXL(); //set UD to high-impedance state
  setXRasAdc();
}
