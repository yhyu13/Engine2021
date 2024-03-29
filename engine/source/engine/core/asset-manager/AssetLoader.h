#pragma once
#include "ResourceManager.h"
#include "../utility/Timer.h"
#include "../utility/TypeHelper.h"
#include "../thread/ThreadUtil.h"

namespace longmarch
{
	/**
	 * @brief Custom AssetManager that features async loading
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class ENGINE_API AssetLoader : BaseAtomicClassNI
	{
	public:
		NONINSTANTIABLE(AssetLoader);
		using DataSourceRef = std::shared_ptr<void>;
		using AsyncCallBack = std::function<DataSourceRef(const fs::path&)>;
		using MainThreadCallBack = std::function<void(DataSourceRef)>;

		//! Options passed when creating an asset Loader.
		struct Options
		{
			Options() = default;
			Options(bool async, bool Watch) : Asynchronous(async), Watch(Watch) {}
			bool Asynchronous{ false };
			bool Watch{ false }; //Put the asset in the watch list, and track whether the asset is modified. If modifies, auto reload asset
		};

		//! Blocking loader
		struct Loader
		{
			NONCOPYABLE(Loader);
			Loader() = default;
			explicit Loader(const fs::path& relativePath, const AsyncCallBack& callback, const Options& options = { false, false })
				:
				m_path(FileSystem::Absolute(FileSystem::ResolveProtocol(relativePath))),
				m_callback(callback),
				m_options(options)
			{
				ENGINE_EXCEPT_IF(!fs::exists(m_path), L"Path does not exist : " + m_path.wstring());
				if (options.Watch)
				{
					m_lastTimeWritten = fs::last_write_time(m_path);
				}
			}
			const fs::path& Path() const
			{
				return m_path;
			}
			//! Returns whether the asset needs to be reloaded or not.
			bool Watch()
			{
				if (!m_options.Watch)
				{
					throw std::logic_error("Watch should be called on loader with Option::Watch set to true!");
					return false;
				}
				if (auto ftime = fs::last_write_time(m_path); m_lastTimeWritten != ftime)
				{
					m_lastTimeWritten = ftime;
					return true;
				}
				else
				{
					return false;
				}
			}
			//! Loads the asset and Notify the Callback function with the new DataSourceRef.
			virtual void Notify() { m_data = m_callback(m_path); m_loaded = true; }
			//! Checks for completeness and other features presents in subclasses.
			virtual void Update() {}
			//! Returns whether the asset has been or not.
			virtual bool IsLoaded() const
			{
				if (!m_loaded)
				{
					return false;
				}
				else
				{
					if (m_data != nullptr)
					{
						return true;
					}
					else
					{
						ENGINE_EXCEPT(L"Loader failed : " + m_path.wstring());
						return false;
					}
				}
			}

		protected:
			friend AssetLoader;
			std::atomic_bool m_loaded{ false };
			Options m_options;

			fs::path m_path;
			std::filesystem::file_time_type m_lastTimeWritten;
			AsyncCallBack m_callback;
			DataSourceRef m_data{ nullptr };
		};

		//! AsyncLoader
		class AsyncLoader final : public Loader
		{
		public:
			NONCOPYABLE(AsyncLoader);
			AsyncLoader() = default;
			explicit AsyncLoader(const fs::path& relativePath, const AsyncCallBack& asyncCallback, const MainThreadCallBack& mainThreadCallback, const Options& options = { true, false })
				:
				Loader(relativePath, asyncCallback, options),
				m_mainThreadCallback(mainThreadCallback)
			{}
			//! Loads the asset in a separated thread and Notify the Callback function with the new DataSourceRef.
			virtual void Notify() override
			{
				m_loaded = false;
				std::thread([this]()
				{
					m_data = m_callback(m_path);
					m_loaded = true;
				}).detach();
			}
			//! Called in the main thread
			virtual void Update() override
			{
				if (IsLoaded())
				{
					if (m_mainThreadCallback)
					{
						m_mainThreadCallback(m_data);
					}
				}
			}

		protected:
			MainThreadCallBack m_mainThreadCallback;
		};

		using LoaderRef = std::shared_ptr<Loader>;

	public:
		//! Load with the default job queue
		inline static void Load(const fs::path& relativePath, const AsyncCallBack& asyncCallback, const Options& options)
		{
			LoaderRef loader;
			if (options.Asynchronous)
			{
				throw std::logic_error("AssetManager Load without a MainThreadCallback provided should have Option::Asynchronous set to false!");
			}
			else
			{
				loader = MemoryManager::Make_shared<Loader>(relativePath, asyncCallback, options);
				loader->Notify();
				ENGINE_EXCEPT_IF(!loader->IsLoaded(), L"Loader failed : " + loader->m_path.wstring());
			}
			if (options.Watch)
			{
				s_watcherQueue.push(loader);
			}
			//! Check status on sychronous loader
		}

		//! Load with the async callback and main thread callback, return nullptr if there exists a loader with the same path
		inline static LoaderRef Load(const fs::path& relativePath, const AsyncCallBack& asyncCallback, const MainThreadCallBack& mainThreadCallback, const Options& options, bool managedAsync)
		{
			LoaderRef loader;
			if (options.Asynchronous)
			{
				loader = MemoryManager::Make_shared<AsyncLoader>(relativePath, asyncCallback, mainThreadCallback, options);
				if (RecordLoading(loader))
				{
					loader->Notify();
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				throw std::logic_error("AssetManager Load with a MainThreadCallback provided should have Option::Asynchronous set to true!");
			}
			if (managedAsync)
			{
				s_asyncLoaderQueue.push(loader);
			}
			if (options.Watch)
			{
				s_watcherQueue.push(loader);
			}
			return loader;
		}

		//! Update job in the default job queue
		inline static void Update()
		{
			// Check watch list
			if (s_watcherTimer.Check(true))
			{
				std::vector<LoaderRef> watchedRefs;
				while (!s_watcherQueue.empty())
				{
					auto loader = s_watcherQueue.pop_front();
					if (loader->Watch())
					{
						PRINT("Reload : " + loader->m_path.string());
						if (loader->m_options.Asynchronous)
						{
							if (RecordLoading(loader))
							{
								loader->Notify();
								s_asyncLoaderQueue.push(loader);
							}
						}
						else // Check status on sychronous loader
						{
							loader->Notify();
							ENGINE_EXCEPT_IF(!loader->IsLoaded(), L"Loader failed : " + loader->m_path.wstring());
						}
					}
					watchedRefs.push_back(std::move(loader));
				}
				for (auto&& loadRef : watchedRefs)
				{
					s_asyncLoaderQueue.push(std::move(loadRef));
				}
			}
			// Check load list
			{
				std::vector<LoaderRef> notLoadedRefs;
				while (!s_asyncLoaderQueue.empty())
				{
					auto loader = s_asyncLoaderQueue.pop_front();
					if (loader->IsLoaded())
					{
						loader->Update();
						UnrecordLoading(loader);
						DEBUG_PRINT(Str("Done loading #%d: %s", ++s_asyncLoaderCounter, loader->m_path.string().c_str()));
					}
					else
					{
						notLoadedRefs.push_back(std::move(loader));
					}
				}
				for (auto&& loadRef : notLoadedRefs)
				{
					s_asyncLoaderQueue.push(std::move(loadRef));
				}
			}
		}

		//! Wait for all job to finish
		inline static void WaitForAll(LongMarch_Vector<LoaderRef>& waitList)
		{
			while (!waitList.empty())
			{
				for (auto it = waitList.begin(); it != waitList.end();)
				{
					if (auto& loader = *it; loader)
					{
						if (loader->IsLoaded())
						{
							loader->Update();
							UnrecordLoading(loader);
							DEBUG_PRINT(Str("Done loading #%d: %s", ++s_asyncLoaderCounter, loader->m_path.string().c_str()));
							it = waitList.erase(it);
						}
						else
						{
							++it;
						}
					}
					else //!< remove nullptr loader
					{
						it = waitList.erase(it);
					}
				}
			}
		}

	private:
		//! Check if there exist a loader
		inline static bool RecordLoading(const LoaderRef& loader)
		{
			LOCK_GUARD_NI();
			if (const auto& path = loader->m_path.string();
				s_pathInLoadingLUT.contains(path))
			{
				return false;
			}
			else
			{
				s_pathInLoadingLUT[loader->m_path.string()] = true;
				return true;
			}
		}
		inline static void UnrecordLoading(const LoaderRef& loader)
		{
			LOCK_GUARD_NI();
			s_pathInLoadingLUT.erase(loader->m_path.string());
		}
	private:
		inline static Timer s_watcherTimer{ 2.0 }; //!< timer for periodically checking watched files
		inline static LongMarch_UnorderedMap_flat<std::string, bool> s_pathInLoadingLUT;
		inline static ConcurrentQueueNC<LoaderRef> s_watcherQueue;
		inline static ConcurrentQueueNC<LoaderRef> s_asyncLoaderQueue;
		inline static int32_t s_asyncLoaderCounter{0};
	};
}
