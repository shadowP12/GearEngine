#pragma once

#include <vector>
#include <functional>
#include <map>
#include <algorithm>

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
    }

    struct EventData {
        EventHandle handle = 0;
        uint32_t order = 0;
        bool block = false;
        std::function<RetType(Args...)> func;
        bool operator < (EventData const& rhs) { return order > rhs.order; }
    };

    EventHandle bind(std::function<RetType(Args...)> func, uint32_t order = 0, bool block = false) {
        EventData data;
        data.handle = ++mNextHandle;
        data.order = order;
        data.block = block;
        data.func = func;
        mEventDatas.push_back(data);
        return mNextHandle;
    }

    void unbind(EventHandle handle) {
        for (typename std::vector<EventData>::iterator iter = mEventDatas.begin(); iter != mEventDatas.end(); iter++) {
            // 为了让编译通过，额外加了一次转化
            EventData& data = *iter;
            if (data.handle == handle) {
                mEventDatas.erase(iter);
                break;
            }
        }
    }

    void block(EventHandle handle, bool block) {
        for (typename std::vector<EventData>::iterator iter = mEventDatas.begin(); iter != mEventDatas.end(); iter++) {
            // 为了让编译通过，额外加了一次转化
            EventData& data = *iter;
            if (data.handle == handle) {
                data.block = block;
                break;
            }
        }
    }

    void dispatch(Args... args) {
        // 对回调函数进行排序
        std::sort(&mEventDatas[0], &mEventDatas[0] + mEventDatas.size());

        for (int i = 0; i < mEventDatas.size(); ++i) {
            mEventDatas[i].func(std::forward<Args>(args)...);

            // 处理截断条件
            if (mEventDatas[i].block) {
                break;
            }
        }
    }
private:
    EventHandle mNextHandle = 0;
    std::vector<EventData> mEventDatas;
};


