/*Acknowledgement
 * Basic Setup : http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/
 * File Transfer: http://www.cs.put.poznan.pl/csobaniec/examples/sockets/
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#define PORT 8080
#define MAX_BUF 1024

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    char credentials[100], hostip[50];
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 2) {
       fprintf(stderr,"usage %s <username:password@hostip>\n", argv[0]);
       exit(0);
    }

    /* Credentials Parsing */
    int credlen = 0;
    char cred[150];
    memset(cred, '\0', sizeof(cred));
    memset(credentials, '\0', sizeof(credentials));
    memset(hostip, '\0', sizeof(hostip));
    strncpy(cred, argv[1], strlen(argv[1]));
    for(int iter= 0; iter < strlen(cred); iter++){
        if(cred[iter] == '@'){
            credlen = iter ;
        }
    }
    int hostlen = strlen(cred) - credlen- 1;
    strncpy(credentials, cred, credlen);
    strncpy(hostip, cred+credlen+1, hostlen);

    /* Connection Estabilishment  */
    portno = PORT ;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(hostip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    /* Sending Credentials */
    n = write(sockfd,credentials,strlen(credentials));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);

    /* Getting Authentication Result*/
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    /* Checking Authentication Result*/
    char check[5];
    memset(check, '\0', sizeof(check));
    strncpy(check, buffer, 5);

    /* Authentication Successful*/
    if (!strcmp(check, "Hello")){


        printf("Please enter the filename: ");
        bzero(buffer,256);
        fgets(buffer,255,stdin);

        /* Sending Filename*/
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
             error("ERROR writing to socket");

        /* Getting the indicator for file search*/
        int count;
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
             error("ERROR reading from socket");
        char temp[1];
        memset(temp, '\0', sizeof(temp));
        strncpy(temp, buffer, 1);
        char buf[MAX_BUF];
        if (temp[0] == '1'){
            
            printf("Got the File...");
    
            shutdown(sockfd, 1);
            while ((count = read(sockfd, buf, MAX_BUF))>0)
            {
                write(1, buf, count);
            }
            if (count == -1)
            {
                perror("Read error");
                exit(1);
            }

            close(sockfd);

        }
        else{

            printf("File Not Found\n");
        }
    }

    return 0;
}
