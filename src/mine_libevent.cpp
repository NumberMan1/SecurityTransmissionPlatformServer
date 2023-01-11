#include "mine_libevent.h"
#include <iostream>
#include <exception>
#include <vector>

std::shared_ptr<mine_libevent::EventHandler> 
mine_libevent::EventHandler::mInstance = nullptr;

mine_libevent::EventHandler::EventHandler() noexcept
    : mSockAddrInPtr(std::make_unique<sockaddr_in>())
    , mBasePtr(nullptr, event_base_free)
    , mine_libeventSignals()
    , mListenerPtr(nullptr, evconnlistener_free)
{
    mSockAddrInPtr->sin_family = AF_INET;
    mSockAddrInPtr->sin_port = htons(PORT);
    mSockAddrInPtr->sin_addr.s_addr = INADDR_ANY;
}

void mine_libevent::EventHandler::init(ListenerCallBack _cb) {
    auto base = event_base_new();
    if (!base) {
        LOG("不能创建event_base!");
        throw std::runtime_error("不能创建event_base!");
    }
    mBasePtr.reset(std::move(base));
    auto listener = 
        evconnlistener_new_bind(mBasePtr.get(), _cb, mBasePtr.get(),
                                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                reinterpret_cast<sockaddr*>(mSockAddrInPtr.get()), 
                                sizeof(*mSockAddrInPtr));
    if (!listener) {
        LOG("不能创建evconnlistener!");
        throw std::runtime_error("不能创建evconnlistener!");
    }
    mListenerPtr.reset(std::move(listener));
}

std::shared_ptr<mine_libevent::EventHandler> 
mine_libevent::EventHandler::instance(ListenerCallBack _cb) {
    if (!mInstance) {
        mInstance = std::move(
            std::shared_ptr<EventHandler>(new EventHandler())
        );
        mInstance->init(_cb);
    }
    return mInstance;
}

mine_libevent::EventHandler::~EventHandler() noexcept {
    std::cout << "~EventHandler 调用" << std::endl;
    mInstance = nullptr;
    try {
        // if (event_base_got_exit(mBasePtr.get()))
            // this->exit();
    } catch (...) {
        LOG("unknow error");
    }
} 
 
// bool mine_libevent::EventHandler::delEvent(const int &_fd) noexcept {
//     if (mine_libevents.contains(_fd)) {
//         mine_libevents.erase(_fd);
//         return true;
//     } else {
//         return false;
//     }
// }

bool mine_libevent::EventHandler::delSignal(const int &_fd) noexcept {
    if (mine_libeventSignals.contains(_fd)) {
        mine_libeventSignals.erase(_fd);
        return true;
    } else {
        return false;
    }
}

void mine_libevent::EventHandler::dispatch() {
    event_base_dispatch(mBasePtr.get());
}

void mine_libevent::EventHandler::exit(const timeval * _timeout) {
    std::cout << "someone exit" << std::endl;
    event_base_loopexit(mBasePtr.get(), _timeout);
}

const std::string_view mine_libevent::EventHandler::getBaseMethod() const {
    return event_base_get_method(mBasePtr.get());
}

// void mine_libevent::EventHandler::addEvent
// (SockFdType _fd, EventCallBack _cb, short _op, void *_arg, timeval *_timeout) {
//     event *ev = event_new(mBasePtr.get(), _fd, _op, _cb, _arg);
//     if (!ev || event_add(ev, _timeout) < 0) {
//         log("不能创建event! 或 event_add失败");
//         throw std::runtime_error("不能创建event! 或 event_add失败");
//     }
//     std::unique_ptr<event, decltype(event_free)*> ptr(std::move(ev), event_free);
//     mine_libevents.insert(std::make_pair<SockFdType, decltype(ptr)>(std::move(_fd), std::move(ptr)));
// }

void mine_libevent::EventHandler::addSignal
(int _x, SignalCallBack _cb, void *_arg, timeval *_timeout) {
    auto signal = evsignal_new(mBasePtr.get(), _x, _cb, _arg);
    if (!signal || evsignal_add(signal, _timeout)) {
        LOG("创建evsignal失败");
        throw std::runtime_error("创建evsignal失败");
    }
    std::unique_ptr<SignalEvent, decltype(event_free)*> ptr(std::move(signal), event_free);
    mine_libeventSignals.insert(std::make_pair<SockFdType, decltype(ptr)>(std::move(_x), std::move(ptr)));
}

