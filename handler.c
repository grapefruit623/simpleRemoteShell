/*
 * =====================================================================================
 *
 *       Filename:  handler.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年04月28日 21時02分49秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  grapefruit623 (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "func.h" 

User allUsers[userCanHandle];
NameAndPasswd legalUsers[userCanHandle];
int howManyUsers = 0;

const char yourName[BUFFSIZE] = ">>Login:";
const char yourPasswd[BUFFSIZE] = ">>Password:";
const char youAreNotUser[BUFFSIZE] = ">>There is no this account\n";
const char yourAreLogout[BUFFSIZE] = "Your Are Logout\n"; 
const char youAreOnline[BUFFSIZE] = "\n>>Your Are on-line, in cmd mode\n";
const char youHaveBeenOnline[BUFFSIZE] = "\nYou have been on-line\n";
const char cmdPrompt[BUFFSIZE] = ">>\0";

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sigFork
 *  Description:  to avoid zombile 
 * =====================================================================================
 */
		void
sigFork ( int sig )
{
		pid_t pid;
		int stat;
		pid = waitpid(-1, &stat, 0);
		printf("pid_t: %d , stat: %d\n", pid, stat);
		return ;
}		/* -----  end of function sigFork  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getAccount
 *  Description:  
 * =====================================================================================
 */
		void
getAccount ( )
{
		int fp, i;
		char account[BUFFSIZE];
		char *userList[BUFFSIZE], *userName, *userPasswd;

		fp = open("account/users.txt", O_RDWR);	
		read(fp, account, BUFFSIZE);
		userList[howManyUsers] = strtok(account, "\n"); 
		while ( NULL != userList[howManyUsers] ) {
				howManyUsers++;
				userList[howManyUsers] = strtok(NULL, "\n");
		}

		for ( i=0 ; i < howManyUsers ; i++ ) {
				userName = strtok( userList[i], ":"  );
				userPasswd = strtok( NULL, ":" );
				strcpy(legalUsers[i].name, userName);
				strcpy(legalUsers[i].passwd, userPasswd);
		}
		close(fp);
		return ;
}		/* -----  end of function getAccount  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  initial
 *  Description:  
 * =====================================================================================
 */
		void
initial ( )
{
		int i = 0;
		
		getAccount();
		for ( i = 0; i<userCanHandle ; i++ ) {
				allUsers[i].stage = offLine;
				allUsers[i].socket = -1;
				allUsers[i].serverCache = i ;
		}
		return ;
}		/* -----  end of function initial  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  requestHandler
 *  Description:  
 * =====================================================================================
 */
		int
requestHandler( int sockfd, char *incomingMes)
{
		char name[BUFFSIZE], passwd[BUFFSIZE], cmdBuf[BUFFSIZE];
		char cmd[BUFFSIZE], userSay[BUFFSIZE];
		int i = 0, j = 0;
		pid_t pid;
		int fd1[2], fd2[2];

		if ( offLine == allUsers[sockfd].stage ) { /* request client to input account */
				write(sockfd, yourName, strlen(yourName));
				allUsers[sockfd].stage = inputAccount;
				return 1;
		}
		if ( inputAccount == allUsers[sockfd].stage ) { /* it get account, now client should send passwd */
				printf ( "Now the incomingMes is account\n" );
				strcpy(name, incomingMes);
				write(sockfd, yourPasswd, strlen(yourPasswd));
				allUsers[sockfd].stage = inputPasswd; 
				return 1;
		}

		if ( inputPasswd == allUsers[sockfd].stage ) { /* to match account data */
				printf ( "Now your should input passwd\n" );
				strcpy(passwd, incomingMes);

				for ( i=0 ; i<howManyUsers ; i++ ) {

						if ( !strcmp(legalUsers[i].name, name) && !strcmp(legalUsers[i].passwd, passwd) ) { /* the user is exist on prebuilt file */
								printf ( "hello %s, %s\n", name, passwd );

								for ( j=0; j<userCanHandle ; j++ ) { /* check repeat log-in */

										if ( !strcmp( allUsers[j].name, name) && ( cmdMode == allUsers[j].stage )) {
												write(sockfd, youHaveBeenOnline, strlen(youHaveBeenOnline));
												return 1;
										}
								}
								write(sockfd, youAreOnline, strlen(youAreOnline));
								allUsers[sockfd].stage = cmdMode;
								strcpy(allUsers[sockfd].name, name);
								strcpy(allUsers[sockfd].passwd, passwd);
								allUsers[sockfd].socket = sockfd;
								allUsers[sockfd].serverCache = i; /* the index of user on the account data  */


								printf ( "%s %d\n", allUsers[sockfd].name, sockfd );
								return 1;
						}
				}
				allUsers[sockfd].stage = offLine;
				write(sockfd, youAreNotUser, strlen(youAreNotUser));
		}
		if ( cmdMode == allUsers[sockfd].stage ) { /* the cmd mode */

				if ( !strcmp("logout", incomingMes) ) {
						allUsers[sockfd].stage = offLine;
						write(sockfd, yourAreLogout, strlen(yourAreLogout));
						strcpy(allUsers[sockfd].name, "offLine");
						strcpy(allUsers[sockfd].passwd, "offLine");
				}

				if ( !strcmp("ls", incomingMes) ) {


						if ( 0 > pipe(fd1) || 0 > pipe(fd2) ) {
								fprintf(stderr, "pipe errer");
						}
						if ( 0 < ( pid = fork()) ) { /* parent */
								close(fd1[0]);
								close(fd2[1]);

								bzero(cmdBuf, BUFFSIZE);
								if ( 0 > read(fd2[0], cmdBuf, BUFFSIZE) ) {
										printf ( "read error in line 177\n" );
								}
								else {
										cmdBuf[strlen(cmdBuf)] = '\0';
										write(sockfd, cmdBuf, strlen(cmdBuf));
								}

								signal(SIGCHLD, sigFork);               /* to avoid zombile */
						}
						else {                  /* son */

								close(fd1[1]);
								close(fd2[0]);
								dup2(fd1[0], fileno(stdin));
								close(fd1[0]);
								dup2(fd2[1], fileno(stdout));
								close(fd2[1]);
								execlp("ls","ls" "-al", NULL );
//								if ( errno == ENOENT ) {
//										printf ( "%s, %d\n","ENOENT", errno );
//										perror("err: ");
//								}
						}
				}
		}

		return 0;
}

