// TI File $Revision: /main/3 $
// Checkin $Date: January 4, 2011   10:07:40 $
//###########################################################################
//
// FILE:    F2806x_BootVars.h
//
// TITLE:   F2806x Boot Variable Definitions.
//
// NOTES:
//
//###########################################################################
// $TI Release: 2806x C/C++ Header Files and Peripheral Examples V1.00 $
// $Release Date: January 11, 2011 $
//###########################################################################

#ifndef F2806x_BOOT_VARS_H
#define F2806x_BOOT_VARS_H

#ifdef __cplusplus
extern "C" {
#endif



//---------------------------------------------------------------------------
// External Boot ROM variable definitions:
//
extern Uint16 EmuKey;
extern Uint16 EmuBMode;
extern Uint32 Flash_CPUScaleFactor;
extern void (*Flash_CallbackPtr) (void);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of F2806x_BOOT_VARS_H definition

//===========================================================================
// End of file.
//===========================================================================

