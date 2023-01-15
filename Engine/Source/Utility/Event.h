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
        std::function<RetType(Args...)> func;
        bool operator < (EventData const& rhs) { return order > rhs.order; }
    };

    EventHandle Bind(std::function<RetType(Args...)> func, uint32_t order = 0) {
        EventData data;
        data.handle = ++_next_handle;
        data.order = order;
        data.func = func;
        _event_datas.push_back(data);
        return data.handle;
    }

    void Unbind(EventHandle handle) {
        for (typename std::vector<EventData>::iterator iter = _event_datas.begin(); iter != _event_datas.end(); iter++) {
            EventData& data = *iter;
            if (data.handle == handle) {
                _event_datas.erase(iter);
                break;
            }
        }
    }

    void Block() {
        _block = true;
    }

    void Dispatch(Args... args) {
        if (_event_datas.size() <= 0) {
            return;
        }

        // Sort callback
        std::sort(&_event_datas[0], &_event_datas[0] + _event_datas.size());

        _block = false;
        for (int i = 0; i < _event_datas.size(); ++i) {
            _event_datas[i].func(std::forward<Args>(args)...);

            // Handle block
            if (_block == true) {
                break;
            }
        }
    }
private:
    EventHandle _next_handle = 0;
    std::vector<EventData> _event_datas;
    bool _block;
};


