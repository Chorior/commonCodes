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
  * ATM业务逻辑状态机
    * 在每个状态,ATM线程都在等待一个特定的信息,然后处理它,并转换到合适的状态;
    * 大致状态图

      ![ATM state](https://github.com/Chorior/commonCodes/blob/master/C%2B%2B/state%20machine/ATM.png)

    * 说明
      * 状态0(等待插入卡片)
        * 初始状态;
        * 若卡片插入,进入状态1;
      * 状态1(等待输入密码)
        * 若按下cancel,进入状态6;
        * 一次输入一个数字,一旦密码位数足够,进入状态2;
        * 可以删除输入的最后一个数字;
      * 状态2(确认密码是否正确)
        * 若按下cancel,进入状态6;
        * 若密码错误,进入状态6;
        * 若密码正确,进入状态3;
      * 状态3(等待用户操作)
        * 若按下cancel,进入状态6;
        * 若按下余额按钮,向银行发送获取余额信息,进入状态4;
        * 若按下取款按钮,向银行发送取款信息,进入状态5;
      * 状态4(显示余额)
        * 若按下cancel,进入状态6;
        * 向ATM物理机发送显示余额信息,进入状态3;
      * 状态5(放款处理)
        * 若按下cancel,进入状态6;
        * 若余额充足,放出现金,进入状态6;
        * 若余额不足,向ATM物理机发送显示余额不足,进入状态6;
      * 状态6(退回卡片)
        * 向ATM物理机发送退卡信息,返回初始状态;

    * 主函数(state是一个函数指针)

      ```C++
	void run()
	{
		state=&atm::waiting_for_card;
		try
		{
			for(;;)
			{
				(this->*state)();
			}
		}
		catch(messaging::close_queue const&)
		{          
		}
	}
      ```

  * 银行状态机
    * 只有一个状态;
    * 处理五类消息
      * 确认密码;
      * 取款信息处理;
      * 余额信息处理
      * 放款成功信息处理;
      * 放款取消处理;
    * 主函数

      ```C++
	void run()
	{
		try
		{
			for(;;)
			{
				incoming.wait()
					.handle<verify_pin>(
						[&](verify_pin const& msg)
						{
							if(msg.pin=="1937")
							{
								msg.atm_queue.send(pin_verified());
							}
							else
							{
								msg.atm_queue.send(pin_incorrect());
							}
						}
						)
					.handle<withdraw>(
						[&](withdraw const& msg)
						{
							if(balance>=msg.amount)
							{
								msg.atm_queue.send(withdraw_ok());
								balance-=msg.amount;
							}
							else
							{
								msg.atm_queue.send(withdraw_denied());
							}
						}
						)
					.handle<get_balance>(
						[&](get_balance const& msg)
						{
							msg.atm_queue.send(::balance(balance));
						}
						)
					.handle<withdrawal_processed>(
						[&](withdrawal_processed const& msg)
						{                  
						}
						)
					.handle<cancel_withdrawal>(
						[&](cancel_withdrawal const& msg)
						{
						}
						);
			}
		}
		catch(messaging::close_queue const&)
		{
		}
	}
      ```

  * 用户界面状态机
    * 只有一个状态;
    * 处理十类信息;
    * 主函数

      ```C++
	void run()
	{
		try
		{
			for(;;)
			{
				incoming.wait()
					.handle<issue_money>(
						[&](issue_money const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"Issuing "
									 <<msg.amount<<std::endl;
							}
						}
						)
					.handle<display_insufficient_funds>(
						[&](display_insufficient_funds const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"Insufficient funds"<<std::endl;
							}
						}
						)
					.handle<display_enter_pin>(
						[&](display_enter_pin const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout
									<<"Please enter your PIN (0-9)"
									<<std::endl;
							}
						}
						)
					.handle<display_enter_card>(
						[&](display_enter_card const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"Please enter your card (I)"
									 <<std::endl;
							}
						}
						)
					.handle<display_balance>(
						[&](display_balance const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout
									<<"The balance of your account is "
									<<msg.amount<<std::endl;
							}
						}
						)
					.handle<display_withdrawal_options>(
						[&](display_withdrawal_options const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"Withdraw 50? (w)"<<std::endl;
								std::cout<<"Display Balance? (b)"
									 <<std::endl;
								std::cout<<"Cancel? (c)"<<std::endl;
							}
						}
						)
					.handle<display_withdrawal_cancelled>(
						[&](display_withdrawal_cancelled const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"Withdrawal cancelled"
									 <<std::endl;
							}
						}
						)
					.handle<display_pin_incorrect_message>(
						[&](display_pin_incorrect_message const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"PIN incorrect"<<std::endl;
							}
						}
						)
					.handle<eject_card>(
						[&](eject_card const& msg)
						{
							{
								std::lock_guard<std::mutex> lk(iom);
								std::cout<<"Ejecting card"<<std::endl;
							}
						}
						);
			}
		}
		catch(messaging::close_queue&)
		{
		}
	}
      ```

* main函数

  ```C++
	while(!quit_pressed)
	{
		char c=getchar();
		switch(c)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			atmqueue.send(digit_pressed(c));
			break;
		case 'b':
			atmqueue.send(balance_pressed());
			break;
		case 'w':
			atmqueue.send(withdraw_pressed(50));
			break;
		case 'c':
			atmqueue.send(cancel_pressed());
			break;
		case 'q':
			quit_pressed=true;
			break;
		case 'i':
			atmqueue.send(card_inserted("acc1234"));
			break;
		}
	}
  ```

* 示例演示

  ```
  $: ./ATM
  Please enter your card (I)
  i
  Please enter your PIN (0-9)
  1973
  PIN incorrect
  Ejecting card
  Please enter your card (I)
  i
  Please enter your PIN (0-9)
  1937
  Withdraw 50? (w)
  Display Balance? (b)
  Cancel? (c)
  b
  The balance of your account is 199
  Withdraw 50? (w)
  Display Balance? (b)
  Cancel? (c)
  w
  Issuing 50
  Ejecting card
  Please enter your card (I)
  ^C
  ```
