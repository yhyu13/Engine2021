#include "../utility/TypeHelper.h"
#include "../exception/EngineException.h"
#include "../thread/Lock.h"

namespace longmarch
{
	/**
	 * @brief Service Locator Pattern for easy instantiate derived classes
	 *
	 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
	 */
	class ServiceLocator : public BaseAtomicClassNI
	{
	public:
		NONINSTANTIABLE(ServiceLocator);
	private:
		struct service_locator_base
		{
			virtual bool empty() noexcept = 0;
			virtual std::shared_ptr<void> getService() noexcept = 0;
			virtual std::shared_ptr<void> copyService() noexcept = 0;
			std::string m_type_name;
		};

		template<typename T, bool IsSingleTon>
		struct service_locator_t : public service_locator_base
		{
			using service_type = T;
			service_locator_t() = delete;
			explicit service_locator_t(const std::shared_ptr<service_type>& s)
				:
				m_service(s)
			{
				m_type_name = std::move(std::string(typeid(service_type).name()));
			}
			virtual bool empty() noexcept override
			{
				return m_service == nullptr;
			}
			virtual std::shared_ptr<void> getService() noexcept override
			{
				return m_service;
			}
			virtual std::shared_ptr<void> copyService() noexcept override
			{
				if constexpr (IsSingleTon)
				{
					ENGINE_EXCEPT(L"copyService should not be called from a singleton registery!");
					return nullptr;
				}
				else
				{
					return MemoryManager::Make_shared<service_type>(*m_service.get());
				}
			}
		private:
			std::shared_ptr<service_type> m_service;
		};

	public:

		/**
		 * @brief Register for copyable classes
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 * @tparam T derived class type
		 * @param name name of the registery
		 */
		template<class T, typename ...Args>
		static void Register(const std::string& name, Args&&... args)
		{
			LOCK_GUARD_NI();
			auto _service = MemoryManager::Make_shared<T>(std::forward<Args>(args)...);
			s_nameLocator[name] = std::move(MemoryManager::Make_unique<service_locator_t<T, false>>(std::move(_service)));
		}
		/**
		 * @brief Register for copyable classes
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 * @tparam T derived class type
		 * @param name name of the registery
		 */
		template<class T>
		static void Provide(const std::string& name, std::shared_ptr<T> service)
		{
			LOCK_GUARD_NI();
			s_nameLocator[name] = MemoryManager::Make_unique<service_locator_t<T, false>>(std::move(service));
		}
		/**
		 * @brief Register for non copyable classes
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 * @tparam T derived class type
		 * @param name name of the registery
		 */
		template<class T, typename ...Args>
		static void RegisterSingleton(const std::string& name, Args&&... args)
		{
			LOCK_GUARD_NI();
			auto _service = MemoryManager::Make_shared<T>(std::forward<Args>(args)...);
			s_nameLocator[name] = MemoryManager::Make_unique<service_locator_t<T, true>>(std::move(_service));
		}
		/**
		 * @brief Register for non copyable classes
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 * @tparam T derived class type
		 * @param name name of the registery
		 */
		template<class T>
		static void ProvideSingleton(const std::string& name, std::shared_ptr<T> service)
		{
			LOCK_GUARD_NI();
			s_nameLocator[name] = MemoryManager::Make_unique<service_locator_t<T, true>>(std::move(service));
		}
		/**
		 * @brief Get stored service
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 * @tparam U base class type
		 * @param name name of the registery
		 */
		template<class U>
		[[nodiscard]] static U* GetSingleton(const std::string& name)
		{
			LOCK_GUARD_NI();
			if (auto it = s_nameLocator.find(name); it != s_nameLocator.end()) [[likely]]
			{
				if (!it->second->empty()) [[likely]]
				{
					if (auto ptr = std::static_pointer_cast<U>(it->second->getService()); ptr) [[likely]]
					{
						return ptr.get();
					}
					else [[unlikely]]
					{
						ENGINE_EXCEPT(str2wstr(name) + L" which has type : " + str2wstr(it->second->m_type_name) + L" could not be cast to type : " + wStr(typeid(U).name()));
						return nullptr;
					}
				}
				else [[unlikely]]
				{
					ENGINE_EXCEPT(str2wstr(name) + L" which has type : " + str2wstr(it->second->m_type_name) + L" is empty!");
					return nullptr;
				}
			}
			else [[unlikely]]
			{
				ENGINE_EXCEPT(str2wstr(name) + L" is not registered yet!");
				return nullptr;
			}
		}
		/**
		 * @brief Get a new copy of the stored service
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 * @tparam U base class type
		 * @param name name of the registery
		 */
		template<class U>
		[[nodiscard]] static std::shared_ptr<U> GetNewInstance(const std::string& name)
		{
			LOCK_GUARD_NI();
			if (auto it = s_nameLocator.find(name); it != s_nameLocator.end()) [[likely]]
			{
				if (!it->second->empty()) [[likely]]
				{
					if (auto ptr = std::static_pointer_cast<U>(it->second->copyService()); ptr) [[likely]]
					{
						return ptr;
					}
					else [[unlikely]]
					{
						ENGINE_EXCEPT(str2wstr(name) + L" which has type : " + str2wstr(it->second->m_type_name) + L" could not be cast to type : " + wStr(typeid(U).name()));
						return nullptr;
					}
				}
				else [[unlikely]]
				{
					ENGINE_EXCEPT(str2wstr(name) + L" which has type : " + str2wstr(it->second->m_type_name) + L" is empty!");
					return nullptr;
				}
			}
			else [[unlikely]]
			{
				ENGINE_EXCEPT(str2wstr(name) + L" is not registered yet!");
				return nullptr;
			}
		}
	private:
		inline static LongMarch_UnorderedMap_Par<std::string, LongMarch_Unique_ptr<service_locator_base>> s_nameLocator;
	};
}
