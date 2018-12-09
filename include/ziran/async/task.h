#pragma once
#include <ziran/async/thread_pool.h>
#include <memory>
#include <future>
namespace ziran
{
	namespace async
	{
		template<typename R>
		class _Task:public ziran::runable<void>
		{
			using runable_ptr = std::shared_ptr<ziran::runable<R>>;
		public:
			~_Task() = default;
			template<typename _Fn, typename ..._Args>
			_Task(_Fn &&f, _Args ...args)
				:m_action(ziran::make_action<R>(std::bind(f, args...)))
				, m_pool(GET_THREAD_POOL())
				, m_promise(std::make_shared<std::promise<R>>())
				, m_future(m_promise->get_future())
				, m_next()

			{
			}

			template<typename _Fn>
			_Task(_Fn &&f)
				:m_action(ziran::make_action<R>(f))
				, m_pool(GET_THREAD_POOL())
				, m_promise(std::make_shared<std::promise<R>>())
				, m_future(m_promise->get_future())
				, m_next()

			{
			}

			template<typename _t>
			struct completer
			{
				static void set_value(runable_ptr &call, std::shared_ptr<std::promise<R>> &promise, std::shared_ptr<ziran::runable<void>> next)
				{
					auto value = call->run();
					promise->set_value(value);
					if (next)
					{
						next->run();
					}
				}
			};
			template<>
			struct completer<void>
			{
				static void set_value(runable_ptr &call, std::shared_ptr<std::promise<R>> &promise, std::shared_ptr<ziran::runable<void>> next)
				{
					call->run();
					promise->set_value();
					if (next)
					{
						next->run();
					}
				}
			};

			void run() override
			{
				auto f = [](runable_ptr r
					, std::shared_ptr<std::promise<R>> promise
					,std::shared_ptr<ziran::runable<void>> next)
				{
					try
					{
						completer<R>::set_value(r, promise,next);
					}
					catch (const std::exception&)
					{
						promise->set_exception(std::current_exception());
					}
				};
				m_pool->run_task(std::bind(f, m_action, m_promise, m_next));
			}

			void wait()
			{
				m_future.wait();
			}

			R get()
			{
				return m_future.get();
			}

			bool valid()
			{
				return m_future.valid();
			}

			template<typename _Fn, typename ..._Args>
			static std::shared_ptr<_Task<R>> make(_Fn &fn,_Args &...args)
			{
				return std::make_shared<_Task<R>>(fn,args...);
			}

			template<typename _t,typename  _r>
			struct next_builder
			{
				template<typename _fn>
				static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<_t> &future)
				{
					return _Task<_r>::make([](_Fn &&fn, std::shared_future<_t> &future)
					{
						return std::bind(fn, future.get())();
					}, fn,future);
				}
			};

			template<typename _r>
			struct next_builder<void,_r>
			{
				template<typename _fn>
				static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<void> &future)
				{
					return _Task<_r>::make([](_fn &&fn, std::shared_future<void> &future)
					{
						return fn();
					}, fn, future);
				}
			};

			operator R()
			{
				return get();
			}

			template<typename _t, typename _r>
			struct next_builder<std::shared_ptr<_Task<_t>>, _r>
			{
				template<typename _fn>
				static std::shared_ptr<_Task<_r>> build(_fn &&fn, std::shared_future<_Task<_t>> &future)
				{
					std::shared_ptr<_Task<_t>> t = future.get();
					return t->then(fn);
				}
			};

			template<typename _R = void,typename _Fn>
			std::shared_ptr<_Task<_R>> then(_Fn &&fn)
			{
				auto t = next_builder<R, _R>::build(fn, m_future);
				if (valid())
				{
					t->run();
				}
				else
				{
					m_next = t;
				}
				return t;
			}

		private:
			runable_ptr m_action;
			THREAD_POOL m_pool;
			std::shared_ptr<std::promise<R>> m_promise;
			std::shared_future<R> m_future;
			std::shared_ptr<ziran::runable<void>> m_next;
		};
		template<typename T>
		using shared_task = std::shared_ptr<ziran::async::_Task<T>>;
#define TASK ziran::async::shared_task
		template<typename _R,typename _Fn,typename ..._Args>
		TASK<_R> make_task(_Fn &fn, _Args &...args)
		{
			return _Task<_R>::make(fn, args...);
		}
#define MAKE_TASK ziran::async::make_task
	}
}

