// TI File $Revision: /main/4 $
// Checkin $Date: January 4, 2011   10:08:39 $
//###########################################################################
//
// FILE:   F2806x_Comp.h
//
// TITLE:  F2806x Device COMP Register Definitions.
//
//###########################################################################
// $TI Release: 2806x C/C++ Header Files and Peripheral Examples V1.00 $   
// $Release Date: January 11, 2011 $   
//###########################################################################

#ifndef F2806x_COMP_H
#define F2806x_COMP_H

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------
// COMP Individual Register Bit Definitions:

struct COMPCTL_BITS {     // bits description
	Uint16	COMPDACEN:1;		// 0	Comparator/DAC Enable
	Uint16	COMPSOURCE:1;		// 1	Source Select for inverting input
	Uint16	CMPINV:1;			// 2	Invert Select for Comparator
	Uint16	QUALSEL:5;			// 7:3	Qualification Period for synchronized output
	Uint16	SYNCSEL:1;			// 8	Synchronization select before ETPWM/GPIO
	Uint16	rsvd1:7;			// 15:9	Reserved
};

union COMPCTL_REG {
	Uint16 all;
	struct COMPCTL_BITS	bit;
};

struct COMPSTS_BITS {     // bits description
	Uint16	COMPSTS:1;			// 0	Logical latched value of the comparator
	Uint16	rsvd1:15;			// 15:1	Reserved
};

union COMPSTS_REG {
	Uint16 all;
	struct COMPSTS_BITS	bit;
};

struct DACVAL_BITS {     // bits description
	Uint16	DACVAL:10;			// 9:0	DAC Value bits
	Uint16	rsvd1:6;			// 15:10	Reserved
};

union DACVAL_REG {
	Uint16 all;
	struct DACVAL_BITS	bit;
};

struct COMP_REGS {
	union	COMPCTL_REG	COMPCTL;			// Comparator Control
	Uint16				rsvd1;				// Reserved
	union	COMPSTS_REG	COMPSTS;			// Compare Output States
	Uint16				rsvd2[3];			// Reserved
	union	DACVAL_REG	DACVAL;				// 10-bit DAC Value
};

//---------------------------------------------------------------------------
// Comp External References & Function Declarations:
//
extern volatile struct COMP_REGS Comp1Regs;
extern volatile struct COMP_REGS Comp2Regs;
extern volatile struct COMP_REGS Comp3Regs;

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of F2806x_COMP_H definition
//===========================================================================
// End of file.
//===========================================================================
