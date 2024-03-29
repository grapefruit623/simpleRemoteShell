/*
 * =====================================================================================
 *
 *       Filename:  hw2.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年04月21日 17時12分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  grapefruit623 (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "func.h"


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
		int bindId, acceptId, forkId;
		int i, n, maxi, maxfd, listenfd, sockfd;
		int nready, client[FD_SETSIZE];
		fd_set rset, allset;
		char buf[BUFFSIZE];
		char ip[64];


		printf ( "The server is running\n" );


		if ( !socketId ) {
				fprintf(stderr, "socket failed");
		}
		
		bzero(&myAddr, sizeof(myAddr));
		myAddr.sin_family = AF_INET;
		myAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any ip for the host */
		myAddr.sin_port = htons(port);

		bindId = bind( socketId, (struct sockaddr *)&myAddr, sizeof(myAddr));
		
		if ( 0 > bindId  ) {
				fprintf(stderr, "bind failed\n");
				return 0;
		}

		if ( 0 > listen( socketId, 10) ) {
				fprintf(stderr, "listening failed\n");
				return 0;
		}
		maxfd = socketId;
		maxi = -1;

		for ( i=0; i<FD_SETSIZE ; i++ ) {
				client[i] = -1;
		}
		FD_ZERO(&allset);
		FD_SET(socketId, &allset);

		initial();                              /* to initial setting */
		for ( ; ; ) {
				rset = allset;
				nready = select(maxfd+1, &rset, NULL, NULL, NULL);
				if ( FD_ISSET(socketId, &rset) ) {
						acceptId = accept(socketId, (struct sockaddr *)&clientInfo, &len);
						strcpy(ip, inet_ntoa(clientInfo.sin_addr));
						printf ( "new client: %s\n", ip );


						for ( i=0; i<FD_SETSIZE ; i++ ) {

								if ( client[i] < 0 ) {
										client[i] = acceptId;
										break;
								}
						}

						if ( FD_SETSIZE == i )
								fprintf(stderr, "too many clients");

						FD_SET(acceptId, &allset);
						if ( acceptId > maxfd )
								maxfd = acceptId;

						if ( i > maxi )
								maxi = i;

						if ( 0 >= --nready )
								continue;
				}
				for ( i=0; i <= maxi ; i++ ) {
						if ( 0 > ( sockfd = client[i] ) )
								continue;

						if ( FD_ISSET( sockfd, &rset) ) {
								bzero(buf, BUFFSIZE);
								if ( 0 == ( n = read(sockfd, buf, BUFFSIZE) ) ) {
										strcpy(buf, "logout");
										buf[strlen(buf)] = '\0';
										requestHandler(sockfd, buf);
										close(sockfd);
										FD_CLR(sockfd, &allset);
										client[i] = -1;
								}
								else {
										buf[strlen(buf)] = '\0';
										requestHandler(sockfd, buf);
								}

								if ( 0 >= --nready )
										break;
						}
				}
		}
		
		close(socketId);	
		close(acceptId);
		return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
