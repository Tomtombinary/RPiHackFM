// Source : https://github.com/rm-hull/pifm/blob/master/pifm.cpp

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include "pifm.h"


void getRealMemPage(void** vAddr, void** pAddr) {
    void* a = valloc(4096);

    ((int*)a)[0] = 1;  // use page to force allocation.

    mlock(a, 4096);  // lock into ram.

    *vAddr = a;  // yay - we know the virtual address

    unsigned long long frameinfo;

    int fp = open("/proc/self/pagemap", 'r');
    lseek(fp, ((int)a)/4096*8, SEEK_SET);
    read(fp, &frameinfo, sizeof(frameinfo));

    *pAddr = (void*)((int)(frameinfo*4096));
}

//Mise a zero de la radio
void reset(){
    struct DMAregs* DMA0 = (struct DMAregs*)&(ACCESS(DMABASE));
    DMA0->CS =1<<31;  // reset dma controller
    setup_fm(0); //shutdown fm
}

void freeRealMemPage(void* vAddr) {

    munlock(vAddr, 4096);  // unlock ram.

    free(vAddr);
}

void setup_fm(int state)
{
    printf("Setup FM : %d\n",state);
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit (-1);
    }

    allof7e = (unsigned *)mmap(
                  NULL,
                  0x01000000,  //len
                  PROT_READ|PROT_WRITE,
                  MAP_SHARED,
                  mem_fd,
                  0x20000000  //base
              );

    if ((int)allof7e==-1) exit(-1);
    SETBIT(GPFSEL0 , 14);
    CLRBIT(GPFSEL0 , 13);
    CLRBIT(GPFSEL0 , 12);


    struct GPCTL setupword = {6/*SRC*/,state, 0, 0, 0,state,0x5a};  
    ACCESS(CM_GP0CTL) = *((int*)&setupword);
    //Etat de la radio indiqué sur la led sur la patte 3
    if(state==1)
        GPIO_SET(3);
    else
        GPIO_CLR(3);
    close(mem_fd);

}

void setup_io(){
   
   /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit (-1);
    }

    //Mappé le fichier en mémoire
   gpio_map = mmap(
        NULL,                 //n'importe quelle addresse
        BLOCK_SIZE,           //taille
        PROT_READ|PROT_WRITE, //Acces lecture ecriteur
        MAP_SHARED,           //Partagée avec d'autre processus
        mem_fd,               //Fichier à mappé
        GPIO_BASE             //offset Peripherique d'entre sortie
    );
   gpio = (volatile unsigned int*)gpio_map;
   close(mem_fd);
   //Definition des entrées sortie
   INP_GPIO(3);
   INP_GPIO(8);
   INP_GPIO(7);

   OUT_GPIO(3);

   GPIO_CLR(3);
}

void modulate(int m)
{
    ACCESS(CM_GP0DIV) = (0x5a << 24) + 0x4d72 + m;
}

void playWav(char* filename, float samplerate)
{
    printf("Play %s\n",filename);
    int fp= STDIN_FILENO;
    if(filename[0]!='-') fp = open(filename, 'r');
    //int sz = lseek(fp, 0L, SEEK_END);
    //lseek(fp, 0L, SEEK_SET);
    //short* data = (short*)malloc(sz);
    //read(fp, data, sz);

    int bufPtr=0;
    float datanew, dataold = 0;
    short data;

    for (int i=0; i<22; i++)
       read(fp, &data, 2);  // read past header

    while (read(fp, &data, 2)) {
        float fmconstant = samplerate * 50.0e-6;  // for pre-emphisis filter.  50us time constant
        int clocksPerSample = 22500.0/samplerate*1400.0;  // for timing

        datanew = (float)(data)/32767;

        float sample = datanew + (dataold-datanew) / (1-fmconstant);  // fir of 1 + s tau
        float dval = sample*15.0;  // actual transmitted sample.  15 is bandwidth (about 75 kHz)

        int intval = (int)(round(dval));  // integer component
        float frac = (dval - (float)intval)/2 + 0.5;
        unsigned int fracval = frac*clocksPerSample;

        bufPtr++;
        while( ACCESS(DMABASE + 0x04 /* CurBlock*/) ==  (int)(instrs[bufPtr].p)) usleep(1000);
        ((struct CB*)(instrs[bufPtr].v))->SOURCE_AD = (int)constPage.p + 2048 + intval*4 - 4 ;

        bufPtr++;
        while( ACCESS(DMABASE + 0x04 /* CurBlock*/) ==  (int)(instrs[bufPtr].p)) usleep(1000);
        ((struct CB*)(instrs[bufPtr].v))->TXFR_LEN = clocksPerSample-fracval;

        bufPtr++;
        while( ACCESS(DMABASE + 0x04 /* CurBlock*/) ==  (int)(instrs[bufPtr].p)) usleep(1000);
        ((struct CB*)(instrs[bufPtr].v))->SOURCE_AD = (int)constPage.p + 2048 + intval*4+4;

        bufPtr=(bufPtr+1) % (1024);
        while( ACCESS(DMABASE + 0x04 /* CurBlock*/) ==  (int)(instrs[bufPtr].p)) usleep(1000);
        ((struct CB*)(instrs[bufPtr].v))->TXFR_LEN = fracval;

        dataold = datanew;
    }
    close(fp);
}

void handSig(){
  exit(0);
}

void setupDMA( float centerFreq ){
  printf("Setup DMA \n");
  //Pour la fermeture
  atexit(shutdown);
  signal (SIGINT, handSig);
  signal (SIGTERM, handSig);
  signal (SIGHUP, handSig);
  signal (SIGQUIT, handSig);

   // allocate a few pages of ram
   getRealMemPage(&constPage.v, &constPage.p);

   int centerFreqDivider = (int)((500.0 / centerFreq) * (float)(1<<12) + 0.5);

   // make data page contents - it's essientially 1024 different commands for the
   // DMA controller to send to the clock module at the correct time.
   for (int i=0; i<1024; i++)
     ((int*)(constPage.v))[i] = (0x5a << 24) + centerFreqDivider - 512 + i;


   int instrCnt = 0;

   while (instrCnt<1024) {
     getRealMemPage(&instrPage.v, &instrPage.p);

     // make copy instructions
     struct CB* instr0= (struct CB*)instrPage.v;

     for (int i=0; i<4096/sizeof(struct CB); i++) {
       instrs[instrCnt].v = (void*)((int)instrPage.v + sizeof(struct CB)*i);
       instrs[instrCnt].p = (void*)((int)instrPage.p + sizeof(struct CB)*i);
       instr0->SOURCE_AD = (unsigned int)constPage.p+2048;
       instr0->DEST_AD = PWMBASE+0x18 /* FIF1 */;
       instr0->TXFR_LEN = 4;
       instr0->STRIDE = 0;
       //instr0->NEXTCONBK = (int)instrPage.p + sizeof(struct CB)*(i+1);
       instr0->TI = (1/* DREQ  */<<6) | (5 /* PWM */<<16) |  (1<<26/* no wide*/) ;
       instr0->RES1 = 0;
       instr0->RES2 = 0;

       if (i%2) {
         instr0->DEST_AD = CM_GP0DIV;
         instr0->STRIDE = 4;
         instr0->TI = (1<<26/* no wide*/) ;
       }

       if (instrCnt!=0) ((struct CB*)(instrs[instrCnt-1].v))->NEXTCONBK = (int)instrs[instrCnt].p;
       instr0++;
       instrCnt++;
     }
   }
   ((struct CB*)(instrs[1023].v))->NEXTCONBK = (int)instrs[0].p;

   // set up a clock for the PWM
   ACCESS(CLKBASE + 40*4 /*PWMCLK_CNTL*/) = 0x5A000026;
   usleep(1000);
   ACCESS(CLKBASE + 41*4 /*PWMCLK_DIV*/)  = 0x5A002800;
   ACCESS(CLKBASE + 40*4 /*PWMCLK_CNTL*/) = 0x5A000016;
   usleep(1000);

   // set up pwm
   ACCESS(PWMBASE + 0x0 /* CTRL*/) = 0;
   usleep(1000);
   ACCESS(PWMBASE + 0x4 /* status*/) = -1;  // clear errors
   usleep(1000);
   ACCESS(PWMBASE + 0x0 /* CTRL*/) = -1; //(1<<13 /* Use fifo */) | (1<<10 /* repeat */) | (1<<9 /* serializer */) | (1<<8 /* enable ch */) ;
   usleep(1000);
   ACCESS(PWMBASE + 0x8 /* DMAC*/) = (1<<31 /* DMA enable */) | 0x0707;

   //activate dma
   struct DMAregs* DMA0 = (struct DMAregs*)&(ACCESS(DMABASE));
   DMA0->CS =1<<31;  // reset
   DMA0->CONBLK_AD=0;
   DMA0->TI=0;
   DMA0->CONBLK_AD = (unsigned int)(instrPage.p);
   DMA0->CS =(1<<0)|(255 <<16);  // enable bit = 0, clear end flag = 1, prio=19-16
}
