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
#define BUFFER_SIZE 1024

char* format_type_http(char* filetype);
void create_response(char* filename, char* type, int fd, FILE* file);
void respond(int fd);
char* parse_blank_space(char* filename);

int main(int argc, char *argv[])
{
    int addrlen;
    struct sockaddr {
        unsigned short sa_family;   // addr family, AF_xxx
        char sa_data[14]; // 14 bytes of proto addr
    };
    struct sockaddr_in { // used for IPv4 only
        short sin_family; // addr family, AF_INET 
        unsigned short sin_port; // port number
        struct in_addr sin_addr; // internet address 
        unsigned char sin_zero[8]; // zeros, same size as sockaddr
    };
    struct in_addr { // used for IPv4 only
        uint32_t sin_port; // 32-bit IPv4 address 
    };
    int socketfd, newfd; 

    /* create a socket */
    socketfd = socket(PF_INET, SOCK_STREAM, 0); // SOCK_STREAM for TCP (with PF_INET)
    if (socketfd == -1) { 
        perror("Error"); 
        exit(1);
    }
    struct sockaddr_in myaddr; 
    struct sockaddr_in client_addr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(MYPORT); 
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(myaddr.sin_zero, '\0', sizeof(myaddr.sin_zero));
    
    /* bind the socket */
    if (bind(socketfd, (struct sockaddr *) &myaddr, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        exit(1); 
    }

    /* listen */
    if (listen(socketfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while (1) { /* main accept() loop */
        /* accept a new connection */
        if ((newfd = accept(socketfd, (struct sockaddr*) &client_addr, &addrlen)) == -1){
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n",inet_ntoa(client_addr.sin_addr));
        
        respond(newfd);

        close(newfd);
    }
    return 0;
}



//parse http request to get filename 
void respond(int fd)
{
    printf("begin");
    //char delim = " "; 
	char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);
	char* filename = NULL;
    printf("buffer initializations"); 

	if (read(fd,buf,BUFFER_SIZE) == -1){
		printf("Invalid HTTP request");
        exit(1);
	}
    printf("read");
    //split http request by whitespace
    filename = strtok(buf, " "); // tokenize the C string
    filename = strtok(NULL, " "); // continue to tokenize the string
    //extract second token
	filename++;  // filename is now a pointer to the second token
    char* parsed_filename = parse_blank_space(filename);
    printf("the filename is %s", parsed_filename);
    
    //send message
    char* errbuf = "404 Not Found";
    FILE* file = fopen(parsed_filename,"r");
    //handling empty file
	if (file == NULL){
		write(fd,errbuf,sizeof(errbuf));
		return;
	}
    
	//find type 
    char *ext = NULL;
    char* filetype = NULL;
    ext = strrchr(filename, '.');
    if(ext != NULL){
        filetype = ext + 1;
    }
    if (filetype != NULL) {
        printf("the filetype is %s", filetype);
    }

    create_response(parsed_filename, filetype, fd, file);

    // header:  HTTP version, status code, content type, and content length are required. 
    
}

void create_response(char* filename, char* type, int fd, FILE* file){
    int LINE_SIZE = 128;
    char* status = "HTTP/1.1 200 OK\r\n";
    char* connection = "Connection: close\r\n";
    char date[LINE_SIZE];
    memset(date, 0, LINE_SIZE);
    char* server = "Server: Apache/2.2.3 (CentOS)\r\n";
    char last_modified[LINE_SIZE];
    memset(last_modified, 0, LINE_SIZE);
    char content_length[LINE_SIZE];
    memset(content_length, 0, LINE_SIZE);
    char content_type[LINE_SIZE];
    memset(content_type, 0, LINE_SIZE);


    // date 
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", &tm);

    // last-modified
    struct tm last_modified_time;
	struct stat st;
	stat(filename,&st);
	last_modified_time = *gmtime(&(st.st_mtime));
	strftime(last_modified, sizeof(last_modified),"Last-Modified: %a, %d %b %Y %H:%M:%S %Z\r\n", &last_modified_time);

    // content_length
	snprintf(content_length,sizeof(content_length),"Content-Length: %lu\r\n",st.st_size);

    // content-type
    snprintf(content_type,sizeof(content_type),format_type_http(type) );

    char header[1000];
    strcpy(header, status);
    strcat(header, connection);
    strcat(header, date);
    strcat(header, server);
    strcat(header, last_modified);
    strcat(header, content_length);
    strcat(header, content_type);
    strcat(header,"\r\n");
	printf("HTTP response message:\r\n\r\n%s",header);
    write(fd, header, strlen(header));

    if(type == NULL) { // binary file
        fclose(file);
        FILE* binary_fd = fopen(filename, "rb");
        unsigned char buffer[st.st_size + 1];
        memset(buffer, 0, st.st_size + 1);
        if(fread(buffer,sizeof(buffer),1,binary_fd) == -1){
            perror(strerror(errno));
            exit(1);
        }
        write(fd, buffer, st.st_size + 1);
        fclose(binary_fd);
    }
    else{
        unsigned char buffer[st.st_size + 1];
        memset(buffer, 0, st.st_size + 1);
        if(fread(buffer,sizeof(buffer),1,file) == -1){
            perror(strerror(errno));
            exit(1);
        }
        write(fd, buffer, st.st_size + 1);
        fclose(file);
    }
 
}
char* format_type_http(char* filetype)
{ 
    if(filetype == NULL){
        return "Content-Type: application/octet-stream\r\n";
    }
    if (strcmp(filetype,"png") == 0){ 
        return "Content-Type: image/png\r\n";
    }
    else if (strcmp(filetype,"html") == 0){ 
        return "Content-Type: text/html\r\n";
    }
    else if (strcmp(filetype,"txt") == 0){ 
        return "Content-Type: text/plain\r\n";
    }
    else if (strcmp(filetype,"jpg") == 0){ 
		return "Content-Type: image/jpeg\r\n";
    }
    else if (strcmp(filetype,"gif") == 0){ 
        return "Content-Type: image/gif\r\n";
    }
    else { 
		return "Content-Type: application/octet-stream\r\n";
    }

}

char* parse_blank_space(char* filename)
{
    // char ret[strlen(filename) + 1]; 
    // memset(ret, 0, strlen(filename) + 1);
    // int i; 
    // int j = 0; 
    // for(i = 0; i < strlen(filename) + 1; i++) { 
    //     if (filename[i] != '%'){ 
    //         ret[j] = filename[i];
    //     }
    //     else if (i + 2 < strlen(filename) && filename[i+ 1] == '2' && filename[i+2] == '0'){
    //         ret[i] = ' ';
    //         i = i + 2;
    //     }
    //     j++ ;
    // }
    // return ret; 

    char* p;
    int start = 0;
    char newbuf[256];
    memset(newbuf, 0, strlen(newbuf)+1);
    char* blank = " ";
    int flag = 0;
    for (p = filename; *p != '\0'; p+=1) {
        if(*p != '%'){
            if(flag==1){
                strncat(newbuf, p, 1);
            }else{
                strncpy(newbuf, p, 1);
                flag = 1;
            }
        }else{
            if(*(p+1) == '2' && *(p+2) == '0'){
                strcat(newbuf, blank);
                p+=2;
            }
        }  
    }
    return newbuf;  
}
