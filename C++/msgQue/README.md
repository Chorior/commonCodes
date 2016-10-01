# 程序说明
---
:art:
---
* Linux环境提供了XSI和POSIX两套消息队列;
* XSI消息队列有四个方法

  ```C++
  #include <sys/types.h>
  #include <sys/ipc.h>
  #include <sys/msg.h>

  int msgget(key_t key, int msgflg);

  int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

  ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
                 int msgflg);

  int msgctl(int msqid, int cmd, struct msqid_ds *buf);
  ```

* 在开始讲上面四个函数前,先来讲`ftok()`函数

  ```C++
  #include <sys/types.h>
  #include <sys/ipc.h>

  key_t ftok(const char *pathname, int proj_id);
  ```

  * ftok - convert a pathname and a project identifier to a System V IPC key;
  * 转换一个路径名和一个工程标识到一个 System V IPC key;
  * 路径名必须存在且在当前权限下可访问;
  * `proj_id`必须非零,虽然它是一个int变量,但是只有八位被用到
  * 创建出来的`key_t`变量可以被`msgget()`、`semget()`,或 `shmget()`使用;
  * 如果所有的`pathname`都指向同一个文件,且`proj_id`值都相同的话,那么返回的值也是一样的;
  * 如果‘pathname’或`proj_id`其中有一个不同,那么返回的值也应该不同;
  * 错误返回-1;
  * typical usage has an ASCII character `proj_id`, that is why the behavior is said to be undefined when `proj_id` is zero;
  * 不保证返回的`key_t`变量是独一无二的;

* `msgget()`函数
  * `int msgget(key_t key, int msgflg);`
  * `msgget()`返回与key关联的 System V 消息队列标识符;
  * 要想创建一个新的消息队列
    * 令key等于`IPC_PRIVATE`;
    * 或者key不等于`IPC_PRIVATE`,没有与key对应的消息队列存在,且msgflg指定为`IPC_CREAT`;
  * 要想访问一个已存在的消息队列,key不等于`IPC_PRIVATE`,`IPC_CREAT`和`IPC_EXCL`没有同时被msgflg指定;
  * 下面两种情况都可以访问一个已存在的消息队列
    * key不等于`IPC_PRIVATE`,msgflg指定为0;
    * key不等于`IPC_PRIVATE`,msgflg指定为`IPC_CREAT`;
  * 如果msgflg指定了`IPC_CREAT`和`IPC_EXCL`,当与key关联的消息队列已经存在的话,`msgget()`会失败,并置errno为`EEXIST`;  
  * `IPC_PRIVATE`
    * `IPC_PRIVATE`不是一个标志字段,而是一个`key_t`类型值;
    * 当`IPC_PRIVATE`被key使用时,`msgget()`只关心msgflg的最后9位,并且创建一个新的消息队列(如果成功的话);
  * 除了创建消息队列,msgflg剩下的位被用来定义消息队列的权限;
  * 消息队列的权限与文件权限相同,如0600(只有后九位有用)代表只有当前用户可读可写;
  * 错误返回-1;
  * 系统可以创建的队列的数量限制可以从`/proc/sys/kernel/msgmni`查看;

    ```
    If a new message queue is created, then its associated data structure msqid_ds (see msgctl(2)) is initialized as follows:

           msg_perm.cuid and msg_perm.uid are set to the effective user ID of the calling process.

           msg_perm.cgid and msg_perm.gid are set to the effective group ID of the calling process.

           The least significant 9 bits of msg_perm.mode are set to the least significant 9 bits of msgflg.

           msg_qnum, msg_lspid, msg_lrpid, msg_stime, and msg_rtime are set to 0.

           msg_ctime is set to the current time.

           msg_qbytes is set to the system limit MSGMNB
    ```

    ```
    On failure, errno is set to one of the following values:

    EACCES A  message  queue  exists  for  key,  but  the  calling  process does not have permission to access the queue, and does not have the CAP_IPC_OWNER capability.

    EEXIST IPC_CREAT and IPC_EXCL were specified in msgflg, but a message queue already exists for key.

    ENOENT No message queue exists for key and msgflg did not specify IPC_CREAT.

    ENOMEM A message queue has to be created but the system does not have enough memory for the new data structure.

    ENOSPC A message queue has to be created but the system limit for the maximum number of message queues (MSGMNI) would be exceeded.
    ```

* `msgsnd()`函数和`msgrcv()`函数

  ```C++
  int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

  ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
                 int msgflg);
  ```

  * `msgsnd()`和`msgrcv()`分别被用来发送信息到一个消息队列中和从一个消息队列中接收消息;
    * 如果你要向一个消息队列中发送消息,那么你所拥有的进程必须要有对该消息队列写的权限;
    * 如果你要从一个消息队列中接收消息,那么你所拥有的进程必须要有对该消息队列读的权限;
  * msgp参数指向一个自定义结构体
    * 这个结构体的一般形式是

      ```C++
      struct msgbuf {
          long mtype;       /* message type, must be > 0 */
          char mtext[1];    /* message data */
      };
      ```

    * 这个结构体的第一个long成员变量为消息类型,必须大于零,后面在接收时可用于筛选信息;
    * 后面的成员为一个数组或其它结构体

      ```C++
      struct myText
      {
        double f8;    
        int i4;
        float f4;
      };

      struct msgbuf {
          long mtype;         /* message type */
          struct myText text; /* message data */
      };
      ```

    * msgsz参数用于指定发送和接收的消息长度(字节);
    * `msgsnd()`
      * `int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);`
      * 发送一个msgp参数指针指向的消息的复制到与msqid参数标识的消息队列;
      * 成功返回0；
      * 失败返回-1并用errno指明错误类型;
      * 如果消息队列空间足够,那么`msgsnd()`立即成功;
      * 队列的容量限制`msg_qbytes`在队列创建时被置为`MSGMNB`,可以用`msgctl()`函数修改
      * 如果消息队列已满
        * `msgsnd()`默认会发生阻塞直到消息队列可以容纳新的消息;
          * 在阻塞期间,如果发生以下两种情况,函数会失败
            * 队列被移除了,失败并置errno为`EIDRM`;
            * a signal is caught,in which case the system call fails with errno set to `EINTR`;
              * see  signal(7);
              * `msgsnd()` is never automatically restarted after being interrupted by a signal handler, regardless of the setting of the `SA_RESTART` flag when establishing a signal handler;
        * 如果`IPC_NOWAIT`被msgflg参数指定,那么函数失败,并置errno为`EAGAIN`;              
      * 消息队列容量字节限制可以从`/proc/sys/kernel/msgmnb`查看和更改;
      * 单个消息最大字节数可以从`/proc/sys/kernel/msgmax`查看和更改;
      * 消息队列在下面两种情况下被认为是满的
        * 添加一个新的消息到消息队列中会使队列总字节数超过队列的容量限制;
        * 添加一个新的消息队列到消息队列中会使队列消息总数超过队列的承受限制;
      * 即使消息是空的,那它还是会占用内核容量;
      * 消息队列的限制都可以从`msgctl()`函数那里修改;
      * `msgsnd()`完成之后,消息队列数据结构msqid_ds发生以下更新
        * `msg_lspid`被设为调用进程ID;
        * `msg_qnum`递增1;
        * `msg_stime`被设为当前时间;

    * `msgrcv()`
      * `ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);`
      * 从一个消息队列中移除一个消息,并将其放置在msgp参数指向的缓冲区里;
      * 成功返回实际复制到mtext数组的字节数;
      * 失败返回-1并用errno指明错误类型;
      * msgsz参数指定了msgp参数指针指向的结构体的mtext成员的最大字节数;
        * 如果消息长度大于msgsz,那么结果依赖于msgflg参数是否指定了`MSG_NOERROR`;
          * 如果指定,那么消息会被截断,并且截断的部分会被丢弃;
          * 如果未指定,那么消息不会从队列中移除,`msgrcv()`函数会失败返回-1并置errno为`E2BIG`;
      * msgtyp参数指定了请求消息的类型
        * 如果msgtyp为0,那么队列的第一个消息被读取;
        * 如果msgtyp大于0
          * 默认情况下,队列中第一个mtype等于msgtyp的消息被读取;
          * 如果msgtyp指定了`MSG_EXCEPT`,那么队列中第一个mtype不等于msgtyp的消息被读取;
        * 如果msgtyp小于0,那么队列中mtype小于等于msgtyp绝对值中最小的那个消息被读取;
        * If msgtyp is less than 0, then the first message in the queue with the lowest type less than or equal to the  absolute value  of  msgtyp will be read;
      * msgflg是一个用0和以下标志以或逻辑构建而成的标志位
        * `IPC_NOWAIT`: 如果没有需求类型的消息,那么失败立即返回-1并置errno为`ENOMSG`;
        * `MSG_EXCEPT`: 取第一个非msgtyp类型的消息;
        * `MSG_NOERROR`: 如果消息长度过长,那么截断消息;
        * `MSG_COPY` (since Linux 3.8)
          * 以消息位置的方式获取消息;
          * msgtyp的含义变为偏移量,第一个消息的偏移量为0;
          * 必须与`IPC_NOWAIT`结合使用;
          * 如果队列中没有可用的消息,那么函数失败立即返回-1并置errno为`ENOMSG`;
          * 不能与`MSG_EXCEPT`一起使用;
          * 需要内核配置打开`CONFIG_CHECKPOINT_RESTORE`选项,这个选项默认是不打开的;
      * 如果没有可用的消息且msgflg也没有指定`IPC_NOWAIT`,那么调用进程会发生阻塞,直到以下条件发生
        * 队列中出现了可用的消息;
        * 队列被移除了,函数失败立即返回-1并置errno为`EIDRM`;
        * The calling process catches a signal.  In this case, the system call fails with errno set to `EINTR`;
      * 成功时,队列数据结构msqid_ds发生以下更新
        * `msg_lrpid`被设为调用进程ID;
        * `msg_qnum`递增1;
        * `msg_rtime`被设为当前时间;

          ```
          When msgsnd() fails, errno will be set to one among the following values:

          EACCES The calling process does not have write permission on the message queue, and does not have the CAP_IPC_OWNER capability.

          EAGAIN The message can't be sent due to the msg_qbytes limit for the queue and IPC_NOWAIT was specified in msgflg.

          EFAULT The address pointed to by msgp isn't accessible.

          EIDRM  The message queue was removed.

          EINTR  Sleeping on a full message queue condition, the process caught a signal.

          EINVAL Invalid msqid value, or nonpositive mtype value, or invalid msgsz value (less than 0 or greater than the system value MSGMAX).

          ENOMEM The system does not have enough memory to make a copy of the message pointed to by msgp.

          When msgrcv() fails, errno will be set to one among the following values:

          E2BIG  The message text length is greater than msgsz and MSG_NOERROR isn't specified in msgflg.

          EACCES The calling process does not have read permission on the message queue, and does not have the CAP_IPC_OWNER capability.

          EFAULT The address pointed to by msgp isn't accessible.

          EIDRM  While the process was sleeping to receive a message, the message queue was removed.

          EINTR  While the process was sleeping to receive a message, the process caught a signal; see signal(7).

          EINVAL msgqid was invalid, or msgsz was less than 0.

          EINVAL (since Linux 3.14)
                 msgflg specified MSG_COPY, but not IPC_NOWAIT.

          EINVAL (since Linux 3.14)
                 msgflg specified both MSG_COPY and MSG_EXCEPT.

          ENOMSG IPC_NOWAIT was specified in msgflg and no message of the requested type existed on the message queue.

          ENOMSG IPC_NOWAIT and MSG_COPY were specified in msgflg and the queue contains less than msgtyp messages.

          ENOSYS (since Linux 3.8)
                 MSG_COPY was specified in msgflg, and this kernel was configured without CONFIG_CHECKPOINT_RESTORE.
          ```

  * 示例

    ```C++
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/msg.h>

    struct mymsgbuf {
    		long mtype;
    		char mtext[80];
    };

    static void
    usage(char *prog_name,const char *msg)
    {
    		if (msg != NULL)
    				fputs(msg, stderr);

    		fprintf(stderr, "Usage: %s [options]\n", prog_name);
    		fprintf(stderr, "Options are:\n");
    		fprintf(stderr, "-s        send message using msgsnd()\n");
    		fprintf(stderr, "-r        read message using msgrcv()\n");
    		fprintf(stderr, "-t        message type (default is 1)\n");
    		fprintf(stderr, "-k        message queue key (default is 1234)\n");
    		exit(EXIT_FAILURE);
    }

    static void
    send_msg(int qid, int msgtype)
    {
    		struct mymsgbuf msg;
    		time_t t;

    		msg.mtype = msgtype;

    		time(&t);
    		snprintf(msg.mtext, sizeof(msg.mtext), "a message at %s",
    						ctime(&t));

    		if (msgsnd(qid, (void *) &msg, sizeof(msg.mtext),
    								IPC_NOWAIT) == -1) {
    				perror("msgsnd error");
    				exit(EXIT_FAILURE);
    		}
    		printf("sent: %s\n", msg.mtext);
    }

    static void
    get_msg(int qid, int msgtype)
    {
    		struct mymsgbuf msg;

    		if (msgrcv(qid, (void *) &msg, sizeof(msg.mtext), msgtype,
    							 MSG_NOERROR | IPC_NOWAIT) == -1) {
    				if (errno != ENOMSG) {
    						perror("msgrcv");
    						exit(EXIT_FAILURE);
    				}
    				printf("No message available for msgrcv()\n");
    		} else
    				printf("message received: %s\n", msg.mtext);
    }

    int
    main(int argc, char *argv[])
    {
    		int qid, opt;
    		int mode = 0;               /* 1 = send, 2 = receive */
    		int msgtype = 1;
    		int msgkey = 1234;

    		while ((opt = getopt(argc, argv, "srt:k:")) != -1) {
    				switch (opt) {
    				case 's':
    						mode = 1;
    						break;
    				case 'r':
    						mode = 2;
    						break;
    				case 't':
    						msgtype = atoi(optarg);
    						if (msgtype <= 0)
    								usage(argv[0], "-t option must be greater than 0\n");
    						break;
    				case 'k':
    						msgkey = atoi(optarg);
    						break;
    				default:
    						usage(argv[0], "Unrecognized option\n");
    				}
    		}

    		if (mode == 0)
    				usage(argv[0], "must use either -s or -r option\n");

    		qid = msgget(msgkey, IPC_CREAT | 0666);

    		if (qid == -1) {
    				perror("msgget");
    				exit(EXIT_FAILURE);
    		}

    		if (mode == 2)
    				get_msg(qid, msgtype);
    		else
    				send_msg(qid, msgtype);

    		exit(EXIT_SUCCESS);
    }
    ```

  * `msgctl()`函数
    * `int msgctl(int msqid, int cmd, struct msqid_ds *buf);`
    * 对msqid关联的消息队列进行控制;
    * msqid_ds在`<sys/msg.h>`定义如下

      ```C++
      struct msqid_ds {
          struct ipc_perm msg_perm;     /* Ownership and permissions */
          time_t          msg_stime;    /* Time of last msgsnd(2) */
          time_t          msg_rtime;    /* Time of last msgrcv(2) */
          time_t          msg_ctime;    /* Time of last change */
          unsigned long   __msg_cbytes; /* Current number of bytes in
                                           queue (nonstandard) */
          msgqnum_t       msg_qnum;     /* Current number of messages
                                           in queue */
          msglen_t        msg_qbytes;   /* Maximum number of bytes
                                           allowed in queue */
          pid_t           msg_lspid;    /* PID of last msgsnd(2) */
          pid_t           msg_lrpid;    /* PID of last msgrcv(2) */
      };
      ```

    * `ipc_perm`定义如下,其中uid、gid、mode可以使用`IPC_SET`设置

      ```C++
      struct ipc_perm {
          key_t          __key;       /* Key supplied to msgget(2) */
          uid_t          uid;         /* Effective UID of owner */
          gid_t          gid;         /* Effective GID of owner */
          uid_t          cuid;        /* Effective UID of creator */
          gid_t          cgid;        /* Effective GID of creator */
          unsigned short mode;        /* Permissions */
          unsigned short __seq;       /* Sequence number */
      };
      ```

    * cmd可用参数如下
      * `IPC_STAT`: 当调用者拥有对队列读的权限时,将队列数据结构信息复制到参数指针buf指向的缓冲区;
      * `IPC_SET`: 通过修改一些buf指向的`msqid_ds`成员来更新队列数据结构信息,同时更新成员`msg_ctime`;
        * `msg_qbytes`,`msg_perm.uid`,`msg_perm.gid`和`msg_perm.mode`的最后九位;
        * 调用进程的有效ID必须符合`msg_perm.uid`或`msg_perm.cuid`,除非调用者有特权;
        * Appropriate  privilege  (Linux:  the  `CAP_SYS_RESOURCE` capability) is required to raise the `msg_qbytes` value beyond the system parameter MSGMNB;
      * `IPC_RMID`: 立即移除队列,唤醒所有等待读和写的进程(返回-1并置errno为`EIDRM`)
        * 第三个参数被忽略;
        * 调用进程的有效ID必须符合`msg_perm.uid`或`msg_perm.cuid`,除非调用者有特权;
      * `IPC_INFO`: 返回队列限制信息
        * 如果`_GNU_SOURCE`特征宏被定义的话,msginfo数据结构定义在`<sys/msg.h>`;
        * 其中被使用的也就三个: msgmax,msgmnb和msgmni;
        * 这三个的设置分别可以以下地方修改
          * `/proc/sys/kernel/msgmax`;
          * `/proc/sys/kernel/msgmnb`;
          * `/proc/sys/kernel/msgmni`;

            ```C++
            struct msginfo {
                int msgpool; /* Size in kibibytes of buffer pool
                                used to hold message data;
                                unused within kernel */
                int msgmap;  /* Maximum number of entries in message
                                map; unused within kernel */
                int msgmax;  /* Maximum number of bytes that can be
                                written in a single message */
                int msgmnb;  /* Maximum number of bytes that can be
                                written to queue; used to initialize
                                msg_qbytes during queue creation
                                (msgget(2)) */
                int msgmni;  /* Maximum number of message queues */
                int msgssz;  /* Message segment size;
                                unused within kernel */
                int msgtql;  /* Maximum number of messages on all queues
                                in system; unused within kernel */
                unsigned short int msgseg;
                             /* Maximum number of segments;
                                unused within kernel */
            };
            ```

      * `MSG_INFO`: 返回一个与`IPC_INFO`包含相同的信息的msginfo结构体,并用以下字段返回队列系统消耗
        * msgpool字段: 返回系统当前存在的队列总数;
        * msgmap字段: 返回系统所有队列中消息的总数;
        * msgtql字段: 返回系统所有队列所有消息的总字节数;
      * `MSG_STAT`: 返回一个与`IPC_STAT`包含相同的信息的`msqid_ds`结构体,然而
        * the msqid argument is not a queue identifier;
        * but instead an index into the kernel's internal array that maintains information about all message queues on the system;
    * 返回值
      * 成功时
        * `IPC_STAT`,`IPC_SET`和`IPC_RMID` 返回0;
        * `MSG_STAT`返回队列的标识符,这个队列的索引在msqid中给出;
        * `IPC_INFO`和`MSG_INFO`
          * returns the index of the highest used entry in the kernel's internal array recording information about all message  queues;
          * This information can be used with repeated `MSG_STAT` operations to obtain information about all queues on the system;
      * 错误返回-1

        ```
        On failure, errno is set to one of the following:

        EACCES The argument cmd is equal to IPC_STAT or MSG_STAT, but the call‐
               ing  process  does not have read permission on the message queue
               msqid, and does not have the CAP_IPC_OWNER capability.

        EFAULT The argument cmd has the value  IPC_SET  or  IPC_STAT,  but  the
               address pointed to by buf isn't accessible.

        EIDRM  The message queue was removed.

        EINVAL Invalid  value  for cmd or msqid.  Or: for a MSG_STAT operation,
               the index value specified in msqid referred  to  an  array  slot
               that is currently unused.

        EPERM  The  argument  cmd  has  the  value IPC_SET or IPC_RMID, but the
               effective user ID of the calling process is not the creator  (as
               found  in msg_perm.cuid) or the owner (as found in msg_perm.uid)
               of the message queue, and the caller is not  privileged  (Linux:
               does not have the CAP_SYS_ADMIN capability).

        EPERM  An  attempt (IPC_SET) was made to increase msg_qbytes beyond the
               system parameter  MSGMNB,  but  the  caller  is  not  privileged
               (Linux: does not have the CAP_SYS_RESOURCE capability).
        ```
