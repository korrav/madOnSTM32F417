#include "stm32f4xx.h"

//unit = 2 * short
#define SIZE_BUF 5000
#define LEN_WINDOW 1000 
#define STEP_WINDOW 100

struct Analog_buf;

struct Analog_buf* getAnalogBuf(unsigned char num); 

void initialAnalogHandler(void);

void interruptDmaSpi(unsigned char num);

void run(void); //processing function of the analog buffer

void setSync(void);

