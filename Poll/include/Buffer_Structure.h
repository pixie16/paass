#ifndef __BUFFER_STRUCTURE_H_
#define __BUFFER_STRUCTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Structures declaration */
struct data_pack {
  struct Packet_Header Head;     /* Either packet header              */
  int  TotalEvents;              /* Total events number              */
  unsigned short  Events;        /* Number of events in this packet  */
  unsigned short  Cont;          /* Continuation flag for large evts */
  // char Data[4*(IO_BUFFER_LENGTH + 4)]; /* Buffer from Pixie16 */
  char  Data[4*(4050 + 4)];     /* Buffer from Pixie16              */
  int  BufLen;                   /* Length of original buffer from Pixie */
};

/* Function prototype */
int spkt_connect(const char *, int );
void send_buf(struct data_pack *);
int socket_poll(char );

#ifdef __cplusplus
}
#endif

#endif // __BUFFER_STRUCTURE_H

