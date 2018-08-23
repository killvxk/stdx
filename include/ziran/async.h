#pragma once
#include <future>
#include <functional>
namespace ziran
{
	namespace tools
	{
		namespace async
		{
			//任务状态
			enum task_status
			{
				//准备好了
				ready = 0,
				//正在运行
				running,
				//完成
				done,
				//出错
				error,
				//等待子任务
				//wait_for_other
			};
			//线程选项
			enum thread_option
			{
				//创建线程
				create = 0,
				//不创建线程
				no_create,
				//池线程
				//pool
			};
			//定义void
#define task_void int
			//定义void参数
#define task_void_arg int
			//定义启动void的参数
#define task_void_arg_start 0
			//任务模板
			template<typename TResult, typename TArgs>
			class task
			{
			public:
				typedef std::function<void(TResult)> call_back;
				//默认构造函数
				task() = default;
				//构造函数
				explicit task(std::function<TResult(TArgs)> func, const thread_option &option);
				//拷贝构造函数
				task(const task<TResult, TArgs> &other);
				//move构造函数
				task(task<TResult, TArgs> &&other);
				//拷贝赋值函数
				task<TResult, TArgs> &operator=(const task<TResult, TArgs> &other);
				//析构函数
				~task() = default;
				//启动任务
				void start(TArgs args);
				//设置回调
				inline void set_call_back(call_back func)
				{
					call_back_event = func;
				}
				//延续任务
				template<typename TOtherResult>
				std::shared_ptr<task<TOtherResult, TResult>> then(std::function<TOtherResult(TResult)> func);
				//延续任务(返回void)
				std::shared_ptr<task<task_void, TResult>> then(std::function<void(TResult)> func);
				//延续任务(参数void)
				template<typename TOtherResult>
				std::shared_ptr<task<TOtherResult, TResult>> then(std::function<TOtherResult()> func);
				//延续任务(接受void,返回void)
				std::shared_ptr<task<task_void, TResult>> then(std::function<void()> func);
				//获取shared_future
				inline std::shared_future<TResult> &get_future()
				{
					return *future_ptr;
				}
				//获取状态
				inline task_status &get_status()
				{
					return status;
				}
				//执行函数
				void exectue_func(const TArgs arg)
				{
					try
					{
						//执行函数
						auto r = run_func(arg);
						//设置结果
						promise_ptr->set_value(r);
						//执行回调
						raise_call_back();
						return;
					}
					////如果抛出异常
					catch (const std::exception&)
					{
						//设置异常
						promise_ptr->set_exception(std::current_exception());
						//设置状态为出错
						set_status(task_status::error);
					}
				}
			private:
				//线程选项
				thread_option option;
				//状态
				task_status status;
				//要执行的函数
				std::function<TResult(TArgs)> run_func;
				//回调函数
				call_back call_back_event;
				//promise指针
				std::shared_ptr<std::promise<TResult>> promise_ptr;
				//shared_future指针
				std::shared_ptr<std::shared_future<TResult>> future_ptr;
				//引起回调
				void raise_call_back()
				{
					//获取结果
					TResult r = future_ptr->get();
					//回调
					call_back_event(r);
					//设置状态
					set_status(task_status::done);
				}
				//设置状态
				inline void set_status(const task_status &status)
				{
					this->status = status;
				}
			};
			//构造函数
			template<typename TResult, typename TArgs>
			task<TResult, TArgs>::task(std::function<TResult(TArgs)> func, const thread_option &option)
				:option(option)
				, status(task_status::ready)
				, run_func(func)
				, call_back_event([](TResult r) {})
			{
			}
			//拷贝构造函数
			template<typename TResult, typename TArgs>
			task<TResult, TArgs>::task(const task<TResult, TArgs> &other)
				:option(other.option)
				, status(other.status)
				, run_func(other.run_func)
				, call_back_event(other.call_back_event)
				, promise_ptr(other.promise_ptr)
				, future_ptr(other.future_ptr)
			{
			}

			//move构造函数
			template<typename TResult, typename TArgs>
			task<TResult, TArgs>::task(task<TResult, TArgs> &&other)
				:option(std::move(other.option))
				, status(std::move(other.status))
				, run_func(std::move(other.run_func))
				, call_back_event(std::move(other.call_back_event))
				, promise_ptr(std::move(other.promise_ptr))
				, future_ptr(std::move(other.future_ptr))
			{
			}
			//拷贝赋值函数
			template<typename TResult, typename TArgs>
			task<TResult, TArgs> & task<TResult, TArgs>::operator=(const task<TResult, TArgs> &other)
			{
				option = other.option;
				status = other.status;
				run_func = other.run_func;
				call_back_event = other.call_back_event;
				promise_ptr = other.promise_ptr;
				future_ptr = other.future_ptr;
				return *this;
			}
			//启动任务
			template<typename TResult, typename TArgs>
			void task<TResult, TArgs>::start(TArgs args)
			{
				//将状态设置为运行中
				status = task_status::running;
				//初始化promise
				this->promise_ptr = std::make_shared<std::promise<TResult>>();
				//初始化future
				this->future_ptr = std::make_shared<std::shared_future<TResult>>(promise_ptr->get_future());
				//如果需要创建线程
				if (option == thread_option::create)
				{
					//新建线程
					std::thread thread([this](TArgs args)
					{
						exectue_func(args);
					}, args);
					//分离线程
					thread.detach();
					return;
				}
				//不需要创建线程
				else if (option == thread_option::no_create)
				{
					//直接执行
					exectue_func(args);
				}
			}
#pragma region task_helper
			//创建返回void的方法
			template<typename TArgs>
			inline std::function<task_void(TArgs)> make_void_task(std::function<void(TArgs)> func)
			{
				return [func](TArgs arg)
				{
					func(arg);
					return 0;
				};
			}
			//创建接受void参数的方法
			template<typename TResult, typename TArgs = task_void_arg>
			inline std::function<TResult(TArgs)> make_void_arg_task(std::function<TResult()> func)
			{
				return [func](TArgs arg)
				{
					return func();
				};
			}
			//创建接受void返回void的方法
			inline std::function<task_void(task_void_arg)> make_void_task(std::function<void()> func)
			{
				return [func](task_void_arg arg)
				{
					func();
					return 0;
				};
			}
			//启动任务(接受void)
			template<typename TResult, typename TArgs>
			inline void start_task(const std::shared_ptr<task<TResult, TArgs>> &task_ptr, const TArgs &args)
			{
				task_ptr->start(args);
			}
			//启动任务
			template<typename TResult>
			inline void start_task(const std::shared_ptr<task<TResult, task_void_arg>> &task_ptr)
			{
				task_ptr->start(task_void_arg_start);
			}
			//创建任务
			template<typename TResult, typename TArgs>
			inline std::shared_ptr<task<TResult, TArgs>> make_task(std::function<TResult(TArgs)> func)
			{
				auto task_ptr = std::make_shared<task<TResult, TArgs>>(func, thread_option::create);
				return task_ptr;
			}
			//创建任务(接受void返回void)
			inline std::shared_ptr<task<task_void, task_void_arg>> make_task(std::function<void()> func)
			{
				auto task_ptr = make_task(make_void_task(func));
				return task_ptr;
			}
			//创建任务(返回void)
			template<typename TArgs>
			inline std::shared_ptr<task<task_void, TArgs>> make_task(std::function<void(TArgs)> func)
			{
				auto task_ptr = make_task(make_void_task(func));
				return task_ptr;
			}
			//创建任务 (接受void)
			template<typename TResult>
			inline std::shared_ptr<task<TResult, task_void_arg>> make_task(std::function<TResult()> func)
			{
				auto task_ptr = make_task(make_void_arg_task(func));
				return task_ptr;
			}
#pragma endregion
#pragma region then_task

			//延续任务
			template<typename TResult, typename TArgs>
			template<typename TOtherResult>
			std::shared_ptr<task<TOtherResult, TResult>> task<TResult, TArgs>::then(std::function<TOtherResult(TResult)> func)
			{
				//创建task
				std::shared_ptr<task<TOtherResult, TResult >> task_ptr = std::make_shared<task<TOtherResult, TResult>>(func, thread_option::no_create);
				//如果任务已完成
				if (status == task_status::done)
				{
					//直接启动task
					std::thread thread([task_ptr]() {
						task_ptr->start(future_ptr->get());
					});
					thread.detach();
				}
				//任务还没开始或运行中
				else if (status == task_status::ready || status == task_status::running)
				{
					//设置回调
					set_call_back([task_ptr](TResult result)
					{
						task_ptr->start(result);
					});
				}
				//任务出错
				//抛出异常
				else if (status == task_status::error)
				{
					try
					{
						future_ptr->get();
					}
					catch (const std::exception &e)
					{
						throw e;
					}
				}
				return task_ptr;
			}
			//延续任务(返回void)
			template<typename TResult, typename TArgs>
			std::shared_ptr<task<task_void, TResult>> task<TResult, TArgs>::then(std::function<void(TResult)> func)
			{
				return then(make_void_task<TResult>(func));
			}

			//延续任务(接受void)
			template<typename TResult, typename TArgs>
			template<typename TOtherResult>
			std::shared_ptr<task<TOtherResult, TResult>> task<TResult, TArgs>::then(std::function<TOtherResult()> func)
			{
				return then(make_void_arg_task<TOtherResult, TResult>(func));
			}

			//延续任务(接受void,返回void)
			template<typename TResult, typename TArgs>
			std::shared_ptr<task<task_void, TResult>> task<TResult, TArgs>::then(std::function<void()> func)
			{
				return then(make_void_task(func));
			}
#pragma endregion
		}
	}
}