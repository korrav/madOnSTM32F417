/**
  ******************************************************************************
  * @file    udp_echoserver.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-October-2011
  * @brief   UDP echo server
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "udp_echoserver.h"
#include "analog_data.h"
#include <string.h>

#define MODE 0 //continioud mode
#define SIGNAL_SAMPL 3
#define SIZE_BUF_COM_DW 380
//answer
#define OK 1
#define NOT_OK 0
//command IDs
#define COM_SYNC 0

static struct dataUnit {
	int ident;
	int mode;
	unsigned int numFirstCount;
	unsigned int amountCount;
	int id_MAD;
	short sampl[LEN_WINDOW * 4];
} packData;

static struct udp_pcb *upcb_d  = NULL; 
static struct udp_pcb *upcb_c  = NULL;
static int32_t buf_com[SIZE_BUF_COM_DW];

static struct { 
	struct ip_addr BagIPaddr;
	u16_t port_d;
	u16_t port_c;
} BagAddr;

/* Private function prototypes -----------------------------------------------*/
void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void trans_Control(void* buf, size_t len);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */
bool udp_echoserver_init(void)
{
	//create data socket
   upcb_d = udp_new(); 
   upcb_c = udp_new();  
   if (upcb_d && upcb_c)
   {
		 udp_bind(upcb_d, IP_ADDR_ANY, MAD_PORT_DATA);
     udp_recv(upcb_d, udp_echoserver_receive_callback, NULL);
		 udp_bind(upcb_c, IP_ADDR_ANY, MAD_PORT_CONTROL);
		 udp_recv(upcb_c, udp_echoserver_receive_callback, NULL);
		 IP4_ADDR(&BagAddr.BagIPaddr, DEST_IP_BADDR0, DEST_IP_BADDR1, DEST_IP_BADDR2, DEST_IP_BADDR3);
		 BagAddr.port_d = BAG_PORT_DATA;
		 BagAddr.port_c = BAG_PORT_CONTROL;
   }
   else
		 return false;
	 packData.id_MAD = ID_MAD;
	 packData.amountCount = LEN_WINDOW;
	 packData.ident = SIGNAL_SAMPL;
	 return true;
}

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
void udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	int answer[2] = {0, 0};
	if(sizeof(buf_com) < p->tot_len || port!= BagAddr.port_c)
		return;
	pbuf_copy_partial(p, buf_com, p->tot_len, 0);
	switch(buf_com[0]) {
		case COM_SYNC:
			if(p->tot_len == sizeof(int32_t)) {
				setSync();
				answer[0] = buf_com[0];
				answer[1] = OK;
				trans_Control(answer, sizeof(answer));
			}
		}
	pbuf_free(p);
	return;
}

void trans_Data(short* buf_ch1, short* buf_ch2, unsigned short num) {
	int i = 0;
	struct pbuf *p;
	for(; i < num; i++) {
		memcpy(&packData.sampl[4 * i], &buf_ch1[2 * i], 2 * sizeof(short));
		memcpy(&packData.sampl[4 * i + 2], &buf_ch2[2 * i], 2 * sizeof(short));
	}
	p = pbuf_alloc(PBUF_TRANSPORT, sizeof(packData), PBUF_POOL);
	if (p != NULL) {
		pbuf_take(p, &packData, sizeof(packData));
    /* send udp data */
		udp_connect(upcb_d, &BagAddr.BagIPaddr, BagAddr.port_d);
    udp_send(upcb_d, p);  
		udp_disconnect(upcb_d);
    /* free pbuf */
    pbuf_free(p);
  }
}

void trans_Control(void* buf, size_t len) {
	struct pbuf *p;
	p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	if (p != NULL) {
		pbuf_take(p, buf, len);
    /* send udp data */
		udp_connect(upcb_c, &BagAddr.BagIPaddr, BagAddr.port_c);
    udp_send(upcb_c, p);  
		udp_disconnect(upcb_c);
    /* free pbuf */
    pbuf_free(p);
  }
	return;
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
