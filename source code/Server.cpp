// The server will create a tcp Socket that listens on a port(the last 4
// digits of your ID number. The server will accept an incoming connection
// and then create a new thread(use the pthreads lobrary) that will handle
// the connection. The new thread will read all the data from the client
// and respond back to it (acknowledgement). The response detail will be
// provided in Server.cpp

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>      // bzero
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <sys/types.h>    // socket, bind
#include <sys/time.h>     // gettimeofday
#include <sys/uio.h>      // writev
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <arpa/inet.h>    // inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <pthread.h>

const int BUFSIZE = 1500;
int serverSD;
int repetition;
using namespace std;


// The server must include your_function, which is the function called
// by the new thread.
void *count_reads(void *ptr)
{

  int sd = *(int *)ptr;
  int repetition = 20000;

  
  // Allocate databuf[BUFSIZE] where BUFSIZE = 1500
  char databuf[BUFSIZE];
  
  // Start a timer by calling gettimeofday.
  timeval start;
  gettimeofday(&start, NULL);

  int count = 0;
  for (int reps = 0; reps < repetition; reps++)

  {

    // Repeat reading data from the client into databuf[BUFSIZE]. 
    // Note that the read system call may return without reading the entire data if the network is slow.
    int remaining = BUFSIZE;

    while(remaining > 0)

    {
      int bytesRead = read(sd, databuf, remaining);
      if (bytesRead == -1)
      {
        printf("read failed\n");
        exit(1);
      }

      count += 1;
      remaining -= bytesRead;
    }

  }

  // Stop the timer by calling gettimeofday
  timeval end;
  gettimeofday(&end, NULL);

  // Send the number of read calls made, as an acknowledgement.
  int written = 0;
  while (written < sizeof(count))

  {

    int bytesWritten = write(sd, &count, sizeof(count));
    if (bytesWritten == -1)

    {

      printf("write failed\n");
      exit(1);

    }
    written += bytesWritten;

  }

  if (close(sd) != 0)

  {
      printf("close failed\n");
      exit(1);
  }

  // Print out the statistics as shown below:
  printf("data-receiving time = %ld usec\n", (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
  fflush(stdout);
  return NULL; 
  
}


int main(int argc, char const *argv[])
{
  // Make sure we got the right number of parameters
  if (argc != 2)
  {
    printf("the number of parameters is invalid\n");
    return -1;
  }
  
  // Parse parameters
  int port = atoi(argv[1]);
    if(port <= 1024)
    {
      printf("port must be a positive number greater than 1024\n");
      return -1;
    }
  
  int repetition = 20000;
  
  int n_connection_rq = 8; // Supposed to be 8

  // Declare a sockaddr_in structure, zero-initialize it by calling b_zero, and set its data members as follows:
  sockaddr_in acceptSockAddr;
  bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr));
  acceptSockAddr.sin_family = AF_INET; // Address family Internet
  acceptSockAddr.sin_port = htons(port);
  acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Open a stream-oriented socket with the Internet address family
  serverSD = socket(AF_INET, SOCK_STREAM, 0);
  if(serverSD == -1){
    printf("socket failed to create file descriptor\n");
    return -1;    
  
  }

  // Set the SO-REUSEADDR options
  const int on = 1;
  int setSoketpt = setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));
  if(setSoketpt == -1){
    printf("setsockopt failed\n");  
    return -1;    
  }  

  // Bind this socket to its local address by calling bind as passing the following arguement: 
  // the socket descriptor, the sockaddr_in structure defined above, and its data size  
  int isBind=bind(serverSD, (sockaddr*)&acceptSockAddr, sizeof(acceptSockAddr));
  if(isBind==-1){
    printf("bind failed\n");  
    return -1;    
  }

  // Instruct the operating system to listen up to n connection requests from clients at a time by calling listen
  int isListen=listen(serverSD, n_connection_rq);
  if(isListen==-1){
    printf("listen failed\n");  
    return -1;    
  }

  // Recieve a request from a client by calling accept that will return a new socket specific to this connection request
  sockaddr_in newSockAddr;
  socklen_t newSockAddrSize = sizeof(newSockAddr);
  int newSd = accept(serverSD, (sockaddr*)&newSockAddr, &newSockAddrSize);
  
  printf("Server: accepted connection\n");
  fflush(stdout);
  while (newSd != -1)
  {
    //Create a new thread
    int* data = new int[1];
    data[0] = newSd;

    pthread_t thread;
    if (pthread_create(&thread, NULL, count_reads, (void *)data) != 0)
    {
      delete data;
      printf("pthread_create failed\n");
      return -1;
    }
    
    // Merge threads back to avoid wasting resources
    pthread_join(thread, NULL);

    // Loop back to the accept command and wait for a new connection
    newSd = accept(serverSD, (sockaddr*)&newSockAddr, &newSockAddrSize);
  }

  if(newSd == -1){
    printf("accepted failed\n");
    return -1;  
    
  }

  return 0;
}