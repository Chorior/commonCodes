# ATM说明
---
:art:
---
* state machine
  * 将每个线程看作一个状态机,当它收到一个消息时,更新自己的状态并发送一个或多个消息给其它线程,其实现依赖于初始状态;
  * 各线程依靠消息队列进行通信;
* ATM
  * 功能
    * 处理人与ATM机间的交互
      * 接收卡片;
      * 显示消息;
      * 处理按键;
      * 放出现金;
      * 退回卡片;
    * ATM机与银行间的交互;
  * 处理方案(分为三个线程)
    * 一个线程处理ATM物理机;
    * 一个线程处理ATM逻辑业务;
    * 一个与银行进行通信;
  * ATM状态机
    * 在每个状态,ATM线程都在等待一个特定的信息,然后处理它,并转换到合适的状态;
    * 大致状态图
    