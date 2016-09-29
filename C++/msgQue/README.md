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
  * 要想访问一个已存在的消息队列
    * msgflg指定为0;
  * 如果msgflg指定了`IPC_CREAT`和`IPC_EXCL`,当与key关联的消息队列已经存在的话,`msgget()`会失败,并置errno为`EEXIST`;  
  * `IPC_PRIVATE`
    * `IPC_PRIVATE`不是一个标志字段,而是一个`key_t`类型值;
    * 当`IPC_PRIVATE`被key使用时,`msgget()`只关心msgflg的最后9位,并且创建一个新的消息队列(如果成功的话);
  * 除了创建消息队列,msgflg剩下的位被用来定义消息队列的权限;
  * 消息队列的权限与文件权限相同,如600代表只有当前用户可读可写;
  * 错误返回-1;

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
      }

      struct msgbuf {
          long mtype;         /* message type */
          struct myText text; /* message data */
      };
      ```

    * msgsz参数用于指定发送和接收的消息长度(字节);
    * `msgsnd()`
      * 发送一个msgp参数指针指向的消息的复制到与msqid参数标识的消息队列;
      * 成功返回0；
      * 失败返回-1并用errorno指明错误类型;
      * 如果消息队列空间足够,那么`msgsnd()`立即成功;
      * 如果消息队列已满
        * `msgsnd()`默认会发生阻塞直到消息队列可以容纳新的消息;
          * 在阻塞期间,如果发生以下两种情况,函数会失败
            * 队列被移除了,失败并置errorno为`EIDRM`;
            * a signal is caught,in which case the system call fails with errno set to `EINTR`;
              * see  signal(7);
              * `msgsnd()` is never automatically restarted after being interrupted by a signal handler, regardless of the setting of the `SA_RESTART` flag when establishing a signal handler;
        * 如果`IPC_NOWAIT`被msgflg参数指定,那么函数失败,并置errorno为`EAGAIN`;              
      * 消息队列容量限制可以从`/proc/sys/kernel/msgmnb`查看,这个限制可以用`msgctl()`函数修改;
      * 消息队列在下面两种情况下被认为是满的
        * 添加一个新的消息到消息队列中会使队列总字节数超过队列的容量限制;
        * 添加一个新的消息队列到消息队列中会使队列消息总数超过队列的承受限制;
      * 即使消息是空的,那它还是会占用内核容量;
      * 消息队列的限制都可以从`msgctl()`函数那里修改;
      * `msgsnd()`完成之后,消息队列数据结构发生以下更新
        * `msg_lspid`被设为调用进程ID;
        * `msg_qnum`递增1;
        * `msg_stime`被设为当前时间;

    * `msgrcv()`
      * 从一个消息队列中移除一个消息,并将其放置在msgp参数指向的缓冲区里;
      * 成功返回实际复制到mtext数组的字节数;
      * 失败返回-1并用errorno指明错误类型;
