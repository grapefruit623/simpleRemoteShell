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
		printf ( "The initial step\n" );
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
requestHandler( int socketId, int acceptId, char *incomingMes)
{
		char name[BUFFSIZE], passwd[BUFFSIZE], cmdBuf[BUFFSIZE];
		char cmd[BUFFSIZE], userSay[BUFFSIZE];
		int i = 0, j = 0;
		pid_t pid;
		int fd[2];
//		printf ( "%s from %d\n", incomingMes, acceptId );
		if ( offLine == allUsers[acceptId].stage ) { /* request client to input account */
				write(acceptId, yourName, strlen(yourName));
				allUsers[acceptId].stage = inputAccount;
				return 1;
		}
		if ( inputAccount == allUsers[acceptId].stage ) { /* it get account, now client should send passwd */
//				printf ( "Now the incomingMes is account\n" );
				strcpy(name, incomingMes);
				write(acceptId, yourPasswd, strlen(yourPasswd));
				allUsers[acceptId].stage = inputPasswd; 
				return 1;
		}

		if ( inputPasswd == allUsers[acceptId].stage ) { /* to match account data */
//				printf ( "Now your should input passwd\n" );
				strcpy(passwd, incomingMes);

				for ( i=0 ; i<howManyUsers ; i++ ) {

						if ( !strcmp(legalUsers[i].name, name) && !strcmp(legalUsers[i].passwd, passwd) ) { /* the user is exist on prebuilt file */
//								printf ( "hello %s, %s\n", name, passwd );

								for ( j=0; j<userCanHandle ; j++ ) { /* check repeat log-in */

										if ( !strcmp( allUsers[j].name, name) && ( cmdMode == allUsers[j].stage )) {
												write(acceptId, youHaveBeenOnline, strlen(youHaveBeenOnline));
												return 1;
										}
								}
								write(acceptId, youAreOnline, strlen(youAreOnline));
								allUsers[acceptId].stage = cmdMode;
								strcpy(allUsers[acceptId].name, name);
								strcpy(allUsers[acceptId].passwd, passwd);
								allUsers[acceptId].socket = acceptId;
								allUsers[acceptId].serverCache = i; /* the index of user on the account data  */


//								printf ( "%s %d\n", allUsers[acceptId].name, acceptId );
								return 1;
						}
				}
				allUsers[acceptId].stage = offLine;
				write(acceptId, youAreNotUser, strlen(youAreNotUser));
		}
		if ( cmdMode == allUsers[acceptId].stage ) { /* the cmd mode */

				if ( !strcmp("logout", incomingMes) ) {
						allUsers[acceptId].stage = offLine;
						write(acceptId, yourAreLogout, strlen(yourAreLogout));
						strcpy(allUsers[acceptId].name, "offLine");
						strcpy(allUsers[acceptId].passwd, "offLine");
						return ;
				}
				else {
						if ( strcmp(incomingMes, "\n") ) { /* space do not rum any cmd */
								if ( 0 > pipe(fd) ) {
										fprintf(stderr, "pipe errer");
								}

								if ( 0 > ( pid = fork() ) ) {
										printf ( "fork error\n" );
								}
								else {
										if ( 0 <  pid  ) { /* parent */
												close(fd[1]);
												bzero(cmdBuf, BUFFSIZE);
												while ( 0 < read(fd[0], cmdBuf, BUFFSIZE) ) {
														cmdBuf[strlen(cmdBuf)] = '\0';
														write(acceptId, cmdBuf, strlen(cmdBuf));
												}
												printf ( "I am father %d\n", getpid() );
										}
										else {                  /* son */

												printf ( "Son exec %s\n", incomingMes );
												close(acceptId);
												close(fd[0]);
												if ( 0 >  dup2(fd[1], fileno(stdout)) ) {
														printf ( "error for dup2\n" );
												}
												execl("/bin/bash", "bash", "-c", incomingMes, NULL );
												exit(0);
										}
								}
						}
				}
		}
//		printf ( "who am i %d\n", getpid() );
		return 0;
}

