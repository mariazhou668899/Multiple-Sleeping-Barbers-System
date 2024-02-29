// The client will create a new socket, connect to the server and send data
// by using 3 different ways of writing data (data transferring). 
// --- 1.	Multiple writes: invokes the write() system call for each data buffer, thus resulting 
//        in calling as many write()s as the number of data buffers
// --- 2.	writev: allocates an array of iovec data structures, each having its *iov_base field point
//        to a different data buffer as well as storing the buffer size in its iov_len field
// --- 3.	single write:allocates an nbufs-sized array of data buffers, and thereafter calls write() 
//        to send this array, (i.e., all data buffers) at once

// It will then wait for a response and output the response.

// The client will take the following six arguments: 

// --- serverName: server name

// --- port: a server IP port

// --- repetition: the repetition of sending a set of data buffers

// --- nbufs: the number of data buffers

// --- bufsize: the size of each data buffer (in bytes)

// --- type: the type of transfer scenario: 1, 2, or 3



#include <stdio.h>
#include <stdlib.h>
#include <strings.h>      // bzero
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl. htons. inet_ntoa
#include <sys/types.h>    // socket, bind
#include <sys/time.h>     // gettimeofday
#include <sys/uio.h>      // writev
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <arpa/inet.h>    // inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR

using namespace std;


const int BUFSIZE = 1500;



int main(int argc, char const *argv[])

{
  // Parse arguments
  const char *serverName=argv[1];
  int port = atoi(argv[2]);
  int repetition = atoi(argv[3]);
  int nbufs = atoi(argv[4]);
  int bufsize = atoi(argv[5]);
  int type = atoi(argv[6]);
  
  // Define argments for tracing time  
  struct timeval start;
  struct timeval stop;
  struct timeval lap;
  long transfer_time;
  long total_time;
  
  
  // get host
  struct hostent* serverHost = gethostbyname(serverName);
  if (serverHost == NULL) {
      fprintf(stderr, "Failed to resolve the server name.\n");
      return -1;
  }

  // Open a stream-oriented socket with the Internet address family
  int clientSD = socket(AF_INET, SOCK_STREAM, 0);
  if(clientSD < 0)

    {
      perror("Error: creating socket");
      close(clientSD);
      printf("\n"); 
      exit(EXIT_FAILURE);
    }
  
  // Declare a sockaddr_in structure, zero-initialize it by calling b_zero, and set its data members as follows:
  struct sockaddr_in sendSockAddr;
  bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
  sendSockAddr.sin_family =  AF_INET; // Address Family Internet
  sendSockAddr.sin_port = htons(port);
  sendSockAddr.sin_addr.s_addr=*((in_addr_t*)serverHost->h_addr);

  // Connect this socket to the server by calling connect as passing the following arguements: 
  // the socket descripto, the sockaddr_in structure defined above, and its data size(obtained from the sizeof() function
  int connectStatus = connect(clientSD, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));

  if(connectStatus < 0)

    {
      perror("Error: Failed to connect to the server~~");
      close(clientSD);
      printf("\n");      
      exit(EXIT_FAILURE);
    }

  // Allocate databuf[nbufs][bufsize] where nbufs * bufsize = 1500
  char databuf[nbufs][bufsize];

  // Start a timer by calling gettimeofday
  gettimeofday(&start, NULL); // no timezone specified

	//char vector_buf = malloc(sizeof(char * nbufs *bufsize));
  //start doing the write portion
  struct iovec vector[nbufs];

  for (int i = 0; i < repetition; i++)
    {
      if(type == 1) // multiple writes
				{
	  		 for (int j = 0; j < nbufs; j++)
	        {
	          write(clientSD, databuf[j], bufsize); // sd: socket descriptor
	        }
	     }
      else if(type == 2) // writev
	     {
	       for (int j = 0; j < nbufs; j++)
	        {
	          vector[j].iov_base = databuf[j];
	          vector[j].iov_len = bufsize;
	        }
	        writev(clientSD, vector, nbufs); // sd: socket descriptor
	     }
      else // type 3 implied, single write;
    	{
	      write(clientSD, databuf, nbufs * bufsize);
	    }
    }

  // Lap the timer by calling gettimeofday, where lap - start = data-sending time
  gettimeofday(&lap, NULL); // no timezone specified

  // Receive from the server an integer acknowledgement that shows how many times the server called read()
  int read_count = 0;
  read(clientSD, &read_count, sizeof(read_count));

  // Stop the timer by calling gettimeofday,where stop - start = tround-trip time
  gettimeofday(&stop, NULL); // no timezone specified
 
  // Calculate the data-sending time in microseconds
  long dataSendingTime = (lap.tv_sec - start.tv_sec) * 1000000 + (lap.tv_usec - start.tv_usec);
  
  // Calculate the round-trip time in microseconds
  long roundTripTime = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec);
  
  // Calculate the throughput in Gbps
  double totalDataSent = repetition * nbufs * bufsize * 8.0;  // Total data sent in bits
  double dataSendingTimeInSeconds = dataSendingTime / 1000000.0;  // Convert time to seconds
  double throughput = totalDataSent / (dataSendingTimeInSeconds) / 1000000000;  // Gbps

  // Print out the statistics. 
  printf("%d*%d:type %d, round-trip time = %ld usec, #reads = %d, throughput = %.4f Gbps\n",
  nbufs, bufsize, type, roundTripTime, read_count, throughput);
  

  // Close the socket by calling close
  close(clientSD);

  return 0;

}
