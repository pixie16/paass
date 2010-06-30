#ifndef  ORPH_UDP_H_
#define  ORPH_UDP_H_
/*   orph.h  -  header file of definitions for VME to Linux
     UDP Ethernet protocol. 
     Robert Varner Feb 1992                                               */
/* version 1.1 12 August 1992 */
/* Linux version  3/11/03     */
/* UDP Version 3/9/05  RLV */
/* This version now uses UDP for Ethernet transmission.  The constants have
   been adjusted for the reduced size of the available data.  All sizes
   are in accord with 802.3 and relevant TCP/IP standards.                */

/*    Some sizes and limits */
/* These sizes are in bytes.  Start from 802.3 definition of packet data
   size, then subtract the IP-protocol headers.  We do NOT use IP options,
   nor will we use TCP options, so the minimum header size is good.       
   The protocol header sizes are:
       IP  - 20 bytes
       TCP - 20 bytes
       UDP -  8 bytes
   A TCP or UDP datagram has an IP header in addition to the protocol header.
*/


#define MAX_ETHER_DATA 1500   /* Bytes, in accord with 802.3              */
#define MIN_ETHER_DATA   46   /* Bytes                                    */

/*  For all IP packets */
#define IP_HDR_SIZE      20   /* Bytes */
#define MAX_IP_DATA   MAX_ETHER_DATA-IP_HDR_SIZE /*1480B IP data   */
#define MIN_IP_DATA   MIN_ETHER_DATA-IP_HDR_SIZE /*  26 bytes      */

/*  For UDP packets (mostly what we use) */
#define UDP_HDR_SIZE      8   /* Bytes */
#define MAX_UDP_DATA  MAX_IP_DATA-UDP_HDR_SIZE  /* 1472b UDP data  */
#define MIN_UDP_DATA  MIN_IP_DATA-UDP_HDR_SIZE  /*   18 bytes      */

/*  For TCP packets (a reference)        */
#define TCP_HDR_SIZE      20    /* Bytes */       
#define MAX_TCP_DATA  MAX_IP_DATA-TCP_HDR_SIZE  /* 1460B TCP data */
#define MIN_TCP_DATA  MIN_IP_DATA-TCP_HDR_SIZE  /*    6 bytes     */


#define PKTHDRLEN        8    /* Number of bytes in addition to data */
#define MAX_ORPH_DATA  MAX_UDP_DATA-PKTHDRLEN /* 1464 bytes, 8B ORPH hdr */
#define MIN_ORPH_DATA  MIN_UDP_DATA-PKTHDRLEN /*   10 bytes, 8B ORPH hdr */
struct UDP_Packet {
   int  Sequence;        /* Sequence number for reliable transport */
   int  DataSize;        /* Number of useable bytes in Data */
   unsigned char Data[MAX_ORPH_DATA]; /* the data to be transmitted */
};

/*    Definitions for protocols, messages and states  */

/*     Ethernet Protocols */
#define PROTO_OFFSET   45000	/* IP port number offset; add to proto# */

#define PROTO_DATA	0x50	/* Event Data */
#define PROTO_REQUEST_BOOT	0x51	/* VME requests to be booted */
#define PROTO_FORCE_BOOT	0x52	/* Force VME to reboot  */
#define PROTO_CODE	0x53	/* Code download to VME		*/
#define PROTO_FASTBUS	0x54	/* Exec FastBus command 	*/
#define PROTO_CNAF	0x55	/* Exec CNAF for CAMAC 		*/
#define PROTO_FECNTRL	0x56	/* Control messages from WS 	*/
#define PROTO_FEMSG	0x57	/* FrontEnd Messages to WS	*/
#define PROTO_TEST	0x58	/* Protocol for testing         */
#define PROTO_SOFT	0x59	/* Message to/from software in 
                                   front-end                    */
#define PROTO_VMEIO     0x5d    /* Control of VME acq modules   */
#define PROTO_LNFILL    0x5e    /* LN filling system            */
#define PROTO_RMSSIO    0x5f    /* RMS/DRS control systems      */
#define PROTO_MAX       0x60    /* Maximum +1 - used for tests  */

/* A sometimes convenient function to calculate the socket number */
#define protsock(x) 45000+x

/*    Enumerated type of servers.  Makes some program logic simple */
 enum server_proto {DATA=0x50, REQUEST_BOOT, FORCE_BOOT, CODE, FASTBUS,
             CNAF, FECNTRL, FEMSG, TEST, SOFT ,SPARE1, SPARE2, SPARE3,
             VMEIO, LNFILL, RMSSIO, MAX_ORPH_PROTO};

#define MAX_TIMEOUT     60   /* maximum number of seconds to wait for reply */

#endif      /* end  ORPH_UDP_H_   */
