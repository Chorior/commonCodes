#include <iostream>
#include "common.h"

int main()
{
	key_t key;
	int qid;
	my_msgBuf msg_rcv;
	my_msgBuf msg_send
	{
		1,
		"server received!"
	};

	if(-1 == (key = ftok(MSG_FILE,'a')))
	{
		perror("ftok() failed!");
		exit(1);
	}

	if(-1 == (qid = msgget(key,IPC_CREAT | PERMISSIONS)))
	{
		perror("msgget() failed!");
		if(-1 == msgctl(qid,IPC_RMID,nullptr))
		{
			perror("msgget() msgctl() failed!");
		}
		exit(1);
	}

	while(1)
	{
		if(-1 == 
			msgrcv(qid,&msg_rcv,
				sizeof(my_msgBuf) - sizeof(long),0,0))
		{
			perror("msgrcv() failed!");
			if(-1 == msgctl(qid,IPC_RMID,nullptr))
			{
				perror("msgrcv() msgctl() failed!");
			}
			exit(1);
		}
		else
		{
			std::cout << "Server received: "
				  << msg_rcv.mtext
				  << std::endl;			
			memset(&msg_rcv,'\0',sizeof(my_msgBuf));

			if(-1 == 
				msgsnd(qid,&msg_send,
					sizeof(my_msgBuf) - sizeof(long),0))
			{
				perror("msgsnd() failed!");
				if(-1 == msgctl(qid,IPC_RMID,nullptr))
				{
					perror("msgsnd() msgctl() failed!");
				}
				exit(1);
			}
		}
	}

	return 	EXIT_SUCCESS;
}	

