/***************** Libraries *****************************/
#include "xparameters.h"
#include "xtmrctr.h"
#include "xil_exception.h"
#include "xintc.h"
#include <stdio.h>
#include "xgpio.h"
#include "xintc.h"

/***************** Macros ********************************/
#define TMRCTR_DEVICE_ID	XPAR_TMRCTR_0_DEVICE_ID
#define TMRCTR_INTERRUPT_ID	XPAR_INTC_0_TMRCTR_0_VEC_ID

#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTC		        XIntc
#define INTC_HANDLER	    XIntc_InterruptHandler

#define TIMER_CNTR_0	    0
#define RESET_VALUE	        0xF0000000

/***************** Function declaration *******************/
void TmrCtrIntrExample(INTC    *IntcInstancePtr,
			           XTmrCtr *InstancePtr,
			           u16      DeviceId,
			           u16      IntrId,
			           u8       TmrCtrNumber);

void TmrCtrSetupIntrSystem(INTC    *IntcInstancePtr,
				           XTmrCtr *InstancePtr,
				           u16      DeviceId,
				           u16      IntrId,
				           u8       TmrCtrNumber);

void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);

/***************** IPs Instantiation **********************/
INTC     InterruptController;
XTmrCtr  TimerCounterInst;
XGpio    GPIO;

/***************** Main ***********************************/
int main(void){

	XGpio_Initialize(&GPIO, XPAR_GPIO_0_DEVICE_ID);

	TmrCtrIntrExample(&InterruptController,
				      &TimerCounterInst,
				      TMRCTR_DEVICE_ID,
				      TMRCTR_INTERRUPT_ID,
				      TIMER_CNTR_0);
}

/***************** Function Definition ********************/
void TmrCtrIntrExample(INTC    *IntcInstancePtr,
			           XTmrCtr *TmrCtrInstancePtr,
			           u16      DeviceId,
			           u16      IntrId,
			           u8       TmrCtrNumber)
{
	XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);

	TmrCtrSetupIntrSystem(IntcInstancePtr,
					      TmrCtrInstancePtr,
					      DeviceId,
					      IntrId,
					      TmrCtrNumber);

	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler, TmrCtrInstancePtr);

	XTmrCtr_SetOptions(TmrCtrInstancePtr, TmrCtrNumber, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	XTmrCtr_SetResetValue(TmrCtrInstancePtr, TmrCtrNumber, RESET_VALUE);

	XTmrCtr_Start(TmrCtrInstancePtr, TmrCtrNumber);

	while (1) {}
}


/***********************************************************/
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	XTmrCtr *InstancePtr = (XTmrCtr *)CallBackRef;

	/*
	 * Check if the timer counter has expired, checking is not necessary
	 * since that's the reason this function is executed, this just shows
	 * how the callback reference can be used as a pointer to the instance
	 * of the timer counter that expired, increment a shared variable so
	 * the main thread of execution can see the timer expired
	 */
	if (XTmrCtr_IsExpired(InstancePtr, TmrCtrNumber)) {

		XGpio_DiscreteWrite(&GPIO, 1, 0xFFFFFFFF);
		usleep(500000);
		XGpio_DiscreteWrite(&GPIO, 1, 0x00000000);
	}
}

/***********************************************************/
void TmrCtrSetupIntrSystem(INTC    *IntcInstancePtr,
				           XTmrCtr *TmrCtrInstancePtr,
				           u16      DeviceId,
				           u16      IntrId,
				           u8       TmrCtrNumber)
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
