/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年04月27日 03時16分32秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  grapefruit623 (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#define	BUFSIZE 4096			/*  */

static struct termios stored_settings;

void echo_off(void)
{
    struct termios new_settings;

    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ECHO);
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void echo_on(void)
{

    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  str_cli
 *  Description:  
 * =====================================================================================
 */
		void
str_cli ( int out, int in, int sockfd )
{
		int n, maxfd, isPasswd = 0;
		fd_set rset;
		char sendLine[BUFSIZE], recvLine[BUFSIZE];

		bzero(recvLine, BUFSIZE);
		bzero(sendLine, BUFSIZE);

		FD_ZERO(&rset);

		write(sockfd, "connectReq", strlen("connectReq") ); /* send connect req to server */
		for ( ; ; ) {
				FD_SET(in, &rset);
				FD_SET(sockfd, &rset);
				maxfd = (sockfd>in)?(sockfd+1):(in+1);
				select(maxfd, &rset, NULL, NULL, NULL);

				if ( FD_ISSET(sockfd, &rset) ) {
						if ( 0 == read(sockfd, recvLine, BUFSIZE) ) {
								fprintf(stderr, "server terminated");
								return ;
						}

						if ( !strcmp(">>Password:", recvLine) )
								isPasswd = 1;
						write(out, recvLine, strlen(recvLine));
						if ( 1 == isPasswd )
								echo_off();
				}
				if ( FD_ISSET(in, &rset) ) {


						if ( 0 == read(in, sendLine, BUFSIZE) ) {
									return ;	
						}


						if ( 1 == isPasswd ) {
								echo_on();
								isPasswd = 0;
						}

						if ( !strcmp(sendLine, "\n") ) {
//								printf ( "n = %d\n", strlen(sendLine) );
								sendLine[1] = '\0';
						}
						else {
								sendLine[strlen(sendLine)-1] = '\0';           /* to elimate '\n' */
						}
						write(sockfd, sendLine, strlen(sendLine));
				}
				bzero(recvLine, BUFSIZE);
				bzero(sendLine, BUFSIZE);
		}
		return ;
}		/* -----  end of function str_cli  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
		int
main ( int argc, char *argv[] )
{
		struct sockaddr_in myAddr;
		struct sockaddr_in clientInfo;
		socklen_t len = sizeof(clientInfo);
		int socketId = socket(AF_INET, SOCK_STREAM, 0);
		int port = 8051;
		char mes[BUFSIZE] = "hello world";
		char recvBuf[BUFSIZE];


		printf ( "The client is running\n" );


		if ( !socketId ) {
				fprintf(stderr, "socket failed");
		}
		
		bzero(&myAddr, sizeof(myAddr));
		myAddr.sin_family = AF_INET;
		myAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any ip for the host */
		myAddr.sin_port = htons(port);
		if ( connect( socketId,(struct sockaddr*)&myAddr, sizeof(myAddr)  ) )
				fprintf(stderr, "connect fail\n");

		str_cli(fileno(stdout), fileno(stdin), socketId);


		return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
