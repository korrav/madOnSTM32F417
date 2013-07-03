#include "analog_data.h"
#include <cstring>
#include "udp_echoserver.h"


struct Analog_buf {
	short buf[SIZE_BUF * 2];
	short rest[LEN_WINDOW * 2];
} buf[2];

static struct {
	unsigned short pos_h;	//position data handler; unit position is equal to two counts
	char side_h; //side data handler
	unsigned short pos_f;	//position data filler
	char side_f;		//side data filler
	unsigned char side[2]; //sides private buffers
	char isRestFill; //rest of the buffer is full
	unsigned int firstCount; //number of first sampl of the buffer	
} status;

static struct {
	char side;
	unsigned int correctFirstCount;
} sync;

struct Analog_buf* getAnalogBuf(unsigned char num){
	return &buf[num];
}

void initialAnalogHandler(void) {
	status.pos_f = 0;
	status.pos_h = 0;
	status.side_f = 0;
	status.side_h = 0;
	status.side[0] = 0;
	status.side[1] = 0;
	status.isRestFill = 0;
	status.firstCount = 0;
	sync.correctFirstCount = 0;
	sync.side = (char)-1;
	return;
}

static unsigned short get_positionDMA(void);
static char recognition(short* buf1, short* buf2, unsigned short num);
static char fillRest(void);

void run(void) {
	while ((status.pos_h + LEN_WINDOW) <= get_positionDMA()) {
		//check fill rest
		if((status.pos_h >= SIZE_BUF - LEN_WINDOW) && !status.isRestFill) {
			if(!fillRest())
				break;
		}	
		//recognition
		if (recognition(&buf[0].buf[2 * status.pos_h], &buf[1].buf[2 * status.pos_h], LEN_WINDOW))
			status.pos_h += LEN_WINDOW;
		else
			status.pos_h += STEP_WINDOW;
		//check of end buffer
		if(status.pos_h > SIZE_BUF) {
			status.pos_h -= SIZE_BUF;
			status.isRestFill = 0;
			status.side_h = !status.side_h;
			status.firstCount += SIZE_BUF;
			//check on synchronization
			if(status.side_h == sync.side) {
				status.firstCount -= sync.correctFirstCount;
				sync.side = (char)-1;
			}
		}
	}
	return;
}

void interruptDmaSpi(unsigned char num) {
	status.side[num] = !status.side[num];
	if(status.side[0] == status.side[1])
		status.side_f = status.side[0];
	return;
	
}

unsigned short get_positionDMA(void) { 
	unsigned short pos_S1 = 0, pos_S2 = 0, pos_S = 0;
	char side[2] = {0};
	__disable_irq ();
	DMA_Cmd(DMA2_Stream0, DISABLE);
	DMA_Cmd(DMA1_Stream3, DISABLE);
 	side[0] = status.side[0];
 	side[1] = status.side[1];
	pos_S = DMA_GetCurrDataCounter(DMA2_Stream0);
	pos_S2 = DMA_GetCurrDataCounter(DMA1_Stream3);
	DMA_Cmd(DMA2_Stream0, ENABLE);
	DMA_Cmd(DMA1_Stream3, ENABLE);
	if(status.side_h != status.side_f) {
		__enable_irq ();
		return SIZE_BUF + LEN_WINDOW - 1;
	}
	__enable_irq ();
	if(status.side_h == side[0])
			pos_S = pos_S1;
	else
			pos_S = pos_S2;
	return SIZE_BUF - (pos_S/2);	
}

char recognition(short* buf1, short* buf2, unsigned short num) {
	trans_Data(buf1, buf2, num);
	return 1;
}
 char fillRest() {
  if (status.side_f == status.side_h || (SIZE_BUF - DMA_GetCurrDataCounter(DMA2_Stream0)/2 - 2) < LEN_WINDOW) {
		status.isRestFill = 0;
	}	else {
		memcpy(buf[0].rest, buf[0].buf, sizeof(buf[0].rest));
		memcpy(buf[1].rest, buf[1].buf, sizeof(buf[1].rest));
		status.isRestFill = 1;
	}
		return status.isRestFill;
 }
 
 void setSync(void) {
	__disable_irq ();
	DMA_Cmd(DMA2_Stream0, DISABLE);
 	sync.side = !status.side[0];
	sync.correctFirstCount = SIZE_BUF - DMA_GetCurrDataCounter(DMA2_Stream0)/2;
	DMA_Cmd(DMA2_Stream0, ENABLE);
	__enable_irq ();
	return;
 }	 
