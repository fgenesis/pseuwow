// PseuWoW_Controller.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <string>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>

#include "../shared/controllercodes.h"

#define gets(c) Gets(c)

#define MAXLEN 1024

bool DEBUG=true;

char *ver="PseuWoW Controlling Unit Build 1";
int error=0,port=10024;
SDL_Thread *connect_thread,*recieve_thread,*cmdhandler_thread,*quitproc_thread,*input_thread;
char *hostname="localhost";
bool ready=false,connected=false,authenticated=false,nopassword=true,quit=false,waitforinput;
TCPsocket tcpsock;
char *incomingtext;
unsigned char incomingcmd;
char *password,*outgoingtext;
char *remotever;

/////////// predeclare functions /////////////////////////
int connect(void *p);
void closeconnection(bool);
void quitproc(void);
int tquitproc(void *p);


//////////////////////////////////////////

void Gets(char *z,int m) {
   int ch;
   int counter=0;
   while((ch=getchar()) != '\n') {
      z[counter++]=ch;
      if(counter >= m)break;
	  if(quit){printf("Q: returning from input.");return;}
   }
   z[counter] = '\0';     /* Terminieren */
}

char *triml(char *data,int count){
	data=data+count;
	return data;
}

bool readconf(void){

// read the shit, then set nopassword=false :)
return 0; //error
}

void send(char code, char* sm_message="()"){
	if(quit)return;
	if(tcpsock==NULL){printf("ERROR: Tried to send data over a cloed socket!\n"); return;}
	int sm_result;
	char *packet=(char*)malloc(1024);
	sprintf(packet,"%c%s",code,sm_message);
	int len = strlen(packet) + 1; // add one for the terminating NULL
	sm_result = SDLNet_TCP_Send(tcpsock,packet,len);
	if( sm_result < len ) {
		printf( "SDLNet_TCP_Send: %s\n", SDLNet_GetError() );
	}
	else{
		//printf("out: %i, %s\n",code,sm_message);
	}
	free(packet);
}

int recieve(void *p) {
	int result;
	char msg[MAXLEN];
	while(connected) {
		//printf("recieve: Waiting for incoming data...\n");
		result = SDLNet_TCP_Recv(tcpsock,msg,MAXLEN);
		if(result <= 0) {
		    // TCP Connection is broken. (because of error or closure)
		    //closeconnection(false);
		
			quitproc_thread=SDL_CreateThread(tquitproc,NULL);
		} else {
		    msg[result] = 0;
		    		
			incomingcmd=msg[0];
			incomingtext=triml(msg,1);
			//printf("in: %i, %s\n",incomingcmd,incomingtext);
			
			
		}
	}
	if(DEBUG)printf("Reciever lost the connection, thread exited.\n");

return 0;
}

int connect(void *p){
	if(quit)return 0;
	IPaddress ip;
	while(SDLNet_ResolveHost(&ip,hostname,port)==-1)SDL_Delay(100);
	printf("Waiting for connection...\n");
	while(tcpsock==NULL){
		tcpsock=SDLNet_TCP_Open(&ip);
		//SDL_Delay(100);
	}
	connected=true;
	if(DEBUG)printf("Connection established.\n");
	recieve_thread=SDL_CreateThread(recieve,NULL);
	send(_REQUESTCONNECTION,ver);
	if(DEBUG)printf("Thread exited: Connector\n");
	return 0;
}


void closeconnection(bool reconnect){
	if(DEBUG)printf("Closing connections. Reconnect=%i\n",reconnect);
	SDL_KillThread(recieve_thread);
	SDLNet_TCP_Close(tcpsock);
	tcpsock=NULL;
    connected=false;
	authenticated=false;
	if(reconnect){
	connect_thread=SDL_CreateThread(connect,NULL);
	if(DEBUG)printf("Thread restarted: Connector\n");
	}
	// more to come
}

int tquitproc(void *p){
	exit(0);
	return 0;
}

void quitproc(void){
	static bool quitted;
	if(quitted) return;
	quitted=true;
	printf("Quitting...\n");
	quit=true;
	SDL_KillThread(input_thread);
	 
	if(DEBUG)printf("Closing Connection\n");
	closeconnection(false);


	printf("End.\n");
	
	//SDL_Delay(100);
	exit(0);
}

void forceclose(void){
//if(connected) disconnect();
quitproc();
}



int cmdhandler(void *p){
	while(true){
		if (incomingcmd!=_NODATA){
			switch(incomingcmd){
			//case _SENDTEXT:printf("%s\n",incomingtext);break; //bugged somehow
			case _PASSWORDOK:{printf("%s has accepted the connection.\n",remotever);authenticated=true;}break;
			case _PASSWORDWRONG:{printf("%s has rejected the connection, exiting...\n",remotever);SDL_Delay(2000);quitproc();}break;
			//case _DISCONNECT:{printf("Got disconnected from the remote side!\n");closeconnection(false);}break; // no longer needed
			case _REQUESTPASSWORD:
				strcpy(remotever,incomingtext);
				printf("%s requests a password.\n",remotever);
				if (nopassword){
					printf("Enter Password, max. 32 chars: ");
					Gets(password,32);
				}
				send(_SENDPASSWORD,password);
				printf("Password sent, waiting for answer...\n");
				break;
			case _RECIEVEDANDREADY:{printf("%s\n",incomingtext);ready=true;}break;
			case _RECIEVEDBUTUNKNOWN:{printf("The opcode was not recognized by the remote side.\n");ready=true;}break;
			case _UNKNOWNCOMMAND:{printf("Unknown command.\n");ready=true;}break;
			case _BADCOMMAND:{printf("Bad command: %s\n",incomingtext);ready=true;}break;
			default:{printf("Recieved unknown opcode!\n");}break;
			}
			incomingcmd=_NODATA;
		}
		SDL_Delay(1);
	}
return 0;
}


bool initproc(void){
printf("%s\n",ver);
if(DEBUG)printf("Allocating buffer memory...\n");

incomingtext=(char*)malloc(1024);
outgoingtext=(char*)malloc(1024);
password=(char*)malloc(32);
remotever=(char*)malloc(128);
// more malloc...

printf("Reading Config file... ");
if (readconf()) printf("done.\n"); else {
	printf("error!\n");
	//printf("Enter Password, max. 32 chars: ");
	//scanf("%s",&password);
}

if(DEBUG)printf("Initializing SDL...\n");
if(SDL_Init(0)==-1) {
		printf("SDL_Init: %s\n", SDL_GetError());
		return false;
	}

if(DEBUG)printf("Initializing SDL_Net...\n");
if(SDLNet_Init()==-1) {
	printf("SDLNet_Init: %s\n", SDLNet_GetError());
	return false;
}
if(DEBUG)printf("Starting threads...\n");

connect_thread=SDL_CreateThread(connect,NULL);
if(DEBUG)printf("Thread started: Connector\n");
cmdhandler_thread=SDL_CreateThread(cmdhandler,NULL);
if(DEBUG)printf("Thread started: CmdHandler\n");

return 1;
}

/*
int waitfortimeout(void *p){
	int timeout;
	for (timeout=10000;timeout>0||!ready;timeout--){
		SDL_Delay(1);
		if(timeout%1000==0) printf(".");
	}
	if(!ready) closeconnection(); // still not ready? disconnect!

return 0;
}
*/

void showhelp(void){
printf("commands - displays a list of commands the remote side supports\n");
printf("ver - show version\n");



}

bool IsLocalCommand(char *txt){
if(strcmp(outgoingtext,"help")==0){showhelp();return true;}
if(strcmp(outgoingtext,"ver")==0){printf("%s\n",ver);return true;}
if(strcmp(outgoingtext,"quit")==0||strcmp(outgoingtext,"exit")==0){quitproc();return true;}
if(strcmp(outgoingtext,"?")==0){showhelp();return true;}
return false;
}


int main(int argc, char* argv[])
{
	atexit(forceclose);

///!	printf("%c",(char)_SAYHELLO);
	if (!initproc()){
		printf("Initialisation failed!\n");
		return 1;
	} else {
		// more init...?
		printf("Initialisation finished.\n");
		while(!(authenticated&&connected)){
		


			SDL_Delay(1);
		}
		send(_RECIEVEDANDREADY);
		printf("Authentication ok, opening console. You can enter commands now.\n");
		printf("Type \"help\" for a short introduction\n");
		while(true){
			while(authenticated){
				SDL_Delay(1);
				
				if(ready){
					printf("> ");
					while(true){
						/*if(!waitforinput){
							SDL_CreateThread(tinput,NULL);
							SDL_Delay(10);
							while(waitforinput)SDL_Delay(1);
						}*/

					Gets(outgoingtext,MAXLEN);
					if(strlen(outgoingtext)<=0)printf("Maybe you want to enter something...?!\n> ");else break;
					SDL_Delay(1);
					}
					if (!IsLocalCommand(outgoingtext)){send(_SENDCOMMAND,outgoingtext);ready=false;}

				
					//SDL_CreateThread(waitfortimeout,NULL);
				}
				
			}
		SDL_Delay(1);


		}
	}
	
	return 0;
}

