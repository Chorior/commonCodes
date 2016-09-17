# state machine 命名空间 messaging 说明
---
:art:
---
* 先说一下这个命名空间的用法
  * messaging中最重要的就是sender和receiver类,它们分别就像是邮箱地址和邮箱一样,比如说
    * 向一个消息队列中发送消息,只要使用`sender.send(Message const& msg)`函数即可(Message为模板参数);
    * 要处理一个消息队列中不同类型的消息,只要使用`receiver.wait().handle(Func&& f)`即可(Func为模板参数);
  * 若一个状态机想要主动发送信息,就需要一个sender成员;
  * 若一个状态机想要接收信息,就需要一个receiver成员,并且需要一个公有的暴露自己邮箱地址的成员函数;
  * 若一个状态机需要根据收到的信息进行回复,可以为信息添加一个sender成员;
  * 为自己的消息类型设计一个结构体;
  * 示例在<https://github.com/Chorior/commonCodes/tree/master/C%2B%2B/socketTCP/linux>

* 下面来看messaging的具体实现
  * 首先是一个消息队列的实现
    * 先定义了一个信息基类结构体message_base,便于后续队列信息的转换

      ```C++
      struct message_base
    	{
    		virtual ~message_base()
    		{}
    	};
      ```

    * 然后定义了自定义封装信息的模板结构

      ```C++
      template<typename Msg>
    	struct wrapped_message:
    		message_base
    	{
    		Msg contents;
    		explicit wrapped_message(Msg const& contents_):
    			contents(contents_)
    		{}
    	};
      ```

    * 最后是信息队列类的实现
      * 定义了一个基类结构体指针的`std::queue`成员;
      * `push()`模板函数为模板参数类型的数据创建一个`std::shared_ptr<wrapped_message<T> >`的临时指针,并将其压入`std::queue`;
      * `wait_and_pop()`函数等待`std::queue`不为空的时候,取走其第一个成员,并将其抹杀;
      * 还是挺简单的,比较难的是信息收发dispatcher的实现;

      ```C++
      class queue
    	{
    		std::mutex m;
    		std::condition_variable c;
    		// Actual queue stores pointers to message_base
    		std::queue<std::shared_ptr<message_base> > q;

    	public:

    		template<typename T>
    		void push(T const& msg)
    		{
    			std::lock_guard<std::mutex> lk(m);
    			// Wrap posted message and store pointer
    			q.push(std::make_shared<wrapped_message<T> >(msg));
    			c.notify_all();
    		}

    		std::shared_ptr<message_base> wait_and_pop()
    		{
    			std::unique_lock<std::mutex> lk(m);
    			// Block until queue isn’t empty
    			c.wait(lk,[&]{return !q.empty();});
    			auto res=q.front();
    			q.pop();
    			return res;
    		}
    	};
      ```

  * 然后是sender和receiver的实现
    * sender

      ```C++
      class sender
    	{
    		// sender is wrapper around queue pointer
    		queue*q;

    	public:

    		// Default-constructed sender has no queue
    		sender():
    			q(nullptr)
    		{}

    		// Allow construction from pointer to queue
    		explicit sender(queue*q_):
    			q(q_)
    		{}

    		template<typename Message>
    		void send(Message const& msg)
    		{
    			if(q)
    			{
    				// Sending pushes message on the queue
    				q->push(msg);
    			}
    		}
    	};
      ```

    * receiver
      * 直接用operator的意思是
        * implicit conversion;
        * 就是默认转换,返回receiver是不可能的,只能获取其地址;
        * 其实想想也对,爸爸的邮箱只能爸爸自己用

      ```C++
      class receiver
    	{
    		// A receiver owns the queue
    		queue q;

    	public:

    		// Allow implicit conversion to a sender that references the queue
    		operator sender()
    		{
    			return sender(&q);
    		}

    		// Waiting for a queue creates a dispatcher
    		dispatcher wait()
    		{
    			return dispatcher(&q);
    		}
    	};
      ```

  * 最难的是dispatcher,邮件的收发,我可能要理解一下
