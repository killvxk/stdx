#pragma once
#include <memory>
#include <any>
#include <unordered_map>
#include <stdexcept>
namespace ziran
{
	namespace injector
	{
		template<typename T>
		class transient
		{
		public:
			transient()=default;
			~transient()=default;
			static std::function<T()> make(std::function<T()> &&maker)
			{
				return maker;
			}
		private:

		};

		template<typename T>
		class singleton
		{
		public:
			singleton()=default;
			~singleton()=default;
			static std::function<T()> make(std::function<T()> &&maker)
			{
				T value = maker();
				return [value]() 
				{
					return value;
				};
			}
		private:
		};

		class service_collection
		{
		public:
			service_collection()=default;
			~service_collection()=default;
			template<typename T,typename MakePolicy = transient<T>>
			service_collection &register_type(std::function<T()> &&maker = []() { return T(); })
			{
				std::string key(typeid(T).name());
				if (map.find(key) != std::end(map))
				{
					throw std::logic_error("该类型已被注册");
				}
				map.insert(std::make_pair(key,MakePolicy::make(std::move(maker))));
				return *this;
			}
			template<typename T>
			T resolve_type()
			{
				std::string key(typeid(T).name());
				auto itera = map.find(key);
				if (itera == std::end(map))
				{
					throw std::invalid_argument("该类型没有被注册");
				}
				std::function<T()> maker = std::any_cast<std::function<T()>>(itera->second);
				return maker();
			}
		private:
			std::unordered_map<std::string, std::any> map;
		};
	}
}