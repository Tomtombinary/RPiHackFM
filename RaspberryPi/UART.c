/*
*  Copyright (C) 2014  Thomas DUBIER
* 
*    This file is part of RPiHackFM.
*
*    RPiHackFM is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    RPiHackFM is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "UART.h"


//Ouverture de la liaison serie
int UARTopen(){
	int uart_descriptor = open("/dev/ttyAMA0",O_RDWR |  O_NOCTTY | O_NDELAY);
        if(uart_descriptor == -1){
                printf("Error - Unable to open UART\n");
                exit(0);
        }
	//Configuration de la liaison serie
        struct termios options;
        tcgetattr(uart_descriptor,&options);
        options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart_descriptor,TCIFLUSH);
        tcsetattr(uart_descriptor,TCSANOW,&options);
	return uart_descriptor;
}

void UARTwrite(unsigned char * buffer,int uart_descriptor){
	int size = strlen(buffer);
	//Envoyer la taille du message
	int count = write(uart_descriptor,&size,sizeof(size));
	printf("[Send] %s",buffer);
	sleep(1);
	//Puis le message
	count = write(uart_descriptor,buffer,size);
	if(count < 0){
		printf("UART TX error\n");
	}
}

void UARTread(unsigned char *receive_buffer,int uart_descriptor,int size){
	int count = read(uart_descriptor,receive_buffer,255); //Lire un message en provenance du module bluetooth
	if(count>0){
		receive_buffer[count] = '\0';//mettre le 0 de fin
		printf("%s\n",receive_buffer);
	}
}

