#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 100
#define BACKLOG 5
#define PORT 49150
#define MSG_SIZE 256

int main(void) {
  struct sockaddr_in srvAddr;
  struct sockaddr_in clntAddr;
  int sfd, cfd;
  ssize_t numRead;
  char buf[BUF_SIZE];
  char msg[MSG_SIZE];
  int total = 0, count = 0;
  socklen_t AddrLen = sizeof(clntAddr);

  /* Server software using STREAM SOCKETS (TCP) stages
    a. socket() -> creates a STREAM socket (also called TCP socket if AF_INET domain is used)
    b. bind() -> binds a STREAM socket to an identifier (internet layer address plus transport 
	   layer address) 
    c. listen() -> makes the socket a listening for new connections socket
    d. accept() -> Server software waits (blocks) for a new TCP connection request 
	   from a client (is called TCP connection if AF_INET domain is used)
    e. send (i.e. write()) and recieve (i.e. read()) messages to/from client software
    f. close() closes the socket

    Note: use 2nd section of manual pages for each function in code e.g. man 2 socket
  */

  /*
    int sfd = socket(domain, type, protocol)

    sfd: socket descriptor, an integer (like a file-handle)
    domain: integer, specifies communication domain. We use 
      AF_ LOCAL: for communication between processes on the same host. 
      AF_INET: communicating between processes on different hosts using IPv4 protocol
      AF_INET6: communicating between processes on different hosts using IPv6 protocol
    type: communication type
      SOCK_STREAM: if used with AF_INET domain, it will be a TCP socket
      SOCK_DGRAM:  if used with AF_INET domain, it will be a UDP socket
    protocol: Normally only a single protocol exists to support a particular socket type within a given protocol family,
              in which case protocol can be specified as 0. (In this example, IPv4 protocol)
  */
  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    perror("socket");
    exit(1);
  }

  /* fills memory area pointed to by srvAddr with 0 */
  memset(&srvAddr, 0, sizeof(struct sockaddr_in));

  srvAddr.sin_family = AF_INET;
  srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  srvAddr.sin_port = htons(PORT);

  /*
    int bind(int sfd, const struct sockaddr *srvAddr, socklen_t addrlen);

    After creation of socket, bind function binds the socket to the IPv4 address AND port number specified in srvAddr data structure.
    With the use of INADDR_ANY, socket sfd is binded to any IPv4 address of host running the server software.
    Also, socket sfd is binded to 49150 TCP port number which must be free at the time you start the server software. 49150 is in 
    REGISTERED PORTs range (1024 - 49150), which is used for server software a user without administrator rigths can start in OS
  */
  if (bind(sfd, (struct sockaddr *) &srvAddr, sizeof(struct sockaddr_in)) == -1) {
    perror("bind");
    close(sfd);
    exit(1);
  }

  /*
    int listen(int sfd, int backlog);

    Puts the server socket in a passive mode, where it waits for the client to approach the server to make a connection. 
    The backlog, defines the maximum length to which the queue of pending connections for sfd may grow. If a connection 
    request arrives when the queue is full, the client may receive an error with an indication of ECONNREFUSED.
  */
  if (listen(sfd, BACKLOG) == -1) {
    perror("listen");
    close(sfd);
    exit(1);
  }

  for (;;) {
    memset(&clntAddr, 0, sizeof(struct sockaddr_in));

    /*
      int cfd = accept(int sfd, struct sockaddr *srvAddr, socklen_t *addrlen);

      It extracts the first connection request on the queue of pending connections for the listening sfd socket, 
      creates a new connected socket, and returns a new file descriptor referring to that socket. At this point, the connection
      is established between client and server, and they are ready to transfer data
    */
    cfd = accept(sfd, (struct sockaddr *) &clntAddr, &AddrLen);

    printf("ACCPT CONN: %s:%d ...\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
    if (cfd == -1) {
      perror("accept");
      close(sfd);
      exit(1);
    }

    bzero(buf, BUF_SIZE);
    total = 0; count = 0;

    /* What server software is serving: Finds average of numbers sended by client */
    while ((numRead = read(cfd, buf, BUF_SIZE)) > 0 && strncmp(buf, "quit", 4) != 0) {
      total += atoi(buf);
      count++;

      bzero(msg, MSG_SIZE);
      snprintf(msg, MSG_SIZE, "%s:%d -> ", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
      write(STDOUT_FILENO, msg, MSG_SIZE);
      write(STDOUT_FILENO, buf, numRead);

      snprintf(buf, BUF_SIZE, "Partial avg is %f\n", (float)total / (float)count);

      /* write message to client */
      write(cfd, buf, strlen(buf));
    }

    if (numRead == -1) {
      perror("read");
      close(cfd);
      close(sfd);
      exit(1);
    }

    snprintf(buf, BUF_SIZE, "Avg in total is %f\n", (float)total / (float)count);

    /* write message to client */
    write(cfd, buf, strlen(buf));

    /* close client connection */
    if (close(cfd) == -1)
      perror("close");

    bzero(msg, MSG_SIZE);
    snprintf(msg, MSG_SIZE, "%s:%d -> closing connection\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
    write(STDOUT_FILENO, msg, MSG_SIZE);
  }
  if (close(sfd) == -1)
    perror("close");
  return 0;
}
