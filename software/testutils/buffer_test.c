#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#define buffer_elements  8
#define buffer_size (buffer_elements + 1)

int buffer[buffer_size];
volatile uint8_t read_pos;
volatile uint8_t write_pos;


void *producer_function(void* arg) {
    uint8_t foo = 1;
    uint8_t next_pos = 0;
    while(1) {




        next_pos = (write_pos + 1) % buffer_size;
        if(next_pos != read_pos) {
            buffer[write_pos] = foo;
            write_pos = next_pos;
            buffer[write_pos] = -1;
            printf("Produced %d\n", foo);
            foo = (foo + 1) % 255;
        }

        usleep((rand() % 500) * 100);
    }
}

void *consumer_function(void* arg) {
    int bar = 0;

    while(1) {

        if(read_pos != write_pos) {
            if(buffer[read_pos] != (bar + 1) % 255) {
                printf("Error occured\n");
                printf("bar: %d\n", bar);
                printf("foo: %d\n", buffer[read_pos]);
                return 0;
            }

            bar = buffer[read_pos];
            printf("Consumed: %d (%d)\n", buffer[read_pos], (write_pos - read_pos + buffer_size) % buffer_size);
            read_pos = (read_pos + 1) % buffer_size;
        }

        usleep((rand() % 500) * 100);
    }
}


int main() {
    pthread_t producer, consumer;
    int ret;

    read_pos = 0;
    write_pos = 0;

    ret = pthread_create(&producer, NULL, producer_function, NULL);
    if(ret) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(-1);
    }

    ret = pthread_create(&consumer, NULL, consumer_function, NULL);
    if(ret) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
        exit(-1);
    }

    pthread_join( consumer, NULL);

    return 0;
}
