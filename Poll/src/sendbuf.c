#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include "pixie16app_defs.h"

#include  "Acq_Params.h"
#include  "Buffer_Structure.h"

/* Static  variables  */

static int clilen,seq = 0; 
static int TotalBuf=0; 
static int sockfd = -1; 
static struct sockaddr_in cli_addr;
static struct hostent *he;
static char events_flag=1;
//----------------------------------

void send_buf(struct data_pack *AcqBuf)
{
  const int BufEnd=0xffffffff;
  short int status,cont_pkt;
  char  *bufptr;
  int size,bytes;
  clilen = sizeof(cli_addr);
  
  
  /*  ffff ffff for even ffff ffff ffff for odd num. words" */
  //   printf("send_buf sockfd= %d\n",sockfd);
  
  size=AcqBuf->BufLen;
  //     printf("send_buf %d \n",size);
  memcpy(AcqBuf->Data+size,&BufEnd,4);
  size+=4;
  // if((size >>1<<1) - size )
  if (size % 2 != 0)
    { memcpy(AcqBuf->Data+size,&BufEnd,2);
      size+=2;
    }
   /*  ffff ffff for even ffff ffff ffff for odd num. words" */


    //size=AcqBuf->BufLen; 


  if ( size <= MAX_PKT_DATA)
    { 
      
      /*----------------------------------------*/
      
      AcqBuf->Head.Sequence = seq++;
      AcqBuf->Head.DataSize = size + 8;    
      AcqBuf->TotalEvents = ++TotalBuf;
      AcqBuf->Events = 1;
      AcqBuf->Cont=0; 
      
      /*--- Sending data -------------------*/
      
      status = sendto(sockfd,(char *)&AcqBuf->Head,size + PKTHDRLEN + 8,0,
		      (struct sockaddr *)&cli_addr,clilen);
      if (status < 0)
	{
	  perror("< data_proc - error at sendto");
	  exit(2003);
	}
      
      return ;
    }
  
  /*------- size > MAX_PKT_DATA ----------------------------------*/
  cont_pkt = 1;
  bufptr =(char *) &AcqBuf->Head;
  TotalBuf++;
  do
    {
      if (size > MAX_PKT_DATA) 
	{ bytes = MAX_PKT_DATA;
	  *((unsigned short *)(bufptr+PKTHDRLEN+4)) = 0;
	} else 
	{ bytes = size;
	  *((unsigned short *)(bufptr+PKTHDRLEN+4)) = 1;
	}
      *((int *)(bufptr+PKTHDRLEN)) = TotalBuf;
      *((unsigned short *)(bufptr+PKTHDRLEN+6)) = cont_pkt;
      *((unsigned int *)(bufptr)) = seq++;
      *((int *)(bufptr+4)) = bytes+8;
      
      status = sendto(sockfd,(char *)bufptr,bytes + PKTHDRLEN + 8,0,
		      (struct sockaddr *)&cli_addr,clilen);
      if (status < 0)
	{
	  perror("> data_proc - error at sendto");
	  exit(2003);
	}
      
      cont_pkt++;
      bufptr += bytes;
      size = size - bytes;
    }
  while(size > 0);
  events_flag=1;
  /*-----END ---(size > MAX_PKT_DATA----------------------------------*/
  
}


int spkt_connect(const char *node, int proto)
{  
  if (sockfd == -1) {    
    if ((he=gethostbyname(node)) == NULL) {  // get the host info
      perror("gethostbyname");
      exit(1);
    }
 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      perror("socket");
      exit(1);
    }
    
    printf(" spkt_connect sockfd= %d\n",sockfd);

    cli_addr.sin_family = AF_INET;     // host byte order
    cli_addr.sin_port = htons(protsock(proto)); // short, network byte order
    cli_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(cli_addr.sin_zero), '\0', 8);  // zero the rest of the struct

  }            /* End of sockfd test */

  return 0;  
}               /* End of spkt_open   */

