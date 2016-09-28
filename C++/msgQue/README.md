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
