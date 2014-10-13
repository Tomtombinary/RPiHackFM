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
*    along with RPiHackFM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UART_HEADER_H
#define UART_HEADER_H

int UARTopen();
void UARTwrite(unsigned char * buffer,int uart_descriptor);
void UARTread(unsigned char *receive_buffer,int uart_descriptor,int size);

#endif
