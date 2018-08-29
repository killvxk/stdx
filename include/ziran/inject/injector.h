#pragma once
#include <memory>
#include <any>
#include <unordered_map>
#include <stdexcept>
namespace ziran
{
	namespace injector
	{
		//短暂生命周期
		//每一次都创建
		template<
			//类型
			typename T
		>
		class transient
		{
		public:
			transient()=default;
			~transient()=default;
			static std::function<T()> make(std::function<T()> &&maker)
			{
				//直接返回
				return maker;
			}
		private:

		};

		//单例生命周期
		//只有单个实例
		template<typename T>
		class singleton
		{
		public:
			singleton()=default;
			~singleton()=default;
			static std::function<T()> make(std::function<T()> &&maker)
			{
				//获取实例
				T value = maker();
				//创建函数对象
				//拷贝实例(必须保证函数对象只有一个)
				return [value]() 
				{
					//返回实例
					return value;
				};
			}
		private:
		};

		//服务容器
		class service_collection
		{
		public:
			service_collection()=default;
			~service_collection()=default;

			//注册类型
			template<
				//类型
				typename T
				//策略
				,typename MakePolicy = transient<T>
			>
			service_collection &register_type(std::function<T()> &&maker = []() { return T(); })
			{
				//获取类型名称
				std::string key(typeid(T).name());
				//判断是否被注册
				if (map.find(key) != std::end(map))
				{
					//如果是
					//抛出异常
					throw std::logic_error("该类型已被注册");
				}
				//加入map
				map.insert(std::make_pair(key,MakePolicy::make(std::move(maker))));
				return *this;
			}
			//获取类型
			template<typename T>
			T resolve_type()
			{
				//获取类型名称
				std::string key(typeid(T).name());
				//寻找迭代器
				auto itera = map.find(key);
				//如果没注册
				if (itera == std::end(map))
				{
					//抛出异常
					throw std::invalid_argument("该类型没有被注册");
				}
				//获取方法
				std::function<T()> maker = std::any_cast<std::function<T()>>(itera->second);
				//执行方法后返回
				return maker();
			}
		private:
			std::unordered_map<std::string, std::any> map;
		};
	}
}