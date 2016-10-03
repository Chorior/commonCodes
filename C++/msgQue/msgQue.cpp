#include <iostream>
#include <stdio.h> // snprintf
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mutex>
#include <thread>
#include <condition_variable>

std::mutex mut_rcv;
std::mutex mut_cout;
std::condition_variable msg_cond;
int qid = -1;

struct mymsgbuf {
	long mtype;
	char mtext[256];
};

static void
send_msg(int qid, int msgtype, const char *text)
{
	struct mymsgbuf msg;
	time_t t;

	msg.mtype = msgtype;

	time(&t);
	snprintf(msg.mtext, sizeof(msg.mtext), "%d:%d: %s",
			localtime(&t)->tm_hour,localtime(&t)->tm_min,text);

	if (msgsnd(qid, (void *) &msg, sizeof(msg.mtext),
			IPC_NOWAIT) == -1) {
		perror("msgsnd error");
		exit(EXIT_FAILURE);
	}
}

static void
get_msg()
{
	while(1)
	{
		struct mymsgbuf msg;
		std::unique_lock<std::mutex> lk(mut_rcv);
		msg_cond.wait(
			lk,
			[]
			{
				struct msqid_ds buf;
				if(-1 != msgctl(qid, IPC_STAT, &buf)){
					return 0 != buf.__msg_cbytes;
				}
				std::lock_guard<std::mutex> lk(mut_cout);
				std::cout << "msgctl() error\n";
				return false;
			}
			);

		if (msgrcv(qid, (void *) &msg, sizeof(msg.mtext), 0,
  				MSG_NOERROR | IPC_NOWAIT) == -1) {
  			if (errno != ENOMSG) {
  				perror("msgrcv");
  			exit(EXIT_FAILURE);
	  		}
			std::lock_guard<std::mutex> lk(mut_cout);
  			std::cout << "No message available for msgrcv()\n";
		} else{
			std::lock_guard<std::mutex> lk(mut_cout);
			std::cout << "message received: \n"
				  << "msgtype = " << msg.mtype << std::endl
				  << "msgtext = " << msg.mtext << std::endl;
		}

	}
}

void input_msg(char *text, const int &textSize, int &msgtype)
{
	std::lock_guard<std::mutex> lk(mut_cout);
	while(0 >= msgtype)
	{
		std::cout << "input your message type(must > 0): ";
		std::cin >> msgtype;

		while('\n' != std::cin.get()){}
	}
	std::cout << "input your message: ";
	std::cin.getline(text,textSize);
}

int
main()
{
	int msgtype = 1;
	int msgkey = 1234;

	qid = msgget(msgkey, IPC_CREAT | 0666);
	if (qid == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	std::thread t(get_msg);

	while(1)
	{
		char msgtext[256] = { 0 };
		int msgtype = 0;

		input_msg(msgtext, sizeof(msgtext), msgtype);
		send_msg(qid,msgtype,msgtext);

		msg_cond.notify_one();
		usleep(10 * 1000);
	}

	t.join();

	exit(EXIT_SUCCESS);
}
