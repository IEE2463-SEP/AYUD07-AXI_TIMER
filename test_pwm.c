/***************** Libraries *****************************/
#include "xtmrctr.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xintc.h"
#include <stdio.h>

/***************** Macros ********************************/
#define TMRCTR_DEVICE_ID        XPAR_TMRCTR_0_DEVICE_ID
#define TMRCTR_INTERRUPT_ID     XPAR_INTC_0_TMRCTR_0_VEC_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define INTC                    XIntc
#define INTC_HANDLER            XIntc_InterruptHandler

#define PWM_PERIOD              500000000    // PWM period (10 ms)
#define TMRCTR_0                0            // Timer 0 ID
#define TMRCTR_1                1            // Timer 1 ID
#define CYCLE_PER_DUTYCYCLE     10           // Clock cycles per duty cycle
#define MAX_DUTYCYCLE           100          // Max duty cycle
#define DUTYCYCLE_DIVISOR       4            // Duty cycle Divisor
#define WAIT_COUNT              PWM_PERIOD   // Interrupt wait counter

/***************** Function declaration *******************/
void TmrCtrPwmExample(INTC *IntcInstancePtr, XTmrCtr *InstancePtr, u16 DeviceId, u16 IntrId);

void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);

void TmrCtrSetupIntrSystem(INTC *IntcInstancePtr, XTmrCtr *InstancePtr, u16 DeviceId, u16 IntrId);

void TmrCtrDisableIntr(INTC *IntcInstancePtr, u16 IntrId);

/***************** IPs Instantiation **********************/
INTC InterruptController;
XTmrCtr TimerCounterInst;

/***************** Variables ******************************/
static int   PeriodTimerHit = FALSE;
static int   HighTimerHit   = FALSE;

/***************** Main ***********************************/
int main(void)
{
	TmrCtrPwmExample(&InterruptController, &TimerCounterInst, TMRCTR_DEVICE_ID, TMRCTR_INTERRUPT_ID);
}

/***************** Function Definition ********************/
void TmrCtrPwmExample(INTC *IntcInstancePtr, XTmrCtr *TmrCtrInstancePtr,
						u16 DeviceId, u16 IntrId)
{
	u8  DutyCycle;
	u8  NoOfCycles;
	u8  Div;
	u32 Period;
	u32 HighTime;
	u64 WaitCount;

	XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	TmrCtrSetupIntrSystem(IntcInstancePtr, TmrCtrInstancePtr, DeviceId, IntrId);
	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler, TmrCtrInstancePtr);

	/* Enable the interrupt of the timer counter */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TMRCTR_0, XTC_INT_MODE_OPTION);
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TMRCTR_1, XTC_INT_MODE_OPTION);

	/* We start with fixed divisor (6) and after every CYCLE_PER_DUTYCYCLE (250)
	 * clock cycles divisor is decremented by 1, resulting in an increased Duty cycle */
	/* This is done until duty cycle reaches MAX_DUTYCYCLE */
	Div = DUTYCYCLE_DIVISOR;

	/* Configure PWM */
	do {
		if (!Div) { // Fail check for 0 divisor
			XTmrCtr_PwmDisable(TmrCtrInstancePtr);        // Disable PWM
			TmrCtrDisableIntr(IntcInstancePtr, DeviceId); // Disable interrupts
		}

		XTmrCtr_PwmDisable(TmrCtrInstancePtr); // Disable PWM for reconfiguration

		// PWM Reconfiguration
		Period    = PWM_PERIOD;
		HighTime  = PWM_PERIOD / Div--;
		DutyCycle = XTmrCtr_PwmConfigure(TmrCtrInstancePtr, Period, HighTime);

		XTmrCtr_PwmEnable(TmrCtrInstancePtr);

		NoOfCycles = 0;
		WaitCount = WAIT_COUNT;

		while (NoOfCycles < CYCLE_PER_DUTYCYCLE) {
			if (PeriodTimerHit == TRUE && HighTimerHit == TRUE) {
				PeriodTimerHit = FALSE;
				HighTimerHit = FALSE;
				WaitCount = WAIT_COUNT;
				NoOfCycles++;
			}
		}

	} while (DutyCycle < MAX_DUTYCYCLE);
}

/***********************************************************/
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	if (TmrCtrNumber == TMRCTR_0) { // Mark if period timer expired
		PeriodTimerHit = TRUE;
	}
	if (TmrCtrNumber == TMRCTR_1) { // Mark if high-time timer expired
		HighTimerHit = TRUE;
	}
}

/***********************************************************/
void TmrCtrSetupIntrSystem(INTC *IntcInstancePtr,
			XTmrCtr *TmrCtrInstancePtr, u16 DeviceId, u16 IntrId)
{
	XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);

	XIntc_Connect(IntcInstancePtr, IntrId,
				 (XInterruptHandler)XTmrCtr_InterruptHandler,
				 (void *)TmrCtrInstancePtr);

	XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	XIntc_Enable(IntcInstancePtr, IntrId);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			                    (Xil_ExceptionHandler)INTC_HANDLER,
								IntcInstancePtr);
	Xil_ExceptionEnable();
}

/***********************************************************/
void TmrCtrDisableIntr(INTC *IntcInstancePtr, u16 IntrId)
{
	XIntc_Disconnect(IntcInstancePtr, IntrId); // Disconnect interrupt
}
