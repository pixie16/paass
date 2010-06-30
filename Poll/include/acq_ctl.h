/*****************************************************************************
*
*                              Physics Division
*                       Oak Ridge National Laboratory
*
*                          Copyright(C) 1992-2001
*
*
*
*                         IMPORTANT NOTICE TO USER
*
*     All statements, technical information and recommendations contained
*     herein are based on tests we believe to be reliable, but the accuracy
*     or completeness thereof is not guaranteed, and the following  is
*     made in lieu of all warranties expressed or implied:
*
*     Our only obligation shall be to correct or replace, at our convenience,
*     any part of the product proven to be defective.  We shall not be liable
*     for any loss or damage, direct or consequential, arising out of the
*     use or the inability to use the product.  Before using, the USER shall
*     determine the suitability of the product for his or her intended use,
*     and the USER assumes all risk and liability whatsoever in connection
*     therewith.
*
******************************************************************************
*
*    Environment:  VME based Data Acquisition System
*
*    File:         /usr/users/mcsq/Dvme3/acq_ctl.h
*
*    Description:  Command codes for control of the acquisition task in
*                  VME processor.  Possible error status codes are listed.
******************************************************************************
*
*    Revision History:
*
*    Date       Programmer   Comments
*
*    8/31/92    MCSQ         Original
*
*   10/30/92    MCSQ         Added status request command and status
*                            return codes.
*
*   12/30/92    MCSQ         Added error code for non-existant hardware
*                            modules.
*
*    4/23/94    MCSQ         Added command to request PAC file name.
*
*    7/21/94    MCSQ         Changed ACQ_NO_CES8170 to ACQ_NO_FERA because
*                            this code may be due to no CES8170 and no
*                            LRS 1190 available.
*
*    2/23/95    MCSQ         Add error code ACQ_SEQ_BUFFERS.  Error occurs
*                            when the number of list sequencer commands
*                            exceed the buffer space available in VMEacq.
*
*    2/27/96    MCSQ         Added error codes for new PAC features.
*                            i.e. count down, gates and special CAMAC
*                            modules.
*
*    8/29/96    MCSQ        Add command to get host Ethernet address.
*
*   10/ 1/96    MCSQ        Add error code for illegal CAMAC module type
*                           in a Gate specification.
*
*    9/17/98    MCSQ        Add codes for illegal crate number.
*
*    8/12/99    MCSQ        Add error codes for XIA stuff.
*
*    7/11/01    MCSQ        Add error codes for CAEN ADCs and TDCs
*
*    1/21/04    MCSQ        Add command to zero the VME acquisition clock.
*****************************************************************************/
#ifndef  ACQ_CTL_H_
#define  ACQ_CTL_H_

/*
*    Command codes to the VME processor task.
*/
#define INIT_ACQ   0x11    /* Initialize the run-time acquisition system */
#define START_ACQ  0x22    /* Start data acquisition                     */
#define STOP_ACQ   0x33    /* Stop data acquisition                      */
#define STATUS_ACQ 0x44    /* Get acquisition status                     */
#define PAC_FILE   0x55    /* Get PAC file name                          */
#define HOST       0x66    /* Get host Ethernet address                  */
#define ZERO_CLK   0x77    /* Zero the acquisition VME clock             */


/*
*   Acquisition status codes
*/
#define ACQ_RUN     1            /* Acquisition running                  */
#define ACQ_STOP    2            /* Acquisition stopped                  */
#define ACQ_UNINIT  3            /* Acquisition not initialized          */

/*
*    Returned status codes.
*/
#define ACQ_OK  0                /* All went well                        */

/*
*    Error status codes
*/
#define ACQ_UNKNOWN_COMMAND (-1)

#define ACQ_STP_HALT      (-5)   /* Acquisition already stopped          */
#define ACQ_STR_RUN       (-4)   /* Acquisition already running          */
#define ACQ_STR_NOINIT    (-3)   /* Start failed. System has not been
                                    initialized.                         */
#define ACQ_INIT_RUN      (-6)   /* Init error - acquisition is running  */
#define ACQ_INVALID_TABLE (-2)   /* Invalid Acq table index              */
#define ACQ_CAM_NOEXIST   (-10)  /* nonexistant crate                    */
#define ACQ_CAM_OFFLINE   (-11)  /* Crate off-line                       */
#define ACQ_CAM_INHIBIT   (-12)  /* Crate Inhibit stuck                  */
#define ACQ_CAM_INIT      (-13)  /* Initialization list CAMAC timeout    */
#define ACQ_FB_NOEXIST    (-20)  /* FASTBUS module does not respond      */
#define ACQ_FB_UNKNOWN    (-21)  /* Unknown FASTBUS module type          */
#define ACQ_FERA_UNKNOWN  (-30)  /* Unknown FERA module type             */
#define ACQ_FERA_INIT     (-31)  /* CAMAC timeout initializing FERA module */
#define ACQ_NO_KSC2917    (-40)  /* No CAMAC interface present           */
#define ACQ_NO_LRS1131    (-41)  /* No VME FASTBUS interface present     */
#define ACQ_NO_LRS1821    (-42)  /* No FASTBUS 1821 sequencer present    */
#define ACQ_NO_ACROMAG    (-43)  /* No Acromag digital I/O present       */
#define ACQ_NO_FERA       (-44)  /* No FERA  interface present           */
#define ACQ_NO_TRIGGER    (-45)  /* No ORNL trigger module present       */
#define ACQ_NO_KSC3982    (-46)  /* No List Sequencer present            */
#define ACQ_SEQ_BUFFERS   (-47)  /* Sequencer buffers are too small      */

#define ACQ_COUNT_DWN     (-48)  /* Count down list too large            */
#define ACQ_RAW_GATE      (-49)  /* Raw gate list too large              */
#define ACQ_CAL_GATE      (-50)  /* Calculated gate list too large       */
#define ACQ_CAM_RO        (-51)  /* Special CAMAC readout list too large */
#define ACQ_CAM_UNKNOWN   (-52)  /* Unknown CAMAC module type            */
#define ACQ_GATE_MODTYPE  (-53)  /* Illegal module type in gate spec.    */

#define ACQ_CR_COND       (-54)  /* Illegal crate in conditional list    */
#define ACQ_CR_UNCOND     (-55)  /* Illegal crate in unconditional list  */
#define ACQ_CR_LATCH      (-56)  /* Illegal crate in latch list          */
#define ACQ_CR_WIND       (-57)  /* Illegal crate in windup list         */
#define ACQ_CR_GATE       (-58)  /* Illegal crate in gate read list      */
#define ACQ_CR_CAMAC      (-59)  /* Illegal crate in CAMAC special modu  */
#define ACQ_CR_RUN        (-60)  /* Illegal crate in $run list           */

#define ACQ_NO_AUX        (-61)  /* No ORNL AUX found                    */
#define ACQ_NO_XIA        (-62)  /* No XIA modules found                 */
#define ACQ_NO_CAEN_ADC   (-63)  /* CAEN ADC not present in VME crate    */
#define ACQ_NO_CAEN_TDC   (-64)  /* CAEN ADC not present in VME crate    */
#endif       /* end  ACQ_CTL_H_    */
