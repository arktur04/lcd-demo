/*************************************************************************
 * Программа, демонстрирующая работу цветного LCD-индикатора 
 * и резистивного сенсора
 *
 * main.c
 **************************************************************************/
#include <intrinsics.h>
#include <stdio.h>
#include "board.h"
#include "sys.h"
#include "drv_glcd.h"
#include "logo.h"
#include "Cursor.h"
#include "stdlib.h"

#include "lcd_sensor.h"

#define TIMER0_TICK_PER_SEC   2

const float coeffXa = 0.46, coeffXb = -60.0, coeffYa = -0.39, coeffYb = 340.0;

char _state = 0;
void debugPin(char val)
{
  FIO2PIN_bit.P2_15 = val;
}

/*************************************************************************
 * Function Name: Timer0IntrHandler
 * Parameters: none
 *
 * Return: none
 *
 * Description: Timer 0 interrupt handler
 *
 *************************************************************************/
void Timer0IntrHandler (void)
{
  // tests
  _state ^= 1;
  debugPin( _state ); 
  /*  testXL( _state );
  testXR( _state );
  testYU( _state );
  testYD( _state );
  */
  // clear interrupt
  T0IR_bit.MR0INT = 1;
  VICADDRESS = 0;
}

void FillMem(void)
 { 
   for(int x = 0; x < 320; x++)
   {
     for(int y = 0; y < 240; y++)
     {
       *(Int32U*) (((Int32U)SRAM_BASE_ADDR) + (y * 320 + x) * 4) =
         (x < 160)? 0x000000FF: 0;
     }
   }
  }

Int32U median3(Int32U * pArr)
{
  if (((pArr[1] <= pArr[0]) && (pArr[0] <= pArr[2])) ||
      ((pArr[1] >= pArr[0]) && (pArr[0] >= pArr[2])))
    return pArr[0];
  
  if (((pArr[0] <= pArr[1]) && (pArr[1] <= pArr[2])) ||
      ((pArr[0] >= pArr[1]) && (pArr[1] >= pArr[2])))
    return pArr[1];

 // if (((pArr[0] <= pArr[2]) && (pArr[2] <= pArr[1])) ||
 //     ((pArr[0] >= pArr[2]) && (pArr[2] >= pArr[1])))
    return pArr[2];  
}

char scrPressed(char * charArr)
{
  for(char i = 0; i < 3; i++)
  {
    if(!charArr[i])
    {
      return FALSE;
    }
  }
  return TRUE;
}

/*************************************************************************
 * Function Name: main
 * Parameters: none
 *
 * Return: none
 *
 * Description: main
 *
 *************************************************************************/
int main(void)
{ 
int cursor_x = (C_GLCD_H_SIZE - CURSOR_H_SIZE) / 2;
int cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE) / 2;
Int32U adcX[3], adcY[3];
Int32U medianX, medianY;
char screenPressed[3];

  GLCD_Ctrl (FALSE);

  // Init GPIO
  GpioInit();

  // MAM init
  MAMCR_bit.MODECTRL = 0;
  MAMTIM_bit.CYCLES  = 3;   // FCLK > 40 MHz
  MAMCR_bit.MODECTRL = 2;   // MAM functions fully enabled

  InitClock();

  ExtMemInit();

  //reset LCD panel
  FIO0DIR_bit.P0_19 = 1;
  FIO0CLR_bit.P0_19 = 1;
  
  delay(1000000);
  FIO0SET_bit.P0_19 = 1;
  //-------------------------------------
  
  VIC_Init();
 
  SetVramAddr((pInt32U)SRAM_BASE_ADDR);
 // SetPalAddr((pInt32U)&palette);
  
 // GLCD init
 // FillMem();
  GLCD_Init (LogoPic.pPicStream, (pInt32U)&palette);//NULL);

  GLCD_Cursor_Dis(0);

  GLCD_Copy_Cursor ((Int32U *)Cursor, 0, sizeof(Cursor)/sizeof(Int32U));

  GLCD_Cursor_Cfg(CRSR_FRAME_SYNC | CRSR_PIX_32);

  GLCD_Move_Cursor(cursor_x, cursor_y);

  GLCD_Cursor_En(0);

  // Enable TIM0 clocks
  PCONP_bit.PCTIM0 = 1; // enable clock
  PCONP_bit.PCAD = 1;
  PCLKSEL0_bit.PCLK_ADC = 1;

  // Init Time0
  T0TCR_bit.CE = 0;     // counting  disable
  T0TCR_bit.CR = 1;     // set reset
  T0TCR_bit.CR = 0;     // release reset
  T0CTCR_bit.CTM = 0;   // Timer Mode: every rising PCLK edge
  T0MCR_bit.MR0I = 1;   // Enable Interrupt on MR0
  T0MCR_bit.MR0R = 1;   // Enable reset on MR0
  T0MCR_bit.MR0S = 0;   // Disable stop on MR0
  // set timer 0 period
  T0PR = 0;
  T0MR0 = SYS_GetFpclk(TIMER0_PCLK_OFFSET)/(TIMER0_TICK_PER_SEC);
  // init timer 0 interrupt
  T0IR_bit.MR0INT = 1;  // clear pending interrupt
  VIC_SetVectoredIRQ(Timer0IntrHandler,0,VIC_TIMER0);
  VICINTENABLE |= 1UL << VIC_TIMER0;
  T0TCR_bit.CE = 1;     // counting Enable
  __enable_interrupt();
  
  GLCD_Ctrl (TRUE);                                                            //

#if 0
  SRAM_Test();
#endif
    
#if 0
 FillMem();
#endif
 
 //debug led
  FIO2DIR_bit.P2_15 = 1;
  FIO2CLR_bit.P2_15 = 1; 
  
  initAdc();
  
  cursor_x = C_GLCD_H_SIZE - CURSOR_H_SIZE/2;
  cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE/2);
  
  while(1)
  {
    #if 1
    if( getScreenPressed() )
    {
      for(int i = 0; i < 3; i++)
      {
        if( getScreenPressed())
        {
          screenPressed[i] = TRUE;
        setState1();
        delay(500);
        adcX[i] = getYUadc();
        setState2();
        delay(500);
        adcY[i] = getXRadc();
        }
        else
        {
          screenPressed[i] = FALSE;
          break;
        }
      };
    }
    if( scrPressed(screenPressed) ) // if the sensor is still pressed
    {
      medianX = median3(adcX);
      medianY = median3(adcY);
      
      cursor_x = (int)(medianX * coeffXa + coeffXb);
      cursor_y = (int)(medianY * coeffYa + coeffYb);
      if(cursor_x < 0) cursor_x = 0;
      if(cursor_x > C_GLCD_H_SIZE - 1) cursor_x = C_GLCD_H_SIZE - 1;
      if(cursor_y < 0) cursor_y = 0;
      if(cursor_y > C_GLCD_H_SIZE - 1) cursor_y = C_GLCD_H_SIZE - 1;
    }
    for(char i = 0; i < 3; i++)
    {
      screenPressed[i] = FALSE;
    }
    #endif
 
//    cursor_x = C_GLCD_H_SIZE - CURSOR_H_SIZE/2;
//    cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE/2);
    GLCD_Move_Cursor(cursor_x, cursor_y);
  }
}
