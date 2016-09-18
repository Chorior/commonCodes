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
      * 还是挺简单的,比较难的是信息处理机制dispatcher的实现;

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

  * 最难的是dispatcher,邮件的处理,我可能要理解一下
    * receiver返回的是一个临时dispatcher实例,它会调用析构函数进行销毁
    * 析构函数在实例可用的情况下调用`wait_and_dispatch()`函数

      ```C++
      ~dispatcher() noexcept(false)
  		{
  			if(!chained)
  			{
  				wait_and_dispatch();
  			}
  		}
      ```

    * `wait_and_dispatch()`函数等待邮箱信息,并将其传送给`dispatch()`函数

      ```C++
      void wait_and_dispatch()
  		{
  			// Loop, waiting for and dispatching messages
  			for(;;)
  			{
  				auto msg=q->wait_and_pop();
  				dispatch(msg);
  			}
  		}
      ```

    * `dispatch()`函数检查信息是否是销毁邮箱信息
      * 如果是,那么抛出一个异常;
        * 这是析构函数以`noexcept(false)`标识的原因;
        * 因为默认是`noexcept(true)`,抛出异常会使程序终止;
      * 若不是,那么返回一个false,表明该信息没有被处理;

        ```C++
        // dispatch() checks for a close_queue message, and throws
    		bool dispatch(
    			std::shared_ptr<message_base> const& msg)
    		{
    			if(dynamic_cast<wrapped_message<close_queue>*>(msg.get()))
    			{
    				throw close_queue();
    			}

    			return false;
    		}
        ```

    * `handle()`函数是一个用来处理邮件信息的函数
      * 由于信息类型不是可推论的,所以使用了模板参数指明类型,并传递一个函数或可调用对象处理这个信息;
      * 这个函数传递了`std::queue`指针、当前邮件处理者和一个处理函数给一个模板类TemplateDispatcher实例用于处理指定类型的邮件信息;

        ```C++
        // Handle a specific type of message with a TemplateDispatcher
    		template<typename Message,typename Func>
    		TemplateDispatcher<dispatcher,Message,Func>
    			handle(Func&& f)
    		{
    			return TemplateDispatcher<dispatcher,Message,Func>(
    				q,this,std::forward<Func>(f));
    		}
        ```

    * 和dispatcher类差不多,模板类TemplateDispatcher的析构函数也调用`wait_and_dispatch()`函数
      * 其等待邮箱信息,并调用`dispatch()`函数进行处理;

        ```C++
        void wait_and_dispatch()
    		{
    			for(;;)
    			{
    				auto msg=q->wait_and_pop();
    				// If we handle the message, break out of the loop
    				if(dispatch(msg))
    					break;
    			}
    		}
        ```

    * `dispatch()`函数处理得到的消息,
      * 若当前处理者能够处理这个信息,那么处理完毕之后返回true结束当前处理任务;
      * 若不是当前处理者能够处理的信息,那么交还给前一个处理者处理
        * 若前一个处理者也不能处理这个信息,那么返回false表示信息未处理但是已经丢弃了,继续等待下一个邮件信息;
        * 若是销毁邮箱信息,那么抛出异常;

          ```C++
          bool dispatch(std::shared_ptr<message_base> const& msg)
      		{
      			// Check the message type, and call the function
      			if(wrapped_message<Msg>* wrapper=
      				dynamic_cast<wrapped_message<Msg>*>(msg.get()))
      			{
      				f(wrapper->contents);
      				return true;
      			}
      			else
      			{
      				// Chain to the previous dispatcher
      				return prev->dispatch(msg);
      			}
      		}
          ```

    * 使用者只要让自己的邮箱不停的等待邮箱信息,自动处理直到捕获到`close_queue`异常即可
      * 就像示例处理的一样,还是挺简单的(妈的真会玩,我以前的类都是结束调用析构,特么直接从析构开始,666)

        ```C++
        try
      	{
      		for(;;)
      		{
      			incoming.wait()
      				.handle<file_received>(
      					[&](file_received const& msg)
      					{
      						std::lock_guard<std::mutex> lk(iom);
      						std::cout << "received a file\n";
      						saveFile(msg.str_file.c_str(),msg.str_file.size());
      					}
      					)
      				.handle<fixed_struct_received>(
      					[&](fixed_struct_received const& msg)
      					{
      						std::lock_guard<std::mutex> lk(iom);
      						std::cout << "received a fixed_struct\n";
      						saveFixedStruct(msg.str_fixed_struct.c_str(),msg.str_fixed_struct.size());
      					}
      					)
      				.handle<mutable_struct_received>(
      					[&](mutable_struct_received const& msg)
      					{
      						std::lock_guard<std::mutex> lk(iom);
      						std::cout << "received a mutable_struct\n";
      						saveMutableStruct(msg.str_mutable_struct.c_str(),msg.str_mutable_struct.size());
      					}
      					);
      		}
      	}
      	catch(messaging::close_queue const&)
      	{
      	}
        ```
