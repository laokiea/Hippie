/*
 * Include necessary header files
 */ 

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

void pthread_once_f(void);
void * client_f(void *);
void * dispatch_f(void *);
void tsd_destructor(void *);

#endif