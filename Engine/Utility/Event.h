#pragma once

#include <functional>
#include <map>

#define CALLBACK_0(func, object, ...) std::bind(&func, object, ##__VA_ARGS__)

#define CALLBACK_1(func, object, ...) std::bind(&func, object, std::placeholders::_1, ##__VA_ARGS__)

#define CALLBACK_2(func, object, ...) std::bind(&func, object, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)

#define CALLBACK_3(func, object, ...) std::bind(&func, object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)

#define CALLBACK_4(func, object, ...) std::bind(&func, object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, ##__VA_ARGS__)

typedef uint32_t EventHandle;

template <class RetType, class... Args>
class Event
{
public:
    Event() {
    }

    ~Event() {
        mFuncs.clear();
    }

    void bind(std::function<RetType(Args...)> func) {
        mFuncs[mNextHandle] = func;
        mNextHandle++;
    }

    void unbind(EventHandle handle) {
        typename std::map<EventHandle, std::function<RetType(Args...)>>::iterator it = mFuncs.find(handle);
        if (it != mFuncs.end()) {
            mFuncs.erase(it);
        }
    }

    void dispatch(Args... args) {
        for (auto& func : mFuncs) {
            func.second(std::forward<Args>(args)...);
        }
    }
private:
    EventHandle mNextHandle = 0;
    std::map<EventHandle, std::function<RetType(Args...)>> mFuncs;
};


