
#include <sys/types.h>
#include <stdio.h>
#include<sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include  <getopt.h> 
#include<dirent.h>
#include<unistd.h>


/* Socket API headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Definations */
#define DEFAULT_BUFLEN 512
void a(int client_fd,const char* direct){
    DIR* dir;
    struct dirent* entry;
    struct stat file_stat;
    char buffer[DEFAULT_BUFLEN];
    
    dir=opendir(direct);
        if(dir==NULL){
            printf("Your directory could not be opened");
            return;
        }
    while((entry = readdir(dir))!=NULL){
        char filePath[DEFAULT_BUFLEN];
                snprintf(filePath,DEFAULT_BUFLEN,"%s/%s",direct,entry->d_name);
     if(stat(filePath,&file_stat)<0)
         continue;
       if(S_ISDIR(file_stat.st_mode))
           continue;
        snprintf(buffer,DEFAULT_BUFLEN,"%s %d bytes\n",entry->d_name,file_stat.st_size);
        send(client_fd,buffer,strlen(buffer),0);
    }

closedir(dir);
}
void do_job(int fd,const char* direct) {
int length,rcnt;
char recvbuf[DEFAULT_BUFLEN],bmsg[DEFAULT_BUFLEN];
int  recvbuflen = DEFAULT_BUFLEN;
  char b[]="Welcome to Ashwyne's server\n";
    send(fd,b,strlen(b),0);
    // Receive until the peer shuts down the connection
    do {
        rcnt = recv(fd, recvbuf, recvbuflen, 0);
        if (rcnt > 0) {
            printf("Bytes received: %d\n", rcnt);
            if(strncmp(recvbuf,"LIST",4)==0){
            a(fd,direct);
            }
         else if(strncmp(recvbuf,"GET",3)==0){
            char name[DEFAULT_BUFLEN];
             sscanf(recvbuf,"GET %s",name);
             char pathh[DEFAULT_BUFLEN];
             snprintf(pathh,DEFAULT_BUFLEN,"%s/%s",direct,name);
             FILE *file = fopen(pathh,"rb");
             if(file==NULL){
             char err[DEFAULT_BUFLEN];
             snprintf(err,DEFAULT_BUFLEN,"This file does not exist on server\n");
             send(fd,err,strlen(err),0);
             }
             else{
                char buff[DEFAULT_BUFLEN];
                 size_t bytes_read;
                 while((bytes_read=fread(buff,1,DEFAULT_BUFLEN,file))>0){
                 send(fd,buff,bytes_read,0);
                 }
                 fclose(file);
             } 
         
         }
            else if(strncmp(recvbuf,"QUIT",4)==0){
            char c[]="Server closing.......\nGoodbye!";
               send(fd,c,strlen(c),0);
                close(fd);
                printf("\n");
            } 
                else{
                printf("Wrong command please try another");
            }
        }

       
        else if (rcnt == 0)
            printf("Connection closing...\n");
        else  {
            printf("Receive failed:\n");
            close(fd);
            break;
        }
    } while (rcnt > 0);
}



int main(int argc,char *argv[])
    
{
int server, client;
struct sockaddr_in local_addr;
struct sockaddr_in remote_addr;
int length,fd,rcnt,optval;
pid_t pid;
    int str;
    int PORT=0;
    char *direct=NULL;
    while((str=getopt(argc,argv,"d:p:"))!=-1){
        switch(str){
            case 'd':
                direct=optarg;
                break;
                
            case 'p':
                PORT=atoi(optarg);
                break;
            default:
                fprintf(stderr,"value of port using -p",argv[0]);
                       return 1;
                           }
                       }
                       if (direct==NULL||PORT==0){
                    printf("Please provide all arguements\n");
                           return 1;
                           }
                

/* Open socket descriptor */
if ((server = socket( AF_INET, SOCK_STREAM, 0)) < 0 ) { 
    perror("Can't create socket!");
    return(1);
}


/* Fill local and remote address structure with zero */
memset( &local_addr, 0, sizeof(local_addr) );
memset( &remote_addr, 0, sizeof(remote_addr) );

/* Set values to local_addr structure */
local_addr.sin_family = AF_INET;
local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
local_addr.sin_port = htons(PORT);

// set SO_REUSEADDR on a socket to true (1):
optval = 1;
setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

if ( bind( server, (struct sockaddr *)&local_addr, sizeof(local_addr) ) < 0 )
{
    /* could not start server */
    perror("Bind error");
    return(1);
}

if ( listen( server, SOMAXCONN ) < 0 ) {
        perror("listen");
        exit(1);
}

printf("Ashwyne's server is Listening on port %d\n",PORT);


while(1) {  // main accept() loop
    length = sizeof remote_addr;
    if ((fd = accept(server, (struct sockaddr *)&remote_addr, \
          &length)) == -1) {
          perror("Accept Problem!");
          continue;
    }

    printf("Server: got connection from %s\n", \
            inet_ntoa(remote_addr.sin_addr));

    /* If fork create Child, take control over child and close on server side */
    if ((pid=fork()) == 0) {
        close(server);
        do_job(fd,direct);
        printf("Child finished their job!\n");
        close(fd);
        exit(0);
    }

}

// Final Cleanup
close(server);

}
