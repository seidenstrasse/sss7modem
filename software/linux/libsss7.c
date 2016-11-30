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


uint8_t uart_rx_byte;
_Atomic uint8_t uart_tx_byte, uart_has_tx_byte, uart_tx_done;

pthread_t event_thread;
pthread_mutex_t state_mutex, rx_buffer_mutex;

int serial_fd;

int get_milliseconds(void) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return 	tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


int uart_init(char *serialport) {
	serial_fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
	if (serial_fd == -1) {
	  perror("Error: Unable to open serialport");
	  return -1;
	}

	//Switch to blocking mode
	fcntl(serial_fd, F_SETFL, 0);

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

	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 0;
	tcsetattr(serial_fd, TCSAFLUSH, &options);

	return 0;
}

void *eventloop(void *arg);

int libsss7_start(char *serialport) {
	int res = 0;

	sss7_init();
	res = uart_init(serialport);
	if(res) {
		return -1;
	}

	uart_has_tx_byte = 0;
	uart_tx_done = 0;

	pthread_mutex_init(&state_mutex, NULL);
	pthread_mutex_init(&rx_buffer_mutex, NULL);

	res = pthread_create(&event_thread, NULL, eventloop, NULL);
	if(res) {
		printf("Could not create eventloop thread\n");
		return -1;
	}

	return 0;
}

int libsss7_can_send(void) {
	return sss7_can_send();
}

void libsss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	pthread_mutex_lock(&state_mutex);
	sss7_send(msg);
	pthread_mutex_unlock(&state_mutex);
}

int libsss7_send_failed(void) {
	return sss7_send_failed();
}

int libsss7_has_received(void) {
	return sss7_has_received();
}

void libsss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	pthread_mutex_lock(&rx_buffer_mutex);
	sss7_get_received(msg);
	pthread_mutex_unlock(&rx_buffer_mutex);
}

void libsss7_stop() {
	pthread_join(event_thread, NULL);

	close(serial_fd);
}

uint8_t uart_get_byte(void) {
    return uart_rx_byte;
}

void uart_put_byte(uint8_t byte) {
	uart_has_tx_byte = 1;
	uart_tx_byte = byte;
}

void *eventloop(void *arg) {
	int res = 0;
	int timestamp = get_milliseconds();
	while(1) {
		printf("Loop\n");
		if(uart_has_tx_byte) {
			write(serial_fd, &uart_tx_byte, 1);
			printf("Send %x\n", uart_tx_byte);
			uart_has_tx_byte = 0;
			uart_tx_done = 1;
		}

		res = read(serial_fd, &uart_rx_byte, 1);
		if(res == 1) {
			pthread_mutex_lock(&state_mutex);
			pthread_mutex_lock(&rx_buffer_mutex);
			sss7_process_rx();
			pthread_mutex_unlock(&rx_buffer_mutex);
			pthread_mutex_unlock(&state_mutex);
		}

		if(uart_tx_done) {
			uart_tx_done = 0;
			pthread_mutex_lock(&state_mutex);
			sss7_process_tx();
			pthread_mutex_unlock(&state_mutex);
		}

		pthread_mutex_lock(&state_mutex);
		int now = get_milliseconds();
		printf("Ticks: %d\n", now - timestamp);
		sss7_process_ticks(now - timestamp);
		timestamp = now;
		pthread_mutex_unlock(&state_mutex);
	}

	return NULL;
}
