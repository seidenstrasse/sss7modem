#include "libsss7.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <sys/time.h>

#include "sss7.h"

#if LIBSSS7_PAYLOAD_SIZE != SSS7_PAYLOAD_SIZE
	#error LIBSSS7_PAYLOAD_SIZE is not equal to SSS7_PAYLOAD_SIZE
#endif

// A state kept a across eventloop iterations to emulate tx complete interrupts
enum UartTxState {
	TX_IDLE,		// Idle state
	TX_HAS_BYTE,	// There is a byte to be send in the next iteration
	TX_COMPLETE		// A byte has been sent in the current iteration
};

// The last byte that has been received, read by uart_get_byte
uint8_t uart_rx_byte;

// The byte to send in the next eventloop iteration
_Atomic uint8_t uart_tx_byte;
// TX state variable
_Atomic enum UartTxState uart_tx_state;

// Flag to stop the event loop thread
_Atomic int stop_event_thread;
// Handle to the event loop thread
pthread_t event_thread;

// Mutex that protects the general sss7 state from concurrent modification
pthread_mutex_t state_mutex;
// Mutex that protects the rx buffer from concurrent modification
pthread_mutex_t rx_buffer_mutex;

// File handle for the serial port
int serial_fd;


// Generate a millisecond timestamp from gettimeofday
unsigned long get_milliseconds(void) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return 	tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Open the serial port and configure it
int serial_init(char *serialport) {
	serial_fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
	if (serial_fd == -1) {
	  perror("Error: Unable to open serialport");
	  return -1;
	}

	//Switch to blocking mode
	fcntl(serial_fd, F_SETFL, 0);

	// Set baudrates for input and output
	struct termios options;
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	// 8N1
	options.c_cflag = (CLOCAL | CREAD);
	options.c_cflag |= CS8;

	// Raw mode, no features
	options.c_lflag = 0;

	// No parity checks, no flow control
	options.c_iflag = 0;

	// No output processing
	options.c_oflag = 0;

	// A timeout of 0.1 seconds for each character
	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 0;
	tcsetattr(serial_fd, TCSAFLUSH, &options);

	return 0;
}


// Event loop thread, design to emulate the uart interrupts on AVR8
void *eventloop(void *arg) {
	int res = 0;
	unsigned long timestamp = get_milliseconds();

	while(!stop_event_thread) {

		// Send a byte, if there is one
		if(uart_tx_state == TX_HAS_BYTE) {
			write(serial_fd, &uart_tx_byte, 1);
			printf("Send %x\n", uart_tx_byte);
			uart_tx_state = TX_COMPLETE;
		}

		// Read back the byte, that was just send.
		// Or read a byte send by somebody else.
		res = read(serial_fd, &uart_rx_byte, 1);
		if(res == 1) {
			printf("Read %x\n", uart_tx_byte);
			// Lock mutexes and call rx handler
			pthread_mutex_lock(&state_mutex);
			pthread_mutex_lock(&rx_buffer_mutex);
			sss7_process_rx();
			pthread_mutex_unlock(&rx_buffer_mutex);
			pthread_mutex_unlock(&state_mutex);
		}

		// Call the tx handler if there has been a byte sent in this iteration
		if(uart_tx_state == TX_COMPLETE) {
			// Keep this before sss7_process_tx
			// uart_put_byte might need to overwrite it
			uart_tx_state = TX_IDLE;

			pthread_mutex_lock(&state_mutex);
			sss7_process_tx();
			pthread_mutex_unlock(&state_mutex);
		}

		// Lock muxte and call the timeout handler
		pthread_mutex_lock(&state_mutex);
		int now = get_milliseconds();
		sss7_process_ticks(now - timestamp);
		timestamp = now;
		pthread_mutex_unlock(&state_mutex);
	}

	return NULL;
}


// Setup function for the serialport and the eventloop thread
// Return 0 on succes -1 on failure.
int libsss7_start(char *serialport) {
	int res = 0;

	sss7_init();
	res = serial_init(serialport);
	if(res) {
		return -1;
	}

	uart_tx_state = TX_IDLE;
	stop_event_thread = 0;

	pthread_mutex_init(&state_mutex, NULL);
	pthread_mutex_init(&rx_buffer_mutex, NULL);

	res = pthread_create(&event_thread, NULL, eventloop, NULL);
	if(res) {
		printf("Could not create eventloop thread\n");
		return -1;
	}

	return 0;
}


// Teardown function for serialport and event loop thread
void libsss7_stop() {
	stop_event_thread = 1;
	pthread_join(event_thread, NULL);

	close(serial_fd);
}

// Wrapper for sss7_can_send
int libsss7_can_send(void) {
	return sss7_can_send();
}

// Wrapper for sss7_send, also locks the state_mutex
void libsss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	pthread_mutex_lock(&state_mutex);
	sss7_send(msg);
	pthread_mutex_unlock(&state_mutex);
}

// Wrapper for sss7_send_failed
int libsss7_send_failed(void) {
	return sss7_send_failed();
}

// Wrapper for sss7_has_received
int libsss7_has_received(void) {
	return sss7_has_received();
}

// Wrapper for sss7_get_received, takes care of locking rx buffer mutex
void libsss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	pthread_mutex_lock(&rx_buffer_mutex);
	sss7_get_received(msg);
	pthread_mutex_unlock(&rx_buffer_mutex);
}

// Platform dependent function required by the sss7core
uint8_t uart_get_byte(void) {
    return uart_rx_byte;
}

// Platform dependent function required by the sss7core
void uart_put_byte(uint8_t byte) {
	uart_tx_state = TX_HAS_BYTE;
	uart_tx_byte = byte;
}
