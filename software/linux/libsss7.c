#include "libsss7.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

uint8_t uart_rx_byte;
uint8_t uart_tx_byte, uart_has_tx_byte;

int serial_fd;

int libsss7_start(char *serialport) {
	sss7_init();

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

	options.c_cc[VTIME] = 20;
	options.c_cc[VMIN] = 0;
	tcsetattr(serial_fd, TCSAFLUSH, &options);

	int res = 0;
	while(1) {
		res = read(serial_fd, &uart_rx_byte, 1);
		if(res == 1) {
			sss7_process_rx();
		}

		if(uart_has_tx_byte) {
			uart_has_tx_byte = 0;
			write(serial_fd, &uart_has_tx_byte, 1);
			sss7_process_tx();
		}
	}

	return 0;
}

int libsss7_can_send(void) {
	return sss7_can_send();
}

void libsss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]) {

}

int libsss7_send_failed(void) {
	return sss7_send_failed();
}

int libsss7_has_received(void) {
	return sss7_has_received();
}

void libsss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	return sss7_get_received(msg);
}

void libsss7_stop() {
	close(serial_fd);
}

uint8_t uart_get_byte(void) {
    return uart_rx_byte;
}

void uart_put_byte(uint8_t byte) {
	uart_has_tx_byte = 1;
	uart_tx_byte = byte;
}
