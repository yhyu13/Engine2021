#include "engine-precompiled-header.h"

// By Emil Ernerfeldt 2014-2016
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.

#include "coroutine.h"
#include "../exception/EngineException.h"
#include <algorithm>
#include <thread>

namespace longmarch
{
	namespace coroutine
	{
		// This is thrown when the outer thread stop() the coroutine.
		struct AbortException {};

		// ----------------------------------------------------------------------------

		// Trick for allowing forward-declaration of std::thread.
		class Coroutine::Thread : public std::thread
		{
		public:
			using std::thread::thread;
		};

		// ----------------------------------------------------------------------------

		Coroutine::Coroutine(std::string_view debug_name_base, std::function<void(InnerControl& ic)> fun)
		{
			_debug_name = std::string(debug_name_base) + "_" + std::to_string(s_cr_counter++);
			_mutex.lock();
			_inner = std::make_unique<InnerControl>(*this);

			_thread = std::make_unique<Thread>([this, fun] {
				DEBUG_PRINT(_debug_name + " : Coroutine thread starting up");

				_mutex.lock();
				ASSERT(!_control_is_outer, "_control_is_outer should be false!");

				try {
					fun(*_inner);
				}
				catch (AbortException&) {
					DEBUG_PRINT(_debug_name + " : AbortException caught!");
				}
				catch (std::exception& e) {
					ENGINE_EXCEPT(wStr(_debug_name) + L" : Exception caught from Coroutine: " + wStr(e.what()) + L"!");
				}
				catch (...) {
					ENGINE_EXCEPT(wStr(_debug_name) + L" : Unknown exception caught from Coroutine!");
				}
				_is_done = true;
				_control_is_outer = true;
				_mutex.unlock();
				_cond.notify_one();
			});
			ASSERT(_inner, "_inner should not be nullptr!");
		}

		Coroutine::~Coroutine()
		{
			stop();
			ASSERT(!_thread, "_thread should be nullptr!");
		}

		void Coroutine::stop()
		{
			if (_thread) {
				if (!_is_done) {
					DEBUG_PRINT(_debug_name + " : Aborting coroutine");
					_abort = true;
					while (!_is_done) {
						poll(0);
					}
				}
				_thread->join();
				ASSERT(_is_done, "_is_done should be true!");
				ASSERT(_control_is_outer, "_control_is_outer should be true!");
				_mutex.unlock(); // We need to unlock before destroying it.
				_thread = nullptr;
			}
		}

		// Returns 'true' on done
		void Coroutine::poll(double dt)
		{
			ASSERT(_inner, "_inner should not be nullptr!");
			if (_is_done) { return; }

			ASSERT(_control_is_outer, "_control_is_outer should be true!");
			_control_is_outer = false;
			_mutex.unlock();
			_cond.notify_one();

			// Let the inner thread do it's business. Wait for it to return to us:

			std::unique_lock<std::mutex> lock(_mutex);
			_cond.wait(lock, [&] { return !!_control_is_outer; });
			lock.release(); // Keep the _mutex locked.
			ASSERT(_control_is_outer, "_control_is_outer should be true!");

			_inner->_time += dt;
		}

		// ----------------------------------------------------------------------------

		void InnerControl::wait_sec(double s)
		{
			auto target_time = _time + s;
			wait_for([=]() { return target_time <= _time; });
		}

		void InnerControl::yield()
		{
			ASSERT(!_cr._control_is_outer, "_control_is_outer should be false!");
			_cr._control_is_outer = true;
			_cr._mutex.unlock();
			_cr._cond.notify_one();

			// Let the outer thread do it's business. Wait for it to return to us:

			std::unique_lock<std::mutex> lock(_cr._mutex);
			_cr._cond.wait(lock, [=] { return !_cr._control_is_outer; });
			lock.release(); // Keep the _mutex locked.
			ASSERT(!_cr._control_is_outer, "_control_is_outer should be false!");

			if (_cr._abort) {
				throw AbortException();
			}
		}

		// ----------------------------------------------------------------------------

		void CoroutineSet::clear()
		{
			_list.clear();
		}

		std::shared_ptr<Coroutine> CoroutineSet::start(std::string_view debug_name, std::function<void(InnerControl& ic)> fun)
		{
			auto cr_ptr = std::make_shared<Coroutine>(debug_name, std::move(fun));
			_list.push_back(cr_ptr);
			return cr_ptr;
		}

		bool CoroutineSet::erase(const std::shared_ptr<Coroutine>& cr_ptr)
		{
			if (auto it = std::find(begin(_list), end(_list), cr_ptr); it != _list.end()) {
				// Do not modify _list - we might be currently iterating over it!
				// Instead, just clear the pointer, and let poll() erase it later.
				it->reset();
				return true;
			}
			else {
				return false;
			}
		}

		void CoroutineSet::poll(double dt)
		{
			// Take care to allow calls to start(), erase() and clear() while this is running (from within poll()):
			for (size_t i = 0; i < _list.size(); ++i) {
				auto& cr_ptr = _list[i];
				if (cr_ptr) {
					cr_ptr->poll(dt);
					if (cr_ptr->done()) {
						_list[i] = nullptr;
					}
				}
			}
			auto _check = [&](const auto& cr_ptr) { return cr_ptr == nullptr; };
			_list.erase(std::remove_if(_list.begin(), _list.end(), _check), _list.end());
		}
	} // namespace emilib
}
