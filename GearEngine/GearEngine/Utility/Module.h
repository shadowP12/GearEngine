#ifndef MODULE_H
#define MODULE_H
#include <iostream>

#define ERROR(info) throw std::runtime_error(info)

template <class T>
class Module {
public:
	static T& instance() {
		if (!isStartedUp()) {
			ERROR("Trying to access a module but it hasn't been started up yet.");
		}

		if (isDestroyed()) {
			ERROR("Trying to access a destroyed module.");
		}

		return *_instance();
	}

	static T* instancePtr() {
		if (!isStartedUp()) {
			ERROR("Trying to access a module but it hasn't been started up yet.");
		}

		if (isDestroyed()) {
			ERROR("Trying to access a destroyed module.");
		}

		return _instance();
	}

	template<class ...Args>
	static void startUp(Args &&...args) {
		if (isStartedUp()) ERROR("Trying to start an already started module.");

		_instance() = new T(std::forward<Args>(args)...);
		isStartedUp() = true;

		((Module*)_instance())->onStartUp();
	}

	static void shutDown() {
		if (isDestroyed()) {
			ERROR("Trying to shut down an already shut down module.");
		}

		if (!isStartedUp()) {
			ERROR("Trying to shut down a module which was never started.");
		}

		((Module*)_instance())->onShutDown();

		delete(_instance());
		isDestroyed() = true;
	}

	static bool isStarted() {
		return isStartedUp() && !isDestroyed();
	}

protected:
	Module() = default;

	virtual ~Module() = default;

	Module(Module&&) = default;
	Module(const Module&) = default;
	Module& operator=(Module&&) = default;
	Module& operator=(const Module&) = default;


	virtual void onStartUp() {}


	virtual void onShutDown() {}

	static T*& _instance() {
		static T* inst = nullptr;
		return inst;
	}


	static bool& isDestroyed() {
		static bool inst = false;
		return inst;
	}

	static bool& isStartedUp() {
		static bool inst = false;
		return inst;
	}
};

#endif