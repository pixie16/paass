/*****************************************************************************
*
*                              Physics Division
*                       Oak Ridge National Laboratory
*
*                          Copyright(C) 1992-2000
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
*    File:         /usr/users/mcsq/Dvme3/Acq_Params.h
*
*    Description:  Definitions of control tables for the VME based
*                  acquisition system.  
******************************************************************************
*
*    Revision History:
*
*    Date       Programmer   Comments
*
*    8/17/92             
*
*    8/27/92    MCSQ       Removed delay time parameter from the
*                          FASTBUS_RO and FERA_RO tables.
*
*    9/29/93    MCSQ       Revised for PAC Version 2.
*
*    4/28/94    MCSQ       Corrected PAC table indexes.
*
*    4/15/95    MCSQ       Added LRS_1875 and LRS_1881 FASTBUS modules.
*
*    2/20/96    MCSQ       Added CAMAC module type codes
*
*    2/25/96    MCSQ       Added structs for new PAC features which MIlner
*                          added many moons ago.  Includes gates, count_down
*                          and special CAMAC modules.
*
*    9/19/96    MCSQ       Add some special module defs.
*
*    9/30/96    MCSQ       Add Start acquisition CNAF list.  Remove
*                          Acromag trigger module definitions.
*
*    9/17/98    MCSQ       Add Clover module type - BAKLASH
*
*    2/24/00    MCSQ       Add XIA_TIME module type and XIA readout
*
*    6/26/00	JWK        Add codes for Ortec AD413 ADC
*
*    8/ 9/00    MCSQ       Add struct and pointer for XIA parameter
*                          storage in the Acq_Params memory segment.
*
*    7/10/01    MCSQ       Add CAEN ADC & TDC VNE modules
*
*   11/03/03    MCSQ       Increase NUM_XIA to 25
*
*    1/21/04    MCSQ       Add VME 100Hz clock
*****************************************************************************/

#ifndef  ACQ_PARAMS_H_
#define  ACQ_PARAMS_H_

#ifndef  ORPH_UDP_H_
#include "orph_udp.h"
#endif

/*
*    Structure of the index to Data Acquisition Parameter tables.
*    This index is an array of type struct tbl_index.
*/
struct tbl_index {
     int  offset;	/* long word offset from start of parameter ram  */
     int  length;	/* Number of table entries                       */
     int  delay;	/* Wait time after Event before execution        */
};

/*
*   Structure for tables which have only CAMAC module addresses.
*/
struct cnaf_list {
     unsigned char  c;
     unsigned char  n;
     unsigned char  a;
     unsigned char  f;
};

/*
*   Structure for tables which have CAMAC module addresses and data
*   for the CAMAC operation or an ID word.
*/
struct cnafdat_list {
     unsigned char  c;
     unsigned char  n;
     unsigned char  a;
     unsigned char  f;
              int   d;
};

/*
*   Structure for XIA modules.
*/
struct xia_list {
     unsigned char  c;
     unsigned char  n;
     unsigned char  vsn;
     unsigned char  group;
};
/*
*   Structure for tables of Special CAMAC, FASTBUS and FERA modules.
*/
struct mod_type  {
     unsigned char  c;
     unsigned char  n;
     unsigned char  dum;
     unsigned char  type_code;
};

/*   FERA module type_codes        */
#define  LRS_4300    1
#define  GAN_812F    2
#define  SILENA_4418 3
#define  MCSQ_FER    4       /* for testing software only */
#define  LRS_3377    5
#define  BAKLASH     6
#define  BAKLASH2    7
#define  BAKLASH3    8
/*      Added support for ORTEC AD413 module.   (JJK, Jan. 23, 1997) */
#define  AD_413    9
/*      End of added support for ORTEC AD413 module.   (JJK, Jan. 23, 1997) */

/*   FASTBUS module type_codes        */
#define  LRS_1885    11
#define  PHIL_10C6   12
#define  LRS_1875    13
#define  LRS_1881M   14
#define  LRS_1877    15
#define  MCSQ_FAS    16       /* for testing software only */

/*   CAMAC module type_codes        */
#define  PHIL_7164    21
#define  PHIL_7186    22
#define  LRS_2277     23
#define  SILENA_4418C 24
#define  MCSQ_CAM     25      /* for testing software only */
#define  LRS_4300C    26
#define  LRS_3377C    27
#define  XIA_TIME     28

/*      Added support for ORTEC AD413 module.   (JJK, Jan. 23, 1997) */
#define  AD_413C        29
/*      End of added support for ORTEC AD413 module.   (JJK, Jan. 23, 1997) */

/*   VME module type_codes          */
#define  CAEN_775     41      /* CAEN Model 775 TDC        */
#define  CAEN_785     42      /* CAEN Model 785 ADC        */

/*
*      Raw and computed gate specifications
*
*/
struct raw_gate_spec {
             int  rdindx;     /* index to the gate readout table       */
             int  low;        /* gate low limit                        */
             int  high;       /* gate high limit                       */
             int  pat;        /* index to pattern register             */
             int  mask;       /* bit mask for bit to set if true       */
};
struct gate_read_table {
             int  c;          /* CAMAC crate number                    */
             int  n;          /* slot number                           */
             int  a;          /* subaddress                            */
             int  f;          /* function code                         */
             int  type_code;  /* module type code, 0 means generic type */
};

/*
*     Count down specifications
*/
struct count_dwn_pgm{
             int  pat;        /* index to pattern register list        */
             int  mask;       /* bit mask for use with pattern reg     */
             int count;       /* Count down value                      */
};

/*
*     Conditional Kill event table
*
*   The Conditional Kill Event table is executed after the gated latches
*   are read and before any other readout occurs.  Therefore, if ANY
*   kill test is true, the complete event is discarded.
*
*   There are two types of Kill test.  The ANY test kills the event if
*   ANY of the masked bits in the latch data word is a ONE.  The NONE
*   test kills the event if ALL of the masked bits in the latch data
*   word are ZERO.
*/

struct cond_kill {
      int  lat;		/* index to latch data word                   */
      int  mask;	/* value of the mask                          */
      int  type;        /* type of test: 0 = NONE, and 1 = ANY        */
};

/*
*                   Conditional readout tables
*
*    Conditional Test nomenclature
*
*   The test is a 16 bitwise AND of the latch data word and the mask value.
*      result = latch .AND. mask
*
*   If the result is zero, the test is FALSE.  If the result is nonzero,
*   the test is TRUE.
*/
struct cond_cam_pgm {   /*  COND_RO   */
      int  lat;		/* index to latch data word                   */
      int  mask;	/* value of the mask                          */
      int  tr_index;	/* index to CNAF_LIST for TRUE (nonzero) test */
      int  tr_num;	/* Number of CNAFs to execute                 */
      int  tr_next;	/* index to next program test                 */
      int  fa_index;	/* index to CNAF_LIST for FALSE (zero) test   */
      int  fa_num;	/* Number of CNAFs to execute                 */
      int  fa_next;	/* index to next program test                 */
};

struct cond_fast_pgm {  /*  FASTBUS_RO   */
      int  lat;		/* index to latch data word                   */
      int  mask;	/* value of the mask                          */
      int  type_code;	/* module type to read  if test is TRUE       */
};

struct cond_fera_pgm {  /*  FERA_RO  */
      int  lat;		/* index to latch data word                   */
      int  mask;	/* value of the mask                          */
      int  type_code;	/* module type to read  if test is TRUE       */
};

struct cond_cam_special {  /*  CAMAC_RO  */
      int  lat;         /* index to latch data word                   */
      int  mask;        /* value of the mask                          */
      int  type_code;   /* module type to read  if test is TRUE       */
};

struct cond_vme_pgm {   /* VME_RO    */
      int  lat;         /* index to latch data word                   */
      int  mask;        /* value of the mask                          */
      int  type_code;   /* module type to read  if test is TRUE       */
};


/*
*    Data Acquisition Table Index.
*
*   We must agree on the order of index entries.   The offset variable
*   in each index entry specifies the start of the table data relative to
*   beginning of the index.
*!   
struct tbl_index  CRATES;		!* List of CAMAC crates             *!
struct tbl_index  CNAF_INIT;		!* initialization                   *!
struct tbl_index  FASTBUS_MODULES;	!* FASTBUS module table             *!
struct tbl_index  FERA_MODULES;		!* FERA module table                *!

struct tbl_index  LATCHES;		!* Gated latches                    *!
struct tbl_index  KILL_LIST;		!* Conditional kill event tests     *!

struct tbl_index  UNCOND_RO;		!* Unconditional readout list       *!
struct tbl_index  COND_RO;		!* Conditional readout program      *!
struct tbl_index  CNAF_LIST;		!* CNAF list for conditional readout *!
struct tbl_index  ID_LIST;		!* ID list for conditional readout  *!
struct tbl_index  FASTBUS_RO;		!* FASTBUS readout program          *!
struct tbl_index  FASTBUS_ID;		!* FASTBUS ID table                 *!
struct tbl_index  FERA_RO;		!* FERA readout program             *!
struct tbl_index  FERA_ID;		!* FERA ID table                    *!

struct tbl_index  WINDUP_CNAF;		!* Windup CNAF list                 *!
struct tbl_index  RUN_CNAF_LIST;	!* Start acquisition CNAF list      *!
*/
/*
*   Indexes to the parameter tables
*/
#define INDEX_FILENAME        0
#define INDEX_CRATES          1
#define INDEX_CNAF_INIT       2
#define INDEX_CAMAC_MODULES   3
#define INDEX_FASTBUS_MODULES 4
#define INDEX_FERA_MODULES    5
#define INDEX_LATCHES         6
#define INDEX_GATE_READ       7
#define INDEX_RAW_GATE_SPEC   8
#define INDEX_CAL_GATE_SPEC   9
#define INDEX_COUNT_DOWN      10
#define INDEX_KILL_LIST       11
#define INDEX_UNCOND_RO       12
#define INDEX_COND_RO         13
#define INDEX_CNAF_LIST       14
#define INDEX_ID_LIST         15
#define INDEX_CAMAC_RO        16
#define INDEX_FASTBUS_RO      17
#define INDEX_FERA_RO         18
#define INDEX_CAMAC_ID        19
#define INDEX_FASTBUS_ID      20
#define INDEX_FERA_ID         21
#define INDEX_WINDUP_CNAF     22
#define INDEX_RUN_CNAF_LIST   23
#define INDEX_XIA_LIST        24
#define INDEX_CAEN_RO         25
#define INDEX_CAEN_ADC_HARD   26
#define INDEX_CAEN_TDC_HARD   27
#define INDEX_CAEN_ADC_ID     28
#define INDEX_CAEN_TDC_ID     29
#define INDEX_CLK_ID          30


/*
*    Structure of memory segment used for intertask communication.
*/
struct acq_share {
       unsigned int  event_number;
                int  avg_param_size;
  struct sockaddr_in Host_Ether_Adr;
               short FB_enabled;
               short FB_error;
               short KSC3982_enabled;
               short acqrun;
                int  spare[250];
               short testrun;
  };

#define ACQ_MAX_CRATE  7
#define ACQ_MAX_FB_MOD 26

#define ACQ_MIN_DELAY  12           /* This is an estimate of the time delay
                                       from the event interrupt to start of
                                       actual processing of the event.  All
                                       delay times are reduced by this amount
                                       to account for the fixed overhead   */
#define ACQ_MAX_DELAY  255

#define LIST_SEQ_C  0   /* Crate with KSC3982 or negative for none  */
#define LIST_SEQ_N 23   /* Module number for the KSC3982            */

/*   Maximum number of XIA modules                                         */
#define NUM_XIA  25
/*
*    Data buffer structure and parameters
*/
#define  AVG_PARAMS       90
#define  MAX_PKT_DATA     (MAX_ORPH_DATA - 8)
#define  PKTS_PER_BUF     8
#define  MAX_PARAMS       700
#define  MAX_DATA_PARAMS  (8200 * (NUM_XIA+1))

/*
*    Data buffer descriptor
*/
struct acq_buf_hdr {
    unsigned short  *str_buf;   /* pointer to event data start in buffer    */
    unsigned short  *end_buf;   /* pointer to end of event data in buffer   */
    unsigned short  *last_event;   /* pointer to previous event data        */
               int  totalevents;   /* event number                          */
               int  events;     /* number of events in this buffer          */
               int  ack;        /* buffer acknowledge - zero = acked        */
               int  busy;       /* buffer busy flag - negative = Busy       */
} ;

struct Packet_Header {
         int  Sequence;
         int  DataSize;
};

struct data_buf {
     struct acq_buf_hdr  Bufhdr;        /* information on buffer content    */
    struct Packet_Header Head;          /* Ether packet header              */
                    int  TotalEvents;   /* event number of first pkt event  */
         unsigned short  Events;        /* number of events this packet     */
         unsigned short  Cont;          /* Continuation flag for large evts */
         unsigned short  Data[MAX_DATA_PARAMS];   /* event data             */
};

/*
*   Command messages to the Acquisition task
*/
struct cntl_msg {
             char  flag;
             char  msg;
             char  reply;
};
/*
*   Parameters for the XIA time readout
*/

struct XIA  {
      int aoutbuffer;
      int gslttimea;
};

#define XIA_RAM  0x3ff00               /* Start of a structure used for XIA 
                                          time parameters                   */

#endif          /* end  ACQ_PARAMS_H_   */
