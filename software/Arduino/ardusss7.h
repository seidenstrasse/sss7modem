#ifndef _ARDUSSS7_H_
#define _ARDUSSS7_H_

#include "sss7.h"

class SSS7Wrapper {
	public:
		void init();
		uint8_t canSend();
		void send(uint8_t msg[SSS7_PAYLOAD_SIZE]);
		uint8_t sendFailed();
		uint8_t hasReceived();
		void getReceived(uint8_t msg[SSS7_PAYLOAD_SIZE]);

	private:
		void setupUart();
		void setupTimer();
};

extern SSS7Wrapper SSS7;

#define BAUD 9600UL

// Some calculations ...
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // Rounding magic
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Real baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD)      // Error in 0.1%

#if ((BAUD_ERROR<950) || (BAUD_ERROR>1050))		// Make sure our UBRR_VAL will work
  #error Baudrate error is bigger then 1% !
#endif

#endif
