#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#define MYPORT 15635
#define BACKLOG 10 
int main(int argc, char *argv[])
{
    int sin_size;
    struct sockaddr {
        unsigned short sa_family;
        char sa_data[14];
    };
    struct sockaddr_in {
        short sin_family;
        ushort sin_port;
        struct in_addr sin_addr;
        unsigned char sin_zero[8];
    };
    int socketfd, newfd; 
    socketfd = socket(PF_INET,SOCK_STREAM,0);
    if (socketfd == -1) { 
        perror("Error"); 
        exit(1);
    
    }
    struct sockaddr_in myaddr; 
    struct sockaddr_in theiraddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(MYPORT); 
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(myaddr.sin_zero, '\0', sizeof(myaddr.sin_zero));
    
    /* bind the socket */
    if (bind(socketfd, (struct sockaddr *) &myaddr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1); 
    }

    /* listen */
    if (listen(socketfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while (1) { /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((newfd = accept(socketfd, (struct sockaddr*) &theiraddr, &sin_size)) == -1){
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n",inet_ntoa(theiraddr.sin_addr));
        close(newfd);
    }
    return 0;
}



//parse http request to get filename 
void parse_HTTP(int fd)
{
    char delim = " "; 
	char buf[1024];
	memset(buf,0,1024);
	char* filename = NULL;
	if (read(fd,buf,1024) == -1){
		print("Invalid HTTP request");
        exit(1);
	}
    //split http request by whitespace
    filename = strtok(buf, delim); // tokenize the C string
    filename = strtok(NULL, delim); // continue to tokenize the string
    //extract second token = filename
	filename++;

    //send message
    
    
}

