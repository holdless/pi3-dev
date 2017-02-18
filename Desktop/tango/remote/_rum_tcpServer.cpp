#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>

#include <sys/types.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr

using namespace std;

// global variables
bool cam_end(false), frame_in(false);
// pthread init
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *tcp_server(void *arg)
{
    int srv_socket , client_socket , c;
    struct sockaddr_in server , client;
    char *message;
	char buffer[256];
	int n,m;
     
    //Create socket
    srv_socket = socket(AF_INET , SOCK_STREAM , 0);
    if (srv_socket == -1)
    {
        printf("Could not create socket");
    }
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 6969 );
     
    //Bind
    if( bind(srv_socket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("bind failed");
        return NULL;
    }
    puts("bind done");
     
    //Listen
    listen(srv_socket , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    client_socket = accept(srv_socket, (struct sockaddr *)&client, (socklen_t*)&c); // <-- "accept" will block to wait

    if (client_socket<0)
    {
        perror("accept failed");
        return NULL;
    }
    puts("Connection accepted");
    
	// 642.2.1011 hiroshi: receive data from client <-- block here untill read something from client
	while(n = read(client_socket, buffer, sizeof(buffer)) > 0)
	{
		printf("Here is the message: %s\n",buffer);
		memset(buffer, 0x0, sizeof(buffer));
	 
        //Reply to the client
        message = "received msg: ";
        m = write(client_socket , message , strlen(message));
			if (m < 0) error("ERROR writing header to socket");
        m = write(client_socket , buffer , strlen(buffer));
			if (m < 0) error("ERROR writing content to socket");
    }
 
	if (n == 0)
	{		
		puts("client disconnected");
		fflush(stdout);
	}
	if (n < 0) 
		error("ERROR reading from socket");
 
    close(client_socket);
    close(srv_socket);

    return NULL;
}

void *camera_buffer_callback(void *arg) 
{
	for (int i = 0; i < *(int*)arg; i++)
	{
		pthread_mutex_lock(&mut);
		frame_in = true;
		pthread_cond_signal(&cond);	
		cout<<"frame:"<<i+1<<endl;
		pthread_mutex_unlock(&mut);
		sleep(3);
	}
	
	cam_end = true;
	return NULL;
}

void *image_proc(void *arg) 
{
//	struct timeval now;
//	struct timespec outtime;
	
	while (!cam_end)
	{
		pthread_mutex_lock(&mut);
		while (!frame_in) // using "while" because of "spurious wakeup" from cpu 
		{	
			cout<<"wait..."<<endl;
			pthread_cond_wait(&cond, &mut);
		}
		cout<<"img proc..."<<endl;
		frame_in = false;
		pthread_mutex_unlock(&mut);
		sleep(1);
	}
	
	return NULL;
}

void *talk(void *arg)
{
	int a = 0;
	while(!cam_end)
	{
		cin>>a;
		cout<<"[talk] your input: "<<a<<endl;
	}
	return NULL;
}

int main()
{
	pthread_t cam_thread;
	pthread_t img_thread;
	pthread_t talk_thread;
	pthread_t srv_thread;
	
	int loop = 10;
	
	//// pthread create ////
	// create camera_buffer_callback thread
	if ( pthread_create( &srv_thread, NULL, tcp_server, NULL) ) {
		printf("error creating srv_thread.");
		abort();
	}
	// create camera_buffer_callback thread
	if ( pthread_create( &cam_thread, NULL, camera_buffer_callback, (void*)&loop) ) {
		printf("error creating cam_thread.");
		abort();
	}
	// create image_proc thread
	if ( pthread_create( &img_thread, NULL, image_proc, NULL) ) {
		printf("error creating img_thread.");
		abort();
	}
	// create talk thread
	if ( pthread_create( &talk_thread, NULL, talk, NULL) ) {
		printf("error creating img_thread.");
		abort();
	}

	//// pthread join ///
	if ( pthread_join ( srv_thread, NULL ) ) 
	{
		printf("error joining srv_thread.");
		abort();
	}
	if ( pthread_join ( cam_thread, NULL ) ) 
	{
		printf("error joining cam_thread.");
		abort();
	}
	if ( pthread_join ( img_thread, NULL ) ) 
	{
		printf("error joining img_thread.");
		abort();
	}
	if ( pthread_join ( talk_thread, NULL ) ) 
	{
		printf("error joining talk_thread.");
		abort();
	}
	
	return 0;
}