# 程序说明
---
:art:
---
* server
  * saveData()
    * server接收client发来的一条10字节的初始信息
      * 6bytes检查信息`verify`;
      * client即将要发送的真正的信息的字节数(unsigned int);
    * server在确认接收到初始信息后,向client回信`OK`,表明自己已收到长度信息，可以开始接收信息;
    * `savedata()`接收到真正的信息之后,返回存储真正信息的智能指针;
  * saveFIle()
    * ___传输文件一定要使用二进制格式读写,这样会避免一些不必要的格式问题___;
  * saveFixedStruct()
    * 对于每个成员长度都固定的结构体来说,在同种位数的系统上,结构体的长度也是不变的,其存储方式也是固定的;
    * 所以可以使用memcpy函数将一个结构体的所有成员都放在一个数组中传递;
    * 然后再使用memcpy将buffer中的数据存储到相应结构体的实例中,得到与传输前相同的结构体实例;
  * saveMutableStruct()
    * 由于结构体成员长度是可变的,故而不能使用memcpy函数将所有成员都放在一个数组中,但是可以做一些添加用于区分各个成员;
    * 算法    
      * 对于固定长度的成员,直接使用memcpy将其复制进一个buffer中,并记录位置偏移offset;
      * 对于不固定长度的成员,首先计算其具体长度(例子中unsigned short已足够),将其复制进上面buffer后面,然后再将成员按字节复制到后面的buffer中;
      * 这样在解析的时候,根据复制的顺序,依次得到传递前结构体实例成员的值,进而完成传输;

* client
  * sendData()
    * 首先发送检查信息`verify`加要发送的数据的长度;
    * 获取server发来的OK信息;
    * 发送具体数据;
  * sendFile()
    * 使用二进制打开文件,读取文件信息;
    * 使用成员函数`sendData()`发送文件信息;
  * sendFixedStruct()
    * 将结构体实例直接使用memcpy复制进一个数组中;
    * 使用`sendData()`发送结构体数据;
  * sendMutableStruct()
    * 首先计算需要的数组长度,为其开辟空间;
    * 将结构体信息根据server算法进行复制;
    * 使用`sendData()`发送数据;

* 扩展
  * 修改初始信息,如何使用多线程,使得不管client发送什么样的数据,server都能获取正确的信息;
  * 由于刚看到状态机,决定试试,但是最近比较忙,所以下周应该会出一些构思;
  * 构思
    * client端class不变,主函数发送多个不同类型的消息;
    * server主循环函数一直循环接收,发送相应消息到消息队列中;
    * 使用状态机处理三类信息(强行状态机)
      * 文件;
      * 长度不变结构体;
      * 长度可变结构体;
    * 为什么感觉这么low呢！
  * 状态机TCP/IP实现说明
    * 首先说一下状态机的实现
      * 每个状态机都可以有自己的邮箱,所有的信息都从邮箱中读取,当然你也可以没有邮箱,这样就接收不到消息;
      * 如果邮箱中有邮件(信息)的话,对邮件中信息的不同会有相应的处理,这个处理由程序员自己实现;
      * 如果你想发送邮件,那么需要什么东西呢?没错,就是对方邮箱的地址
        * 你可以在初始化自己的时候就获取你想寄邮件的邮箱地址;
        * 你也可以从收到的邮件中获取你想回复的邮箱的地址;
      * 如果你想接收邮件,就把自己的邮箱地址暴露出去;
      * 记得死的时候要关闭自己的邮箱;
    * 然后,确定自己要开几个线程,示例强行开了两个
      * 主线程循环接收client发来的消息;
      * 信息处理线程处理接收到的消息;
    * 为每个线程定做一个类
      * 如果这个类要接收信息,定义一个私有成员receiver,将其地址暴露出去;
      * 如果这个类要主动发送信息,定义相应个数个私有成员sender,在构造的时候为其赋值;
      * 如果这个类要回复信息,可以将邮箱地址写在信息里;
      * 定义一个销毁邮箱的函数,你也可以在析构函数里销毁邮箱;
      * 定义一个主函数,用于处理邮箱信息;
    * 为每种信息定义一个结构体,包含需要的信息,如

      ```C++
      struct file_received
      {
      	std::string str_file;

      	explicit file_received(std::string const &str_file_):
      		str_file(str_file_)
      	{}

      	explicit file_received( const I1 *array, const U4 u4_dataSize)
      	{
      		str_file = std::string(array,u4_dataSize);
      	}
      };
      ```

    * 示例
      * 信息处理邮箱一共处理三类信息
        * 文件;
        * 定长结构体;
        * 变长结构体;
      * 该类的定义如下
        * receiver用于接收邮件;
        * 不需要发送信息,故而没有sender;
        * `run()`函数为主函数,用于处理相应信息
          * saveFile;
          * saveFixedStruct;
          * saveMutableStruct;
        * `get_sender()`函数暴露自己的邮箱地址;
        * `done()`函数销毁自己的邮箱;

        ```C++
        class save_pocket
        {
        	messaging::receiver incoming;
        	std::mutex iom;
        	const std::string FILE_PATH = "receive_file";

        	save_pocket(save_pocket const&)=delete;
        	save_pocket& operator=(save_pocket const&)=delete;

        public:

        	save_pocket() = default;
        	~save_pocket() = default;

        	void run();
        	void saveFile(const I1 *data, const U4 &u4_dataSize);
        	void saveFixedStruct(const I1 *data,const U4 &u4_dataSize);
        	void saveMutableStruct(const I1 *data,const U4 &u4_dataSize);

        	void done()
        	{
        		get_sender().send(messaging::close_queue());
        	}

        	messaging::sender get_sender()
        	{
        		return incoming;
        	}
        };
        ```

      * `run()`函数
        * 在打印消息的时候由于使用了公有资源`std::cout`,所以使用`lock_guard`锁定;

        ```C++
        void save_pocket::run()
        {
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
        }
        ```
