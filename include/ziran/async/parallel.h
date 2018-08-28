#pragma once
#include <vector>
#include <memory>
#include <initializer_list>
#include <functional>
#include <omp.h>
namespace ziran
{
	namespace async
	{
		//异步结果集
		template<typename T,typename TContainer = std::vector<T>>
		struct parallel_result
		{
			std::shared_ptr<TContainer> container_ptr;
			void set_result(std::shared_ptr<TContainer> ptr)
			{
				container_ptr = ptr;
			}
		};
		namespace parallel
		{
			//并行调用
			template<
				//结果类型
				typename T
				//容器类型
				,typename TContainer = std::vector<T>
				//结果集类型
				,typename TResult = parallel_result<T,TContainer>
			>
			TResult invoke(std::initializer_list<std::function<T()>> &&func_list)
			{
				//创建结果集
				TResult result;
				//创建容器指针
				std::shared_ptr<TContainer> container_ptr = std::make_shared<TContainer>();
				//获取大小
				size_t size = func_list.size();
				//OpenMP for循环调用
				#pragma omp parallel for schedule(dynamic)
				for (int i = 0; i < size; i++)
					container_ptr->push_back((*(func_list.begin() + i))());

				//设置结果
				result.set_result(container_ptr);
				//返回结果
				return result;
			}

			//并行调用
			void invoke_void(std::initializer_list<std::function<void()>> &&func_list)
			{
				//获取大小
				size_t size = func_list.size();
				//OpenMP for 循环调用
				#pragma omp parallel for schedule(dynamic)
				for (int i = 0; i < size; i++)
					(*(func_list.begin() + i))();
				return;
			}
			//并行for_each
			template<
				//类型
				typename T
				//容器类型
				,typename TContainer = std::vector<T>
			>
			void for_each(const TContainer &container,std::function<void(T&)> &&func)
			{
				//获取大小
				size_t size = container.size();
				//OpenMP for循环调用
				#pragma omp parallel for schedule(dynamic)
				for (int i = 0; i < size; i++)
				{
					func(container[i]);
				}
			}
			//并行调用
			void for_invoke(std::function<void()> &&func,size_t &&count)
			{
				//OpenMP for循环调用
				#pragma omp parallel for schedule(dynamic)
				for (int i = 0; i < count; i++)
				{
					func();
				}
			}
		}
	}
}