/*==================================================================================*/
/*	User specific Linker command file for running from FLASH						*/
/*==================================================================================*/
/*	FILE:			F2806x_FLASH_AsymmetricPWM.CMD
/*                                                                                  */
/*	Description:	Linker command file for User custom sections targetted to run   */
/*					from FLASH.  			                                        */
/*                                                                                  */
/*  Target:  		TMS320F2806x					                                */
/*                                                                                  */
/*	Version: 		1.0                                 							*/
/*                                                                                  */
/*----------------------------------------------------------------------------------*/
/*  Copyright Texas Instruments © 2010                                			    */	
/*----------------------------------------------------------------------------------*/
/*  Revision History:                                                               */
/*----------------------------------------------------------------------------------*/
/*  Date	  | Description                                                         */
/*----------------------------------------------------------------------------------*/
/*  01/11/11  | Release 1.0  		 			                                    */
/*----------------------------------------------------------------------------------*/
 

 /* Define the memory block start/length for the F2806x  
   PAGE 0 will be used to organize program sections
   PAGE 1 will be used to organize data sections

   Notes: 
         Memory blocks on F2806x are uniform (ie same
         physical memory) in both PAGE 0 and PAGE 1.  
         That is the same memory region should not be
         defined for both PAGE 0 and PAGE 1.
         Doing so will result in corruption of program 
         and/or data. 
         
         The L0 memory block is mirrored - that is
         it can be accessed in high memory or low memory.
         For simplicity only one instance is used in this
         linker file. 
         
         Contiguous SARAM memory blocks or flash sectors can be
         be combined if required to create a larger memory block. 
*/
 
 MEMORY
{
PAGE 0:
   /* Program Memory */
   /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE1 for data allocation */

	BOOT_RSVD   : origin = 0x000000, length = 0x000050     /* Part of M0, BOOT rom will use this for stack */
	progRAM		: origin = 0x008000, length = 0x000C00
		
	OTP         : origin = 0x3D7800, length = 0x000400     /* on-chip OTP */
	FLASHH      : origin = 0x3D8000, length = 0x004000     /* on-chip FLASH */
   	FLASHG      : origin = 0x3DC000, length = 0x004000     /* on-chip FLASH */
   	FLASHF      : origin = 0x3E0000, length = 0x004000     /* on-chip FLASH */
   	FLASHE      : origin = 0x3E4000, length = 0x004000     /* on-chip FLASH */   
   	FLASHD      : origin = 0x3E8000, length = 0x004000     /* on-chip FLASH */
   	FLASHC      : origin = 0x3EC000, length = 0x004000     /* on-chip FLASH */
   	FLASHA      : origin = 0x3F4000, length = 0x003F80     /* on-chip FLASH */
	CSM_RSVD    : origin = 0x3F7F80, length = 0x000076     /* Part of FLASHA.  Program with all 0x0000 when CSM is in use. */
	BEGIN       : origin = 0x3F7FF6, length = 0x000002     /* Part of FLASHA.  Used for "boot to Flash" bootloader mode. */
	CSM_PWL     : origin = 0x3F7FF8, length = 0x000008     /* Part of FLASHA.  CSM password locations in FLASHA */
	
   	FPUTABLES   : origin = 0x3FD860, length = 0x0006A0	  /* FPU Tables in Boot ROM */
   	IQTABLES    : origin = 0x3FDF00, length = 0x000B50     /* IQ Math Tables in Boot ROM */
   	IQTABLES2   : origin = 0x3FEA50, length = 0x00008C     /* IQ Math Tables in Boot ROM */
   	IQTABLES3   : origin = 0x3FEADC, length = 0x0000AA	  /* IQ Math Tables in Boot ROM */

	BOOTROM     : origin = 0x3FF3B0, length = 0x000C10     /* Boot ROM */
	RESET       : origin = 0x3FFFC0, length = 0x000002     /* part of boot ROM  */
	VECTORS     : origin = 0x3FFFC2, length = 0x00003E     /* part of boot ROM  */

PAGE 1 : 

   /* Data Memory */
   /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE0 for program allocation */
   /* Registers remain on PAGE1 */

	RAMM0       : origin = 0x000050, length = 0x0003B0     /* on-chip RAM block M0 */
   	RAMM1       : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
	dataRAM		: origin = 0x008C00, length = 0x001400
   	RAML4       : origin = 0x00A000, length = 0x002000     /* on-chip RAM block L4 */
   	RAML5       : origin = 0x00C000, length = 0x002000     /* on-chip RAM block L5 */
   	RAML6       : origin = 0x00E000, length = 0x002000     /* on-chip RAM block L6 */
   	RAML7       : origin = 0x010000, length = 0x002000     /* on-chip RAM block L7 */
   	RAML8       : origin = 0x012000, length = 0x002000     /* on-chip RAM block L8 */ 
	FLASHB      : origin = 0x3F4000, length = 0x002000
}

 
 
SECTIONS
{
   /* Allocate program areas: */
   .cinit              	: > FLASHA,     PAGE = 0
   .pinit              	: > FLASHA,     PAGE = 0
   .text               	: > FLASHA,     PAGE = 0

   codestart           : > BEGIN       PAGE = 0
   ramfuncs            : LOAD = FLASHA, 
                         RUN = progRAM, 
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         PAGE = 0

   csmpasswds          : > CSM_PWL     PAGE = 0
   csm_rsvd            : > CSM_RSVD    PAGE = 0
   
   /* Allocate uninitalized data sections: */
   .stack           : > RAMM0,      PAGE = 1
   .ebss            : > dataRAM,    PAGE = 1
   .esysmem         : > dataRAM,      PAGE = 1

   /* Initalized sections go in Flash */
   /* For SDFlash to program these, they must be allocated to page 0 */
   .econst             : > FLASHA      PAGE = 0
   .switch             : > FLASHA      PAGE = 0      

   /* Allocate IQ math areas: */
   IQmath              : > FLASHA      PAGE = 0                  /* Math Code */
   IQmathTables        : > IQTABLES    PAGE = 0, TYPE = NOLOAD   /* Math Tables In ROM */

   /* Uncomment the section below if calling the IQNexp() or IQexp()
      functions from the IQMath.lib library in order to utilize the 
      relevant IQ Math table in Boot ROM (This saves space and Boot ROM 
      is 1 wait-state). If this section is not uncommented, IQmathTables2
      will be loaded into other memory (SARAM, Flash, etc.) and will take
      up space, but 0 wait-state is possible.
   */
   /*
   IQmathTables2    : > IQTABLES2, PAGE = 0, TYPE = NOLOAD 
   {
   
              IQmath.lib<IQNexpTable.obj> (IQmathTablesRam)
   
   }
   */
   /* Uncomment the section below if calling the IQNasin() or IQasin()
      functions from the IQMath.lib library in order to utilize the 
      relevant IQ Math table in Boot ROM (This saves space and Boot ROM 
      is 1 wait-state). If this section is not uncommented, IQmathTables2
      will be loaded into other memory (SARAM, Flash, etc.) and will take
      up space, but 0 wait-state is possible.
   */
   /*
   IQmathTables3    : > IQTABLES3, PAGE = 0, TYPE = NOLOAD 
   {
   
              IQmath.lib<IQNasinTable.obj> (IQmathTablesRam)
   
   }   
   */
 
   .reset              : > RESET,      PAGE = 0, TYPE = DSECT
   vectors             : > VECTORS     PAGE = 0, TYPE = DSECT

}
    
    
SECTIONS
{
	Net_terminals:	> dataRAM,PAGE = 1
}



 
