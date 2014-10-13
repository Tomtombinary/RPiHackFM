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
#include <pthread.h>
#include "pifm.h"
#include "UART.h"


#define MSGSIZE 256

int uart0_descriptor = -1;
FILE * musicBase = -1;

pthread_t threadplay = NULL;

void getMusicCmdById(char * buffer,int file_id);
void sendFile();
void * thread_play(void *arg);

float frequency = 87.5;
char fileName[128];

int main(int argc, char **argv)
{
    //Setup Entree/Sortie
    setup_io();

    //Ouverture de la liaison serie avec le module bluetooth
    uart0_descriptor = UARTopen();
    /*Ouverture de la base de film se trouvant dans un fichier texte sous la forme
    *  id:nomfichier!description
    */
    musicBase = fopen("musicBase.txt","r");

    //Buffer de traitement
    char buffer_cmd[MSGSIZE];
    char buffer_cpy[MSGSIZE];
    char cmd[32];

    while(1){
    	//Lire sur la liaison serie vers le buffer_cmd
    	UARTread(buffer_cmd,uart0_descriptor,MSGSIZE);
    	//H4ckP1FM commande pour obtenir la liste des fichiers musicaux
    	if(strcmp(buffer_cmd,"H4ckP1FM")==0){
    		printf("Send file list\n");
			//Attente que l'application soit prete (on est en Java coté telephone)
			sleep(2);
			//Envoi des nom et description de fichier dans l'ordre
    		sendFile();
    	//Eteindre la radio
    	}else if(strcmp(buffer_cmd,"OFF")==0){
    		printf("Stop FM\n");
		reset();
		//Sinon c'est une commande spéciale
    	}else{
    		//Buffer_cpy pour faire des modifications
    		strncpy(buffer_cpy,buffer_cmd,MSGSIZE);
    		char * ptr;
    		ptr = buffer_cpy;
    		//La commande se trouve sous la forme CMD:Parametres
    		while((*ptr)!=':' && ptr<buffer_cpy+MSGSIZE){
    			ptr++;
    		}
    		(*ptr) = '\0';
    		strncpy(cmd,buffer_cpy,32);
    		ptr++;
    		//Si CMD==ON allumer la radio a la frequence desiré ON:88,5 par exemple
    		if(strcmp(cmd,"ON")==0){
			char * virguletopoint = ptr;
			//Transformer la virgule en point pour la conversion chaine vers float
			while((*virguletopoint)!=',' && virguletopoint<buffer_cpy+MSGSIZE)
				virguletopoint++;
			(*virguletopoint) = '.';
			//Conversion chaine vers float
    		frequency = atof(ptr);
    		printf("Start FM [frequency:%f]\n",frequency);
    		//Demarre la fm
			setup_fm(1);
			setupDMA(frequency);
			//Si jouer un son PLAYID:IDduSon
    		}else if(strcmp(cmd,"PLAYID")==0){
    			//Si un son etait en train d'etre jouer on l'arrete
    			if(threadplay!=NULL)
					pthread_cancel(threadplay);
				//Conversion chaine vers int
				int id = atoi(ptr)+1;
				//Retrouver le nom de la musique
    			getMusicCmdById(fileName,id);
    			printf("Play name %s [id:%d] \n",fileName,id);
    		//Creer un thread qui va jouer le son
			if(pthread_create(&threadplay,NULL,thread_play,(void*)0)<0){
				printf("Error thread create");
			}
    		}
    	}
	buffer_cmd[0] = '\0';
	//100 ms d'attente entre chaque message
	usleep(100);
    }
    return 0;
}

//Jouer un son (echantillonage 22500 Hz)
void * thread_play(void *arg){
	playWav(fileName,22500);
}

//Fonction de fermeture 
void shutdown(){
    printf("exiting\n");
    struct DMAregs* DMA0 = (struct DMAregs*)&(ACCESS(DMABASE));
    DMA0->CS =1<<31;  // reset dma controller
    setup_fm(0); //shutdown fm
    if(uart0_descriptor!=-1) //Fermeture liaison serie
    	close(uart0_descriptor);
    if(musicBase!=-1) //Fermeture fichier des musiques
    	close(musicBase);
    if(threadplay!=NULL)//Fermeture du thread
        pthread_cancel(threadplay);
}

//Envoyer les noms et descriptions des fichiers musicaux par bluetooth
void sendFile(){
	//Se placer au debut du fichiers
	fseek(musicBase,0,SEEK_SET);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	//Lire ligne par ligne
	while((read = getline(&line,&len,musicBase))!=-1){
		char *ptr_debut  = line;
		//Recuperer uniquement ce qu'il y a apres ID:
		while((*ptr_debut)!=':' && ptr_debut<line+256){
			ptr_debut++;
		}

		if(ptr_debut<line+256){
			char cmd[256];
			//Construire la commande FILE:nom!description
			sprintf(cmd,"FILE%s",ptr_debut);
			//Envoyer
			UARTwrite(cmd,uart0_descriptor);
			//Attendre 3 seconde entre chaque envoye
			sleep(3);
		}
	}

	//Liberer l'espace alloué pour les lignes
	if(line)
		free(line);
}

//Recuperer musique par l'id
void getMusicCmdById(char * buffer,int file_id){
	//Debut du fichier
	fseek(musicBase,0,SEEK_SET);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int id;

	//Lire ligne par ligne
	while((read = getline(&line,&len,musicBase))!=-1){
		char * ptr_fin = line;
		//Recuperer l'id
		while((*ptr_fin)!=':'){
			ptr_fin++;
		}
		(*ptr_fin) = '\0';
		int id = atoi(line);
		ptr_fin++;
		//Si id est l'id de la musique
		if(id==file_id){
			char *ptr_debut = ptr_fin;
			while((*ptr_fin)!='!')
				ptr_fin++;
			(*ptr_fin) = '\0';
			//Ecrire dans buffer le nom complet de la musique
			strcpy(buffer,ptr_debut);
			break;
		}
	}

	//Liberer la memoire alloué pour les lignes
	if(line)
		free(line);
}
