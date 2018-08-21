#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <limits.h>
#include "statistics.h"

#define BUFSIZE 1024

/*
 * UDP STATISTICS SERVER
 */

// ---- Struct & Functions declaration ----
struct TIME
{
    int seconds;
    int minutes;
    int hours;
};
void differenceBetweenTimePeriod(struct TIME t1, struct TIME t2, struct TIME *diff);

// ---- Aux functions ----
void error(char *msg)
{
    perror(msg);
    exit(1);
}
// -----------------------

int main(int argc, char **argv)
{
    //Socket vars:
	int sockfd;                                     //socket
    int portno;                                     //port to socket listening
    int clientlen;                                  //byte size of client address
    struct sockaddr_in serveraddr;                  //server address
    struct sockaddr_in clientaddr;                  //client address
    struct hostent *hostp;                          //client host info
    char *buf;                                      //message buf
    char *hostaddrp;                                //dotted decimal host addr string
    int optval;                                     //flag value for setsockopt
    int n;                                          //message byte size
    //Time vars:
    struct TIME pastTime, currentTime, diff;        //structs for calculate time diff between msgs
    struct tm *tm_struct;
    //Data vars:
    int accumulate = 1;
    struct dataperminute dataM = { .bytes = 0, .msgs = 0, .maxmsg = 0 };
    //File to export data, get current directory to export the txt with info:
    char cwd[PATH_MAX];                             //Directory for data files location
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        strcat(cwd, "/DataFiles");
        //Create directory if doesnt exist:
        struct stat st = {0};
        if (stat(cwd, &st) == -1){
            mkdir(cwd, 0700);
        }
        //Create the file to export data:
        char dateName[100];
        time_t currentT = time(NULL);
        struct tm *t = localtime(&currentT);
        strftime(dateName, sizeof(dateName)-1, "%d_%m_%Y_%H:%M:%S", t);
        strcat(cwd, "/");
        strcat(cwd, dateName);
        strcat(cwd, ".txt");
        printf("\nFile to export data: %s\n\n", cwd);
    } else {
        error("Error with current working dir!\n");
        return 1;
    }
    
    
    /*
     * check command line for port
     */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);
    
    //****************************** SOCKET ******************************
    /*
     * create server socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    /*
     * setsockopt:handy debugging trick that lets us rerun the server.
     * Eliminates ERROR on binding: address already in use error.
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    
    /*
     *  build server's Internet address
     */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    
    /*
     * bind: associate the parent socket with a port
     */
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        error("ERROR on binding");
    
    //initiate timestamps  
    time_t now = time(0);
    tm_struct = localtime(&now);
    pastTime.hours = tm_struct->tm_hour;
    pastTime.minutes = tm_struct->tm_min;
    pastTime.seconds = tm_struct->tm_sec;
    printf("%d:%d:%d", pastTime.hours, pastTime.minutes, pastTime.seconds);
    fflush(stdout);
    printf(" -STATISTICS SERVER- initiated ... listening on %d\n", portno);                  
    
    /*
     * main loop: wait for a datagram, then echo it
     */
    int firstrec = 0;
    clientlen = sizeof(clientaddr);
    while (1){
        
        /*
         * recvfrom: receive a UDP datagram
         */
        buf = malloc (BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);
        if (n < 0)
            error("ERROR in recvfrom");
            
        else{
            
            //------------- UDP DATAGRAM RECEIVED -------------            
            //Get Timestamp:
            time_t now = time(0);
            tm_struct = localtime(&now);
            currentTime.hours = tm_struct->tm_hour;
            currentTime.minutes = tm_struct->tm_min;
            currentTime.seconds = tm_struct->tm_sec;
            //printf("%d:%d:%d - datagram received (%d)\n", currentTime.hours, currentTime.minutes, currentTime.seconds, n);
            fflush(stdout);
            
            //If datagram in the same minute than last, data accumulate, else print minute resume:
            accumulate = 1;
            differenceBetweenTimePeriod(currentTime, pastTime, &diff);            
            if(diff.minutes > 0) accumulate = 0;
            
            //data accumulate:                    
            dataM.bytes = dataM.bytes + n;
            dataM.msgs = dataM.msgs + 1;
            if(dataM.maxmsg < n)
                dataM.maxmsg = n;                        
            
            //Minute data resume:
            if(accumulate == 0){
                 
                //Print minute info and reset it:                               
                fflush(stdout);
                if (firstrec == 0){
                    printf("..Receiving and exporting data.."); 
                    firstrec = 1;
                } else {
                    printf("."); 
                }                               
                fflush(stdout);
                
                int check = appendDataToFile(cwd, &dataM, diff.minutes);
                if (check == 0)
                    printf("ERROR writing data on file!\n");
                
                resetData(&dataM);
                
                //After resume minute info, currentTime is the past printed minute data:
                pastTime.hours = currentTime.hours;
                pastTime.minutes = currentTime.minutes;
                pastTime.seconds = currentTime.seconds;

            }                        
                        
        }
        
    }
    
	return 0;
}

//Function for time difference calculating:
void differenceBetweenTimePeriod(struct TIME t1, struct TIME t2, struct TIME *diff){
    
    if(t2.seconds > t1.seconds){
        --t1.minutes;
        t1.seconds += 60;
    }
    
    diff->seconds = t1.seconds - t2.seconds;
    if(t2.minutes > t1.minutes){
        --t1.hours;
        t1.minutes += 60;
    }
    
    diff->minutes = t1.minutes - t2.minutes;
    diff->hours = t1.hours -t2.hours;
}