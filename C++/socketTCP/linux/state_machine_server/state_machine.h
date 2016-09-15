#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include "types.h"

namespace messaging
{
	// Base class of our queue entries
	struct message_base
	{
		virtual ~message_base()
		{}
	};

	// Each message type has a specialization
	template<typename Msg>
	struct wrapped_message:
		message_base
	{
		Msg contents;
		explicit wrapped_message(Msg const& contents_):
			contents(contents_)
		{}
	};

	// Our message queue
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

	// The message for closing the queue
	class close_queue
	{};

	template<typename PreviousDispatcher,typename Msg,typename Func>
	class TemplateDispatcher
	{
		queue* q;
		PreviousDispatcher* prev;
		Func f;
		bool chained;

		TemplateDispatcher(TemplateDispatcher const&)=delete;
		TemplateDispatcher& operator=(TemplateDispatcher const&)=delete;

		// TemplateDispatcher instantiations are friends of each other
		template<typename Dispatcher,typename OtherMsg,typename OtherFunc>
		friend class TemplateDispatcher;

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

	public:

		TemplateDispatcher(TemplateDispatcher&& other):
			q(other.q),
			prev(other.prev),
			f(std::move(other.f)),
			chained(other.chained)
		{
			other.chained=true;
		}

		TemplateDispatcher(queue* q_,PreviousDispatcher* prev_,Func&& f_):
			q(q_),
			prev(prev_),
			f(std::forward<Func>(f_)),
			chained(false)
		{
			prev_->chained=true;
		}

		// Additional handlers can be chained
		template<typename OtherMsg,typename OtherFunc>
		TemplateDispatcher<TemplateDispatcher,OtherMsg,OtherFunc>
			handle(OtherFunc&& of)
		{
			return TemplateDispatcher<
				TemplateDispatcher,OtherMsg,OtherFunc>(
					q,this,std::forward<OtherFunc>(of));
		}

		// The destructor is noexcept(false) again
		~TemplateDispatcher() noexcept(false)
		{
			if(!chained)
			{
				wait_and_dispatch();
			}
		}
	};

	class dispatcher
	{
		queue* q;
		bool chained;

		// dispatcher instances cannot be copied
		dispatcher(dispatcher const&)=delete;
		dispatcher& operator=(dispatcher const&)=delete;

		// Allow TemplateDispatcher instances to access the internals
		template<
			typename Dispatcher,
			typename Msg,
			typename Func>
		friend class TemplateDispatcher;

		void wait_and_dispatch()
		{
			// Loop, waiting for and dispatching messages
			for(;;)
			{
				auto msg=q->wait_and_pop();
				dispatch(msg);
			}
		}

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

	public:

		// dispatcher instances can be moved
		dispatcher(dispatcher&& other):
			q(other.q),
			chained(other.chained)
		{
			// The source mustn’t wait for messages
			other.chained=true;
		}

		explicit dispatcher(queue* q_):
			q(q_),
			chained(false)
		{}

		// Handle a specific type of message with a TemplateDispatcher
		template<typename Message,typename Func>
		TemplateDispatcher<dispatcher,Message,Func>
			handle(Func&& f)
		{
			return TemplateDispatcher<dispatcher,Message,Func>(
				q,this,std::forward<Func>(f));
		}

		// The destructor might throw exceptions
		~dispatcher() noexcept(false)
		{
			if(!chained)
			{
				wait_and_dispatch();
			}
		}
	};

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
}

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

struct fixed_struct_received
{
	std::string str_fixed_struct;

	explicit fixed_struct_received(std::string const &str_fixed_struct_):
		str_fixed_struct(str_fixed_struct_)
	{}

	explicit fixed_struct_received( const I1 *array, const U4 u4_dataSize)
	{
		str_fixed_struct = std::string(array,u4_dataSize);
	}
};

struct mutable_struct_received
{
	std::string str_mutable_struct;

	explicit mutable_struct_received(std::string const &str_mutable_struct_):
		str_mutable_struct(str_mutable_struct_)
	{}

	explicit mutable_struct_received( const I1 *array, const U4 u4_dataSize)
	{
		str_mutable_struct = std::string(array,u4_dataSize);
	}
};

#endif // _STATE_MACHINE_H_
