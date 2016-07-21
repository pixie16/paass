#ifndef __PIXIE16APP_DEFS_H
#define __PIXIE16APP_DEFS_H

/*----------------------------------------------------------------------
 * Copyright (c) 2005 - 2009, XIA LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided 
 * that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above 
 *     copyright notice, this list of conditions and the 
 *     following disclaimer.
 *   * Redistributions in binary form must reproduce the 
 *     above copyright notice, this list of conditions and the 
 *     following disclaimer in the documentation and/or other 
 *     materials provided with the distribution.
 *   * Neither the name of XIA LLC nor the names of its
 *     contributors may be used to endorse or promote products
 *     derived from this software without specific prior
 *     written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *----------------------------------------------------------------------*/

/******************************************************************************
 *
 * File Name:
 *
 *		pixie16app_defs.h
 *
 * Description:
 *
 *		Constant definitions.
 *
 * $Rev: 13856 $
 * $Id: pixie16app_defs.h 13856 2009-11-20 23:03:35Z htan $
 ******************************************************************************/

/* If this is compiled by a C++ compiler, make it */
/* clear that these are C routines.               */
#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------- 
  Pixie16 hardware revisions
  -------------------------------------*/
#define PIXIE16_REVA        0
#define PIXIE16_REVB        1
#define PIXIE16_REVC_MSU    2
#define PIXIE16_REVC_GENERAL 3
#define PIXIE16_REVD_ITHEMBA 4
#define PIXIE16_REVD_GENERAL 5
// Changing PIXIE16_REVISION here affects the code globally
#define PIXIE16_REVISION    PIXIE16_REVD_GENERAL


/*------------------------------------- 
    Define special operation modes
	(normally requires a special firmware)
  -------------------------------------*/
#ifdef CAPTURE_SLOW_TRACE
	#undef CAPTURE_SLOW_TRACE
#endif
#ifdef MSU_SEGA_MODE
	#undef MSU_SEGA_MODE
#endif
#ifdef EXTENDED_FASTFILTER_LEN
	#undef EXTENDED_FASTFILTER_LEN
#endif
#ifdef ORNL_PSD
	#undef ORNL_PSD
#endif


/*------------------------------------- 
    At which platform to compile this code -
	Windows or Linux?
  -------------------------------------*/
#define PIXIE16_WINDOWS_APPAPI    0
#define PIXIE16_LINUX_APPAPI      1
// Changing PIXIE16_APPAPI_VER here affects the code globally
#define PIXIE16_APPAPI_VER        PIXIE16_LINUX_APPAPI	


/*------------------------------------- 
    Define EXPORT macro
  -------------------------------------*/
#if PIXIE16_APPAPI_VER == PIXIE16_WINDOWS_APPAPI
	#define PIXIE16APP_EXPORT __declspec(dllexport)
	#define PIXIE16APP_API _stdcall
#elif PIXIE16_APPAPI_VER == PIXIE16_LINUX_APPAPI
	#define PIXIE16APP_EXPORT  
	#define PIXIE16APP_API  
#endif


/*------------------------------------- 
    Define math constants
  -------------------------------------*/

#ifndef PI
	#define PI					3.14159265358979
#endif

#ifndef PI2
	#define PI2					6.28318530717959
#endif


/*-----------------------------------------------------------------
    size of system FPGA, trigger FPGA, Fippi, DSP parameters files
  -----------------------------------------------------------------*/

#if PIXIE16_REVISION == PIXIE16_REVA
#define N_COM_FPGA_CONF            58614      // size of communications FPGA configuration (32-bit word)
#define N_TRIG_FPGA_CONF           58614      // size of trigger FPGA configuration (32-bit word)
#define N_SP_FPGA_CONF            127581      // size of signal processing FPGA configuration (32-bit word)
#elif PIXIE16_REVISION == PIXIE16_REVB || PIXIE16_REVISION == PIXIE16_REVC_MSU || PIXIE16_REVISION == PIXIE16_REVC_GENERAL
#define N_COM_FPGA_CONF           162962      // size of communications FPGA configuration (32-bit word)
#define N_SP_FPGA_CONF            162962      // size of signal processing FPGA configuration (32-bit word)
#endif
#define N_DSP_PAR                   1280      // number of DSP parameters (32-bit word)
#define DSP_IO_BORDER                832      // number of DSP I/O variables


/*-----------------------------------------------------------------
    module specifications
  -----------------------------------------------------------------*/

#define PRESET_MAX_MODULES            24      // Preset maximum number of Pixie modules
#define NUMBER_OF_CHANNELS            16

#define SYSTEM_CLOCK_MHZ             100      // system (ADC and FPGA) clock frequency in MHz

#if PIXIE16_REVISION == PIXIE16_REVA
#define DSP_CLOCK_MHZ                 80      // DSP clock frequency in MHz
#elif PIXIE16_REVISION == PIXIE16_REVB || PIXIE16_REVISION == PIXIE16_REVC_MSU || PIXIE16_REVISION == PIXIE16_REVC_GENERAL || PIXIE16_REVISION == PIXIE16_REVD_ITHEMBA || PIXIE16_REVISION == PIXIE16_REVD_GENERAL
#define DSP_CLOCK_MHZ                100      // DSP clock frequency in MHz
#endif

#define DAC_VOLTAGE_RANGE            3.0      // Pixie-16 DAC range is -1.5 V to +1.5 V

#define MAX_ADC_TRACE_LEN           8192      // Maximum ADC trace length for a channel

/*-----------------------------------------------------------------
    run type
  -----------------------------------------------------------------*/

#define NEW_RUN                       1       // New data run
#define RESUME_RUN                    0       // Resume run

#define LIST_MODE_RUN0            0x100       // List mode run (chl=9, with traces)
#define LIST_MODE_RUN1            0x101       // List mode run (chl=9, no traces)
#define LIST_MODE_RUN2            0x102       // List mode run (chl=4, no traces)
#define LIST_MODE_RUN3            0x103       // List mode run (chl=2, no traces)
#define HISTOGRAM_RUN             0x301       // Histogram run


/*-----------------------------------------------------------------
    I/O mode
  -----------------------------------------------------------------*/

#define MOD_READ                      1       // Host read from modules
#define MOD_WRITE                     0       // Host write to modules


/*-----------------------------------------------------------------
    Data memory, buffer, histogram, and list mode data structure
  -----------------------------------------------------------------*/
	
#define DSP_IMBUFFER_START_ADDR   0x40000     // 32-bit wide
#define DSP_IMBUFFER_END_ADDR     0x5FFFF     // 32-bit wide

#define DSP_EMBUFFER_START_ADDR       0x0     // 32-bit wide
#if PIXIE16_REVISION == PIXIE16_REVA
#define DSP_EMBUFFER_END_ADDR     0xFFFFF     // 32-bit wide 
#else
#define DSP_EMBUFFER_END_ADDR     0x7FFFF     // 32-bit wide 
#endif

#if PIXIE16_REVISION == PIXIE16_REVA
#define EM_PINGPONGBUFA_ADDR      0x80000     // 32-bit wide 
#define EM_PINGPONGBUFB_ADDR      0xC0000     // 32-bit wide 
#endif

#define DATA_MEMORY_ADDRESS      0x4A000      // DSP data memory address
#define HISTOGRAM_MEMORY_ADDRESS     0x0      // histogram memory buffer in external memory   	
#define MAX_HISTOGRAM_LENGTH       32768      // Maximum MCA histogram length
#define IO_BUFFER_ADDRESS        0x50000      // Address of I/O output buffer
#define IO_BUFFER_LENGTH           65536      // Length of I/O output buffer
#define EXTERNAL_FIFO_LENGTH      131072      // Length of external FIFO

#define BUFFER_HEAD_LENGTH             6      // Output buffer header length
#define EVENT_HEAD_LENGTH              3      // Event header length
#define CHANNEL_HEAD_LENGTH            9      // Channel header length

#define EVENT_INFO_LENGTH             68      // Information length for each event
#define CHANNEL_INFO_LENGTH            4      // Information length for each channel
#define EVENT_INFO_HEADER_LENGTH       4      // Information length for each event header


/*------------------------------------- 
	Length limits for certain DSP parameters
 --------------------------------------*/ 

#ifdef EXTENDED_FASTFILTER_LEN
#define FASTFILTER_MAX_LEN 128
#else
#if (PIXIE16_REVISION == PIXIE16_REVD_ITHEMBA) || (PIXIE16_REVISION == PIXIE16_REVD_GENERAL)
#define FASTFILTER_MAX_LEN 64
#else
#define FASTFILTER_MAX_LEN 32
#endif
#endif
#define MIN_FASTLENGTH_LEN 1

#define SLOWFILTER_MAX_LEN 128
#define MIN_SLOWLENGTH_LEN 2
#define MIN_SLOWGAP_LEN 3

#ifdef EXTENDED_FASTFILTER_LEN
#define FAST_THRESHOLD_MAX 65536
#else
#define FAST_THRESHOLD_MAX 16384
#endif

#if (PIXIE16_REVISION == PIXIE16_REVD_ITHEMBA) || (PIXIE16_REVISION == PIXIE16_REVD_GENERAL)
#define CFDDELAY_MAX 63
#define CFDDELAY_MIN 1

#define EXTTRIGSTRETCH_MAX 4095
#define EXTTRIGSTRETCH_MIN 1

#define VETOSTRETCH_MAX 4095
#define VETOSTRETCH_MIN 1

#define FASTTRIGBACKLEN_MAX 4095
#define FASTTRIGBACKLEN_MIN 1

#define EXTDELAYLEN_MAX 255
#define EXTDELAYLEN_MIN 1

#define FASTTRIGBACKDELAY_MAX 127
#define FASTTRIGBACKDELAY_MIN 0

#define QDCLEN_MAX 32767
#define QDCLEN_MIN 1
#endif

/*------------------------------------- 
	CHANCSRA bits definitions
 --------------------------------------*/ 

#define CCSRA_GOOD         2    // Channel may be read, good-channel bit
#define CCSRA_POLARITY     5    // Control polarity: 1: negative, 0: positive
#if (VARIANT == REVD_GENERAL)
#define CCSRA_TRACEENA     8    // 1: enable trace capture and associated header data; 0: disable trace capture and associated header data
#define CCSRA_QDCENA       9    // 1: enable QDC summing and associated header data; 0: dsiable QDC summing and associated header data
#endif
#define CCSRA_ENARELAY    14    // Control input relay: 1: connect, 0: disconnect

 /*------------------------------------- 
	MODCSRB bits definitions
 --------------------------------------*/ 

#define MODCSRB_PULLUP        0  // Control pullup: 1: pulled up, 0: not pulled up
#define MODCSRB_BPCONNECTION  1  // Control BP_Connection: 1: connected, 0: disconnected
#define MODCSRB_MASTERMODULE  2  // Control Master/Worker module: 1: Master module, 0: Worker module
#define MODCSRB_FASTTRIGSRC   3  // Control fast trigger source in system FPGA: 1: Master module only, 0: all modules wired-or

#define MODCSRB_DIRMOD        4  // Control Director module: 1: Director module; 0: other modules 
#define MODCSRB_RECTRIGENA    5  // Enable Director module's record trigger (1) or disable it (0)
#define MODCSRB_CHASSISMASTER 6  // Control chassis master module: 1: chassis master module; 0: chassis non-master module 
#define MODCSRB_MWMOD         7  // Control Manger/Worker module: 1: Manger/Worker module; 0: other modules
#define MODCSRB_RIGHTASTMOD   8  // Control Right Assistant module: 1: Right Assistant module; 0: other modules
#define MODCSRB_LEFTASTMOD    9  // Control Left Assistant module: 1: Left Assistant module; 0: other modules
#define MODCSRB_INHIBITENA   10  // Control external INHIBIT signal: 1: use INHIBIT; 0: don't use INHIBIT
#define MODCSRB_MULTCRATES   11  // Distribute clock and triggers in multiple crates: multiple crates (1) or only single crate (0)

/*------------------------------------- 
	Control parameters
 --------------------------------------*/ 

#define MAX_PAR_NAME_LENGTH     65  // Maximum length of parameter names
#define RANDOMINDICES_LENGTH  8192  // number of random indices (currently only used for tau finder)
#define MAX_ERRMSG_LENGTH     1024  // Maximum length of error message

#define BASELINES_BLOCK_LEN     18  // Length of each baslines length (default: 2 timestamp words + 16 baselines)
#define MAX_NUM_BASELINES     3640  // Maximum number of baselines available after each baseline acquisition run

#define EXTFIFO_READ_THRESH   1024  // Reading out threshold for external FIFO watermmark level

#if (PIXIE16_REVISION == PIXIE16_REVD_ITHEMBA) || (PIXIE16_REVISION == PIXIE16_REVD_GENERAL)
#define PCI_STOPRUN_REGADDR   0x44  // PCI register address in the System FPGA for stopping run
#endif

/*------------------------------------- 
	Frequently used Control Tasks
 --------------------------------------*/ 

#define SET_DACS                      0       // Set DACs
#define ENABLE_INPUT                  1       // Enable detect signal input
#define RAMP_OFFSETDACS               3       // Ramp Offset DACs
#define GET_TRACES                    4       // Acquire ADC traces
#define PROGRAM_FIPPI                 5       // Program FIPPIs
#define GET_BASELINES                 6       // Get baselines
#define ADJUST_OFFSETS                7       // Adjust DC-offsets

#ifdef __cplusplus
}
#endif

#endif
