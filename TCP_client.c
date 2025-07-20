#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 100
#define PORT 49150
#define SRVIPv4ADDR "change this to IPv4 address of host running server software e.x 83.212.76.143"

int main(void) {
    struct sockaddr_in addr;
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];


    /*
    Client software using STREAM SOCKETS (TCP) stages
      a. socket()
      b. connct()
      c. send (i.e. write()) and recieve (i.e. read()) mesages to/from server software
      d. close()

      Note: use 2nd section of manual pages for each function appears in code e.g. man 2 socket and so on
    */

    sfd = socket(AF_INET, SOCK_STREAM, 0);      /* Create client socket */
    if (sfd == -1) {
      perror("socket");
      exit(1);
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT); 

    if (inet_pton(AF_INET, SRVIPv4ADDR, &addr.sin_addr) <= 0) {
      printf( "\nInvalid address/ Address not supported \n");
      return -1;
    }

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1) {
      perror("connect");
      exit(1);
    }

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
      if (write(sfd, buf, numRead) != numRead) {
        perror("partial/failed write");
        exit(1);
      }
      if (strncmp(buf, "quit", 4) == 0) {
        numRead = read(sfd, buf, BUF_SIZE);
        write(STDOUT_FILENO, buf, numRead);
        close(sfd);
        exit(EXIT_SUCCESS);
      }
      else {
        numRead = read(sfd, buf, BUF_SIZE);
        write(STDOUT_FILENO, buf, numRead);
      }
    }
    if (numRead == -1) {
            perror("read");
            exit(1);
    }
    exit(EXIT_SUCCESS);
}
