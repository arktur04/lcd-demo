/*************************************************************************
 * ���������, ��������������� ������ �������� LCD-���������� 
 * � ������������ �������
 *
 * sys.h
 **************************************************************************/
#include "sys.h"
#include <nxp/iolpc2478.h>

void ExtMemInit(void)
{
  PINSEL6  &= BIN32(11111111,11111111,00000000,00000000);
  PINMODE6 &= BIN32(11111111,11111111,00000000,00000000);
  PINSEL6  |= BIN32(00000000,00000000,01010101,01010101);
  PINMODE6 |= BIN32(00000000,00000000,10101010,10101010);

  PINSEL8  = BIN32(01010101,01010101,01010101,01010101);
  PINMODE8 = BIN32(10101010,10101010,10101010,10101010);
  
  PINSEL9 &= BIN32(11001111,11001100,11111111,11000000);
  PINSEL9 |= BIN32(00010000,00010001,00000000,00010101);
  PINMODE9&= BIN32(11001111,11001100,11111111,11000000);
  PINMODE9|= BIN32(00100000,00100010,00000000,00101010);

  // Init SDRAM controller
  // Enable EMC clock
  PCONP_bit.PCEMC    = 1;

  EMCSTATICWAITRD0   = 0x00000001;
  EMCSTATICWAITPG0   = 0x00000000;
  EMCSTATICWAITWR0   = 0x00000000;
  EMCSTATICWAITTURN0 = 0x00000000;
  
  EMCCONTROL      = 1;         // enable EMC
}

char SRAM_Test(void)
{
// 32 bits access
  for (Int32U i = 0; i < 0x80000; i+=sizeof(Int32U))
  {
    *(Int32U*)(((Int32U)SRAM_BASE_ADDR)+i) = i;
  }

  for (Int32U i = 0; i < 0x80000; i+=sizeof(Int32U))
  {
    if (*(Int32U*)(((Int32U)SRAM_BASE_ADDR)+i) != i)
    {
      printf("Verification error on address : 0x%x\n",(Int32U)SRAM_BASE_ADDR+i);
      return(FALSE);
    }
  }
  
  return TRUE;
}

#ifndef __FIQ_USED
/*************************************************************************
 * Function Name: fiq_handler
 * Parameters: none
 *
 * Return: none
 *
 * Description: FIQ handler
 *
 *************************************************************************/
__fiq __arm void FIQ_Handler (void)
{
  while(1);
}
#endif // __FIQ_USED

#ifndef __IRQ_USED
/*************************************************************************
 * Function Name: irq_handler
 * Parameters: none
 *
 * Return: none
 *
 * Description: IRQ handler
 *
 *************************************************************************/
__irq __arm void IRQ_Handler (void)
{
void (*interrupt_function)();
unsigned int vector;

  vector = VICADDRESS;     // Get interrupt vector.
  interrupt_function = (void(*)())vector;
  if(interrupt_function != NULL)
  {
    interrupt_function();  // Call vectored interrupt function.
  }
  else
  {
    VICADDRESS = 0;      // Clear interrupt in VIC.
  }
}
#endif // __IRQ_USED

/*************************************************************************
 * Function Name: VIC_Init
 * Parameters: void
 * Return: void
 *
 * Description: Initialize VIC
 *
 *************************************************************************/
void VIC_Init(void)
{
volatile unsigned long * pVecAdd, *pVecCntl;
int i;
  // Assign all interrupt channels to IRQ
  VICINTSELECT  =  0;
  // Disable all interrupts
  VICINTENCLEAR = 0xFFFFFFFF;
  // Clear all software interrupts
  VICSOFTINTCLEAR = 0xFFFFFFFF;
  // VIC registers can be accessed in User or privileged mode
  VICPROTECTION = 0;
  // Clear interrupt
  VICADDRESS = 0;

  // Clear address of the Interrupt Service routine (ISR) for vectored IRQs
  // and disable all vectored IRQ slots
  for(i = 0,  pVecCntl = &VICVECTPRIORITY0, pVecAdd = &VICVECTADDR0; i < 32; ++i)
  {
    *pVecCntl++ = *pVecAdd++ = 0;
  }
}

/*************************************************************************
 * Function Name: VIC_SetVectoredIRQ
 * Parameters:  void(*pIRQSub)()
 *              unsigned int VicIrqSlot
 *              unsigned int VicIntSouce
 *
 * Return: void
 *
 * Description:  Init vectored interrupts
 *
 *************************************************************************/
void VIC_SetVectoredIRQ(void(*pIRQSub)(), unsigned int Priority,
                        unsigned int VicIntSource)
{
unsigned long volatile *pReg;
  // load base address of vectored address registers
  pReg = &VICVECTADDR0;
  // Set Address of callback function to corresponding Slot
  *(pReg+VicIntSource) = (unsigned long)pIRQSub;
  // load base address of ctrl registers
  pReg = &VICVECTPRIORITY0;
  // Set source channel and enable the slot
  *(pReg+VicIntSource) = Priority;
  // Clear FIQ select bit
  VICINTSELECT &= ~(1<<VicIntSource);
}

/*************************************************************************
 * Function Name: InitClock
 * Parameters: void
 * Return: void
 *
 * Description: Initialize PLL and clocks' dividers. Hclk - 288MHz,
 * Cclk- 72MHz, Usbclk - 48MHz
 *
 *************************************************************************/
void InitClock(void)
{
  // 1. Init OSC
  SCS_bit.OSCRANGE = 0;
  SCS_bit.OSCEN = 1;
  // 2.  Wait for OSC ready
  while(!SCS_bit.OSCSTAT);
  // 3. Disconnect PLL
  PLLCON_bit.PLLC = 0;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 4. Disable PLL
  PLLCON_bit.PLLE = 0;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 5. Select source clock for PLL
  CLKSRCSEL_bit.CLKSRC = 1; // Selects the main oscillator as a PLL clock source.
  // 6. Set PLL settings 288 MHz
  PLLCFG_bit.MSEL = 24-1;
  PLLCFG_bit.NSEL = 2-1;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 7. Enable PLL
  PLLCON_bit.PLLE = 1;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // 8. Wait for the PLL to achieve lock
  while(!PLLSTAT_bit.PLOCK);
  // 9. Set clk divider settings
  CCLKCFG   = 4-1;            // 1/4 Fpll - 72 MHz
  USBCLKCFG = 6-1;            // 1/6 Fpll - 48 MHz
  PCLKSEL0 = PCLKSEL1 = 0;    // other peripherals - 18MHz (Fcclk/4)
  // 10. Connect the PLL
  PLLCON_bit.PLLC = 1;
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  // stop all Peripherals' clocks
  PCONP = 0;

  // Configure AHB1 arbitration
  AHBCFG1_bit.SHDL = 0;     // priority arbitration
  AHBCFG1_bit.BB   = 0;     // Never break defined length bursts.
  AHBCFG1_bit.QT   = 0;     // A quantum is an AHB clock.
  AHBCFG1_bit.QT   = 4;     // Preemptive, re-arbitrate after 16 AHB quanta.
  AHBCFG1_bit.DM   = 1;     // Master 1 (CPU) is the default master.
  AHBCFG1_bit.EP1  = 1;     // External priority for master 1 (CPU).
  AHBCFG1_bit.EP2  = 2;     // External priority for master 2 (GPDMA).
  AHBCFG1_bit.EP3  = 3;     // External priority for master 3 (AHB1).
  AHBCFG1_bit.EP4  = 5;     // External priority for master 4 (USB).
  AHBCFG1_bit.EP5  = 4;     // External priority for master 5 (LCD).
}

/*************************************************************************
 * Function Name: SYS_GetFsclk
 * Parameters: none
 * Return: Int32U
 *
 * Description: return Sclk [Hz]
 *
 *************************************************************************/
Int32U SYS_GetFsclk(void)
{
Int32U Mul = 1, Div = 1, Osc, Fsclk;
  if(   PLLSTAT_bit.PLLC
     && PLLSTAT_bit.PLLE)
  {
    // when PLL is connected
    Mul = PLLSTAT_bit.MSEL + 1;
    Div = PLLSTAT_bit.NSEL + 1;
  }

  // Find clk source
  switch(CLKSRCSEL_bit.CLKSRC)
  {
  case 0:
    Osc = I_RC_OSC_FREQ;
    break;
  case 1:
    Osc = MAIN_OSC_FREQ;
    break;
  case 2:
    Osc = RTC_OSC_FREQ;
    break;
  default:
    Osc = 0;
  }
  // Calculate system frequency
  Fsclk = Osc*Mul*2;
  Fsclk /= Div*(CCLKCFG+1);
  return(Fsclk);
}

/*************************************************************************
 * Function Name: SYS_GetFpclk
 * Parameters: Int32U Periphery
 * Return: Int32U
 *
 * Description: return Pclk [Hz]
 *
 *************************************************************************/
Int32U SYS_GetFpclk(Int32U Periphery)
{
Int32U Fpclk;
pInt32U pReg = (pInt32U)((Periphery < 32)?&PCLKSEL0:&PCLKSEL1);

  Periphery  &= 0x1F;   // %32
  Fpclk = SYS_GetFsclk();
  // find peripheral appropriate periphery divider
  switch((*pReg >> Periphery) & 3)
  {
  case 0:
    Fpclk /= 4;
    break;
  case 1:
    break;
  case 2:
    Fpclk /= 2;
    break;
  default:
    Fpclk /= 8;
  }
  return(Fpclk);
}

/*************************************************************************
 * Function Name: GpioInit
 * Parameters: void
 * Return: void
 *
 * Description: Reset all GPIO pins to default: primary function
 *
 *************************************************************************/
void GpioInit(void)
{
  // Set to inputs
  IO0DIR  = 0;
  IO1DIR  = 0;
  FIO0DIR = 0;
  FIO1DIR = 0;
  FIO2DIR = 0;
  FIO3DIR = 0;
  FIO4DIR = 0;

  // Enable Fast GPIO0,1
  SCS_bit.GPIOM = 1;

  // clear mask registers
  FIO0MASK = 0;
  FIO1MASK = 0;
  FIO2MASK = 0;
  FIO3MASK = 0;
  FIO4MASK = 0;

#ifndef SDRAM_DEBUG
  // Reset all GPIO pins to default primary function
  PINSEL0 = 0;
  PINSEL1 = 0;
  PINSEL2 = 0;
  PINSEL3 = 0;
  PINSEL4 = 0;
  PINSEL5 = 0;
  PINSEL6 = 0;
  PINSEL7 = 0;
  PINSEL8 = 0;
  PINSEL9 = 0;
#endif
}

