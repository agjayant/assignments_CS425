/*Acknowledgement
 * Basic Setup : http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/
 * File Transfer: http://www.cs.put.poznan.pl/csobaniec/examples/sockets/
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUF 1024

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = PORT;

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     /*Getting Credentials*/
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);

     /*Parsing Credentials*/
     int userlen=0, passlen=0;
     char username[50], pswd[50];

     memset(username, '\0', sizeof(username));
     memset(pswd, '\0', sizeof(pswd));
     for(int iter= 0; iter < strlen(buffer); iter++){
         if(buffer[iter] == ':'){
             userlen = iter ;
         }
     }
     passlen = strlen(buffer) - userlen -1;
     strncpy(username, buffer, userlen);
     strncpy(pswd, buffer+userlen+1, passlen);

     /* Authenticating...*/
     FILE* file = fopen("users.txt", "r");
     char line[256];

     char newname[50], newpswd[50];
     int templen = 0;
     int authDone = 0;
     char ack[] = "Hello ";
     while (fgets(line, sizeof(line), file)) {
            memset(newname, '\0', sizeof(newname));
            for(int iter= 0; iter < strlen(line); iter++){
                if(line[iter] == ':'){
                    templen = iter ;
                }
            }
            strncpy(newname, line, templen);
            if (strcmp(newname, username)){
                continue;
            }
            memset(newpswd, '\0', sizeof(newpswd));
            strncpy(newpswd, line+templen+1, strlen(line)-templen-2);

            if(strcmp(newpswd, pswd)){
                char auth[] = "Authentication Failure!!!";
                n = write(newsockfd,auth,strlen(auth));
                if (n < 0) error("ERROR writing to socket");
                break;
            }

            /*Authentication Successful */
            strcat(ack, username);
            n = write(newsockfd,ack,strlen(ack));
            if (n < 0) error("ERROR writing to socket");
            authDone =1;
            break;
     }

     fclose(file);

     if(authDone){

        /*Getting Filename*/
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("Filename: %s\n",buffer);
        char filename[50];

        memset(filename, '\0', sizeof(filename));
        strncpy(filename, buffer, strlen(buffer)-1);

        /*Sending File (if present)*/
       int file = open(filename, O_RDONLY); 
       char buf[MAX_BUF];
       char* bufptr;
       if (file != -1){

            /*Sending file search indicator*/
            char mes[] = "1";
            n = write(newsockfd,mes,strlen(mes));
            if (n < 0) error("ERROR writing to socket");

            /*printf("File is present!!\n");*/
            int count_r, count_w;
            while((count_r = read(file, buf, MAX_BUF))>0)
            {
                count_w = 0;
                bufptr = buf;
                while (count_w < count_r)
                {
                    count_r -= count_w;
                    bufptr += count_w;
                    count_w = write(newsockfd, bufptr, count_r);
                    if (count_w == -1) 
                    {
                        perror("Socket read error");
                        exit(1);
                    }
                }
            }
            printf("File Sent\n");

            close(file);
       }
       else{
            /*Sending file search indicator*/
            char mes[]="0";
            n = write(newsockfd,mes,strlen(mes));
            if (n < 0) error("ERROR writing to socket");
       }
  
     }
     close(newsockfd);
     close(sockfd);

     return 0; 
}
