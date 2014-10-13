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


#ifndef PIFM_HEADER_H
#define PIFM_HEADER_H

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
char *gpio_mem, *gpio_map;
char *spi0_mem, *spi0_map;


// Acces d'entré sortie
volatile unsigned *gpio;
volatile unsigned *allof7e;

#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000) //Addresse de base pour controller les peripheriques de la Raspberry Pi

//Macro de configuration des entrées sortie ,INP_GPIO à utiliser toujours avant de définir la patte en entrée (de base) ou en sortie
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3)) //Configuration en sortie
#define IN_GPIO(g) *(gpio+((g)/10)) &=  ~(7<<(((g)%10)*3)) //Configuration en entrée
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET(g) *(gpio+7)=1<<g  // Mettre la patte g à l'état haut
#define GPIO_CLR(g) *(gpio+10)=1<<g // Mettre la patte g à l'état bas
#define GPIO_GET(g) *((gpio+13)+(g/32))&(1 << g%32)  // Récuperer l'état de la patte g

#define ACCESS(base) *(volatile int*)((int)allof7e+base-0x7e000000) //Acces pour la DMA
#define SETBIT(base, bit) ACCESS(base) |= 1<<bit    // mettre un bit à 1
#define CLRBIT(base, bit) ACCESS(base) &= ~(1<<bit) // mettre un bit à 0

#define CM_GP0CTL (0x7e101070)
#define GPFSEL0 (0x7E200000)
#define CM_GP0DIV (0x7e101074)
#define CLKBASE (0x7E101000)
#define DMABASE (0x7E007000)
#define PWMBASE  (0x7e20C000) /* PWM controlleur */

/*
*  Tiré de pifm
*/
struct GPCTL {
    char SRC         : 4;
    char ENAB        : 1;
    char KILL        : 1;
    char             : 1;
    char BUSY        : 1;
    char FLIP        : 1;
    char MASH        : 2;
    unsigned int     : 13;
    char PASSWD      : 8;
};

struct CB {
    volatile unsigned int TI;
    volatile unsigned int SOURCE_AD;
    volatile unsigned int DEST_AD;
    volatile unsigned int TXFR_LEN;
    volatile unsigned int STRIDE;
    volatile unsigned int NEXTCONBK;
    volatile unsigned int RES1;
    volatile unsigned int RES2;

};

struct DMAregs {
    volatile unsigned int CS;
    volatile unsigned int CONBLK_AD;
    volatile unsigned int TI;
    volatile unsigned int SOURCE_AD;
    volatile unsigned int DEST_AD;
    volatile unsigned int TXFR_LEN;
    volatile unsigned int STRIDE;
    volatile unsigned int NEXTCONBK;
    volatile unsigned int DEBUG;
};

struct PageInfo {
    void* p;  // physical address
    void* v;   // virtual address
};

struct PageInfo constPage;
struct PageInfo instrPage;
struct PageInfo instrs[1024];

//Liste des fonctions
void getRealMemPage(void** vAddr,void** pAddr);
void freeRealMemPage(void* vAddr);
void setup_fm(int state);
void setup_io();
void reset();
void modulate(int m);
void playWav(char *filename,float samplerate);
void shutdown();
void setupDMA(float centerFreq);

#endif
