//------------------------------------------------------------------------------------
//	FILE:	AsymmetricPWM-Main.c
//			(Up, Single Edge Asymmetric Waveform, With Independent Modulation 
//			on EPWM2A and EPWM2B)
//
//	Description:	This program sets up the EV TIMER2 to generate complimentary 
//			PWM waveforms. The user can then observe the waveforms using an scope from 
//			ePWM2A and ePWM2B pins. 
//			- In order to change the PWM frequency, the user should change
//			 the value of"period". 
//			- The duty-cycles can independently be adjusted by changing compare 
//			values (duty_cycle_A & duty_cycle_B) for ePWM2A and ePWM2B.  
//			- For further details, please search for the SPRU791.PDF 
//			(TMS320x28xx, 28xxx Enhanced Pulse Width Modulator Module) at ti.com
//
//  Target: TMS320F2806x or TMS320F2803x families (F28069)
//
//------------------------------------------------------------------------------------
//  $TI Release:$ 	V1.0
//  $Release Date:$ 11 Jan 2010 - VSC
//------------------------------------------------------------------------------------
//
// PLEASE READ - Useful notes about this Project

// Although this project is made up of several files, the most important ones are:
//	 "AsymmetricPWM .c",	this file
//		- Application Initialization, Peripheral config
//		- Application management
//		- Slower background code loops and Task scheduling
//	 "AsymmetricPWM-DevInit_F28xxx.c"
//		- Device Initialization, e.g. Clock, PLL, WD, GPIO mapping
//		- Peripheral clock enables
// The other files are generally used for support and defining the registers as C
// structs. In general these files will not need to be changed.
//   "F2806x_RAM_AsymmetricPWM.CMD" or "F2806x_FLASH_AsymmetricPWM.CMD"
//		- Allocates the program and data spaces into the device's memory map.
//   "F2806x_Headers_nonBIOS.cmd" and "F2806x_GlobalVariableDefs.c"
//		- Allocate the register structs into data memory.  These register structs are
//		  defined in the peripheral header includes (F2806x_Adc.h, ...) 
//
//----------------------------------------------------------------------------------

#include "PeripheralHeaderIncludes.h"
#include "F2806x_EPwm_defines.h" 	    // useful defines for initialization
																		 

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// FUNCTION PROTOTYPES
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void DeviceInit(void);
void InitFlash(void);
void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr);
interrupt void MainISR(void);

//NTM
extern void DSP28x_usDelay(Uint32 Count);
#define CPU_RATE   12.500L //this is 80mhz in nanoseconds
#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_RATE) - 9.0L) / 5.0L)

//
#define noWork 0
#define startCcdRead 10
#define startIntegration 20
#define endIntegration 30
#define getElements 40

//TCD1304AP Signalling delay conditions and configuration of data readout  
#define gateClearMinDelay 0.1 //100 nanoseconds, max 1000
#define minimumIntegrationTime 2000 //nanoseconds, [SH]utter line @ 50% duty in relation to [ICG] line
#define minimumElectronicShutterIntegrationTime 10000 //nanoseconds, or 10 microseconds
#define dataRate 4 //microsecond, 4X slower than the master clock of 1 MHZ, or 250 KHZ
#define numPixelElements 3694 //data readout cycles
#define numDummyPixels 16 //junk data, CCD warming up or something
#define numShieldedPixels 13 //use these for dark current subtraction
#define numTransitionPixels 3 //more junk probably due to diffraction as you transition
			    //from the shielded pixels to the unshielded ones
#define numSignalPixels 3648
#define numEndPixels 14

#define useElectronicShutter 1
#define useManualShutter 0

int adc_readings[3694];
int electronicShutter = useElectronicShutter;

int state = 10;
int integrationTimer = 0;

int exposureTime = 10; //in multiples of 2 microseconds, we'll fix it to be in increments of 1 later
int exposureTimeHalf=0; //set just before endless for loop below
int dataTicker = 0;
int pixelTicker = 0;



int elementTicker=0;
int aa=0;
int a =0;
int aT=0;
int b = 0;
int exposureTicker=0;

//NTM

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// VARIABLE DECLARATIONS - GENERAL
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// Used for running BackGround in flash and the ISR in RAM
extern Uint16 RamfuncsLoadStart, RamfuncsLoadEnd, RamfuncsRunStart;
Uint16 duty_cycle_A=40;	// Set duty 50% initially
Uint16 duty_cycle_B=40;	// Set duty 50% initially
Uint32 ISR_ticker = 0;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// MAIN CODE - starts here
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void main(void)
{
Uint32 i =0;
//=================================
//	INITIALISATION - General
//=================================

	DeviceInit();	// Device Life support & GPIO mux settings

// Only used if running from FLASH
// Note that the variable FLASH is defined by the compiler (-d FLASH)
#ifdef FLASH		
// Copy time critical code and Flash setup code to RAM
// The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
// symbols are created by the linker. Refer to the linker files. 
	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
	InitFlash();	// Call the flash wrapper init function
#endif //(FLASH)


//4 CMPA 200
//2 CMPA 79
//4 PRD 319
//2 PRD 159
//4 DBRED 29

//-------------------------------------------------------------

#define period 79							  // 80kHz when PLL is set to 0x10 (80MHz)

	EPwm1Regs.TBPRD = 1+period*2;       		   // Set timer period, PWM frequency = 1 / period
   	EPwm1Regs.TBPHS.all = 0;				   // Time-Base Phase Register
   	EPwm1Regs.TBCTR = 0;					   // Time-Base Counter Register
    EPwm1Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;  // Set Immediate load
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count-up mode: used for asymmetric PWM
	EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;	   // Disable phase loading
	EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
	EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
   	// Setup shadow register load on ZERO
   	EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   	EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
   	EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;	// load on CTR=Zero
   	EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;	// load on CTR=Zero

   	EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
   	EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_LO;
   	EPwm1Regs.DBFED = 0;
   	EPwm1Regs.DBRED = 0;
   	// Set Compare values
   	EPwm1Regs.CMPA.half.CMPA = duty_cycle_A;    // Set duty 50% initially
   	//EPwm1Regs.CMPB = duty_cycle_B;	            // Set duty 50% initially
   	// Set actions

   	EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM2A on Zero
   	EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Clear PWM2A on event A, up count


  //    period 500   			    	      // 160kHz when PLL is set to 0x10 (80MHz)
	EPwm4Regs.TBPRD = 1+period*2;       		   // Set timer period, PWM frequency = 1 / period
   	EPwm4Regs.TBPHS.all = 0;				   // Time-Base Phase Register
   	EPwm4Regs.TBCTR = 0;					   // Time-Base Counter Register	
    EPwm4Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;  // Set Immediate load
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count-up mode: used for asymmetric PWM
	EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;	   // Disable phase loading
	EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
	EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
	EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;
   	// Setup shadow register load on ZERO
   	EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   	EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
   	EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;	// load on CTR=Zero
   	EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;	// load on CTR=Zero
   	
   	EPwm4Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
   	EPwm4Regs.DBCTL.bit.POLSEL = DB_ACTV_LO;
   	EPwm4Regs.DBFED = 0;
   	EPwm4Regs.DBRED = 0;
   	// Set Compare values
   	EPwm4Regs.CMPA.half.CMPA = duty_cycle_A;    // Set duty 50% initially
   	//EPwm4Regs.CMPB = duty_cycle_B;	            // Set duty 50% initially
   	// Set actions
   	
   	EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM2A on Zero
   	EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Clear PWM2A on event A, up count
   	
   	//EPwm4Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Set PWM2A on Zero
   	//EPwm4Regs.AQCTLA.bit.CAU = AQ_SET;          // Clear PWM2A on event A, up count
   	//EPwm4Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // Set PWM2B on Zero
   	//EPwm4Regs.AQCTLB.bit.CBU = AQ_SET;            // Clear PWM2B on event B, up count

  // Time-base registers

   	EPwm2Regs.TBPRD = period;       		   // Set timer period, PWM frequency = 1 / period
   	EPwm2Regs.TBPHS.all = 0;				   // Time-Base Phase Register
   	EPwm2Regs.TBCTR = 0;					   // Time-Base Counter Register	
    EPwm2Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;  // Set Immediate load
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count-up mode: used for asymmetric PWM
	EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;	   // Disable phase loading
	EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
	EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;

   	// Setup shadow register load on ZERO

   	EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
   	EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
   	EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;	// load on CTR=Zero
   	EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;	// load on CTR=Zero

   	// Set Compare values

   	EPwm2Regs.CMPA.half.CMPA = duty_cycle_A;    // Set duty 50% initially
   	EPwm2Regs.CMPB = duty_cycle_B;	            // Set duty 50% initially

   	// Set actions

   	EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM2A on Zero
   	EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Clear PWM2A on event A, up count

   	EPwm2Regs.AQCTLB.bit.ZRO = AQ_CLEAR;          // Set PWM2B on Zero
   	EPwm2Regs.AQCTLB.bit.CBU = AQ_SET;            // Clear PWM2B on event B, up count

   	// configure ADC to sample on EPWM2A compare match
   	// ADC INITIALISATION
   	    EALLOW;
   	   	AdcRegs.ADCCTL1.bit.ADCBGPWD	= 1;	// Power up band gap
   	   	AdcRegs.ADCCTL1.bit.ADCREFPWD	= 1;	// Power up reference
   	   	AdcRegs.ADCCTL1.bit.ADCPWDN 	= 1;	// Power up rest of ADC
   		AdcRegs.ADCCTL1.bit.ADCENABLE	= 1;	// set timming to 13cycles as required by ADC module
   	    for(i=0; i<5000; i++){}					// wait 60000 cycles = 1ms (each iteration is 12 cycles)

   		AdcRegs.ADCCTL1.bit.INTPULSEPOS	= 1;	// create int pulses 1 cycle prior to output latch

   		// Configure ADC
   		AdcRegs.ADCSOC0CTL.bit.CHSEL 	= 0;	//set SOC0 channel select to ADCINA2
   		AdcRegs.ADCSOC0CTL.bit.TRIGSEL 	= 0x07;	//set SOC0 start trigger on EPWM2A, due to round-robin SOC0 converts first then SOC1
   		AdcRegs.ADCSOC0CTL.bit.ACQPS 	= 8;	//set SOC0 S/H Window to 8 ADC Clock Cycles, (8 ACQPS plus 1)
   		EDIS;

   		// Reassign ISRs.

   			EALLOW;	// This is needed to write to EALLOW protected registers
   			PieVectTable.EPWM2_INT = &MainISR;
   			EDIS;

   		// Enable PIE group 3 interrupt 2 for EPWM2_INT
   		    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;

			EPwm2Regs.ETSEL.bit.SOCAEN = 1;
			EPwm2Regs.ETSEL.bit.SOCASEL = 1;
			EPwm2Regs.ETPS.bit.SOCAPRD = 3;

   		// Enable CNT_zero interrupt using EPWM1 Time-base
   		    EPwm2Regs.ETSEL.bit.INTEN = 1;   // Enable EPWM1INT generation
   		    EPwm2Regs.ETSEL.bit.INTSEL = 4;  // Enable interrupt CNT=COMPA when incrementing event
   		    EPwm2Regs.ETPS.bit.INTPRD = 1;   // Generate interrupt on the 1st event
   			EPwm2Regs.ETCLR.bit.INT = 1;     // Enable more interrupts

   		// Enable CPU INT3 for EPWM1_INT:
   			IER |= M_INT3;
   		// Enable global Interrupts and higher priority real-time debug events:
   			EINT;   // Enable Global interrupt INTM
   			ERTM;	// Enable Global realtime interrupt DBGM


  //=================================
  //	Forever LOOP
  //=================================
  // Just sit and loop forever:
  // PWM pins can be observed with a scope.	
  // GPIO2 is the master clock, controlled by PWM at 1MHz
  // GPIO18 is a test line for syncing with an oscilloscope
  // GPIO01 is the SH line
  // GPIO19 is the ICG line

	exposureTimeHalf=exposureTime/2;
  	GpioDataRegs.GPADAT.bit.GPIO19 = 1; //pull the [ICG] line high, just so I know what state its in to begin with
		
  	for(;;)
	{
  		//whether starting fresh, or looping, ICG is set HIGH
  		if (state==startCcdRead) {
  			while (GpioDataRegs.GPADAT.bit.GPIO2==1){} // attempt to wait for a rising edge from the PWM master clock, not sure if this works correctly
  			GpioDataRegs.GPASET.bit.GPIO18 = 1; //test, goes high at beginning for triggering oscilloscope

  			//Prepare to get a CCD frame

  			GpioDataRegs.GPASET.bit.GPIO19 = 1; //pull the [ICG] line high to warm up things, maybe
			for (aa=0;aa<6;aa++){
				for (aT=0;aT<exposureTime*2;aT++){} //wait 50% duty
				GpioDataRegs.GPASET.bit.GPIO1 = 1; //pull the [SH] line high
  				for (aT=0;aT<exposureTime;aT++){} //wait 50% duty
  				GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; //pull the [SH] line low

			}
			for (aT=0;aT<(exposureTime*2)-1;aT++){} //wait almost 50% duty
  			//for (aT=0;aT<a;aT++){}
  			GpioDataRegs.GPACLEAR.bit.GPIO19 = 1; //pull the [ICG] line low to begin exposure process
  			GpioDataRegs.GPASET.bit.GPIO1 = 1; //electronic shutter [SH] line goes high
  			for (aT=0;aT<exposureTime;aT++){} //wait 50% duty
  			GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; //electronic shutter [SH] line goes low
  			for (aT=0;aT<exposureTime;aT++){} //wait 50% duty

  			GpioDataRegs.GPASET.bit.GPIO19 = 1; //pull the [ICG] line high to end exposure process
  			pixelTicker=0;
  			elementTicker=0;
  			state=getElements;
  			for (aT=0;aT<exposureTime-1;aT++){} //wait 50% duty
  			GpioDataRegs.GPASET.bit.GPIO1 = 1; //electronic shutter [SH] line goes high, continues to cycle
			for (aa=0;aa<numPixelElements;aa++){
  				for (aT=0;aT<exposureTime;aT++){} //wait 50% duty
  				GpioDataRegs.GPATOGGLE.bit.GPIO1 = 1; //toggle [SH] line
				for (aT=0;aT<exposureTime*2;aT++){} //wait 50% duty
				GpioDataRegs.GPATOGGLE.bit.GPIO1 = 1; //toggle [SH] line
			}
			state=startCcdRead;

  			GpioDataRegs.GPACLEAR.bit.GPIO18 = 1; //test, goes low at end for watching on oscilloscope

  		}

  		/* volatile long i;
 	while(ISR_ticker > 0)
	for(i = 0; i<10000; i++){}  // wait ~2.75ms
 	EPwm2Regs.CMPA.half.CMPA = period /2; 	 // start PWM back up
	*/
  		//GpioDataRegs.GPATOGGLE.bit.GPIO18 = 1;
  		//for (aT=0;aT<a;aT++){}
	}

	

}

//figure out how to set off the ISR just before the rising edge, then again just after the rising edge


// MainISR
interrupt void MainISR(void) {
	//GpioDataRegs.GPATOGGLE.bit.GPIO1 = 1;
/*
	for (aT=0;aT<a;aT++){}
	//Pulse the exposure time GPIO at 50% duty
	if (exposureTicker==(exposureTime/2)){// && (electronicShutter == useElectronicShutter)) {
		//goes high
		GpioDataRegs.GPATOGGLE.bit.GPIO1 = 1; //toggle electronic shutter at 50% duty
		exposureTicker++;
	} else if (exposureTicker==exposureTime){
		//goes low
		GpioDataRegs.GPATOGGLE.bit.GPIO1 = 1; //toggle electronic shutter at 50% duty
		exposureTicker=0;
	}else {
		exposureTicker++;
	}
	if (state==startCcdRead && exposureTicker == (exposureTime/2)-1 ) {
		//GpioDataRegs.GPACLEAR.bit.GPIO19 = 1; //pull the [ICG] line low to begin exposure process
		GpioDataRegs.GPACLEAR.bit.GPIO19 = 1; //pull the [ICG] line low to begin exposure process

		GpioDataRegs.GPASET.bit.GPIO18 = 1;//test

		//dataTicker=0;
		pixelTicker=0;
		integrationTimer=0;
		elementTicker=0;
		state = startIntegration;
	} else if (state == startIntegration) {
		if (integrationTimer>3 && exposureTicker == (exposureTime/2)+1) { //3 is a magic #, just to get the second check to wait a few cycles before being checked
			GpioDataRegs.GPASET.bit.GPIO19 = 1; //reset the [ICG] line
			state=getElements;
		}
		integrationTimer++;
	} else*/ if (state==getElements) {
		if (pixelTicker>=numPixelElements) {
			//we got a whole line of data
			state=noWork;
			//GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; //clear electronic shutter line
			GpioDataRegs.GPACLEAR.bit.GPIO18 = 1; //test line
			state=10;
		} else if (elementTicker==(dataRate/2) ) {
			//get ADC reading in the middle of every data element
			//adc_readings[((i-(dataRate/2))%dataRate)+(i/dataRate)] = AdcResult.ADCRESULT0;
			adc_readings[pixelTicker] = AdcResult.ADCRESULT0;
			elementTicker=0;
			pixelTicker++;
		} else {
			elementTicker++;
		}
	}
	/*
	if (dataTicker==exposureTime+(numPixelElements*dataRate)-b) {
		dataTicker=0;
	}else{*/
		//dataTicker++;
	//}
	EPwm2Regs.ETCLR.bit.INT = 1;
	// Acknowledge interrupt to recieve more interrupts from PIE group 3
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}
