#include <iostream>
#include "common.h"

int main()
{
	key_t key;
	int qid;
	my_msgBuf msg_send,msg_rcv;

	if(-1 == (key = ftok(MSG_FILE,'a')))
	{
		perror("ftok() failed!");
		exit(1);
	}

	if(-1 == (qid = msgget(key,PERMISSIONS)))
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
		std::cout << "input a message: ";
		msg_send.mtype = 1;
		if(nullptr == fgets(msg_send.mtext,BUFFER_SIZE,stdin))
		{
			perror("no message!");
			continue;
		}
		
		if(-1 == msgsnd(qid,&msg_send,
			sizeof(my_msgBuf) - sizeof(long),0))
		{
			perror("msgsnd() failed!");
			if(-1 == msgctl(qid,IPC_RMID,nullptr))
			{
				perror("msgsnd() msgctl() failed!");
			}
			exit(1);
		}
		else
		{
			if(-1 == msgrcv(qid,&msg_rcv,
				sizeof(my_msgBuf) - sizeof(long),0,0))
			{
				perror("msgrcv() failed!");
				if(-1 == msgctl(qid,IPC_RMID,nullptr))
				{
					perror("msgrcv() msgctl() failed!");					}
				exit(1);
			}
			
			std::cout << "client received: "
				  << msg_rcv.mtext
				  << std::endl;
			
			memset(&msg_send,'\0',sizeof(my_msgBuf));
			memset(&msg_rcv,'\0',sizeof(my_msgBuf));
		}
	}

	return EXIT_SUCCESS;
}
