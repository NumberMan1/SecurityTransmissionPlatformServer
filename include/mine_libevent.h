/*t
    基于libevent的进一步封装
*/
#ifndef MINE_LIBEVENT_H_
#define MINE_LIBEVENT_H_

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <memory>
#include <map>
#include "log.hpp"

namespace mine_libevent{

using Base = event_base;
using SignalEvent = event;
using ConListener = evconnlistener;
using SockFdType = int;

using SignalCallBack = void (*)(SockFdType _x, short _events, void *_arg);
using ListenerCallBack = void (*)(ConListener *_evl,
                                  SockFdType _fd,
                                  sockaddr *_cliaddr,
                                  int _socklen, void *_arg);
using BufferReadCallBack =  void (*)(bufferevent *_bev, void *_ctx);
using BufferWriteCallBack = BufferReadCallBack;
using BufferEventCallBack = void (*)(bufferevent *_bev, short _what, void *_ctx);

/*
若要想设置持续的读事件则： EV_READ | EV_PERSIST
        EV_TIMEOUT    超时事件
        EV_READ       读事件
        EV_WRITE      写事件
        EV_SIGNAL     信号事件
        EV_PERSIST    周期性触发
        EV_ET         边缘触发, 如果底层模型支持设置则有效, 若不支持则无效.

以下为ConListener的falgs
        LEV_OPT_LEAVE_SOCKETS_BLOCKING  文件描述符为阻塞的
 	    LEV_OPT_CLOSE_ON_FREE           关闭时自动释放
 	    LEV_OPT_REUSEABLE               端口复用
 	    LEV_OPT_THREADSAFE              分配锁, 线程安全
*/

enum {
    PORT = 8888
};

class EventHandler {
public:
    ~EventHandler() noexcept;
    EventHandler(const EventHandler&) = delete;
    EventHandler(EventHandler&&) noexcept = delete;
    EventHandler& operator=(const EventHandler&) = delete;
    EventHandler& operator=(EventHandler&&) noexcept = delete;
    // 实例化对象
    static std::shared_ptr<EventHandler> instance(ListenerCallBack _cb);
    static std::shared_ptr<EventHandler> mInstance;
    // 开启循环
    void dispatch();
    // 结束循环
    void exit(const timeval *_timeout = nullptr);
    // // 删除对应_fd的event
    // bool delEvent(const int &_fd) noexcept;
    // 删除对应_fd的信号事件
    bool delSignal(const int &_fd) noexcept;
    // 返回当前event_base的方法
    const std::string_view  getBaseMethod() const;
    // // 添加对应_fd的事件
    // void addEvent(SockFdType _fd,
    //               EventCallBack _cb,
    //               short _op,
    //               void *_arg = nullptr,
    //               timeval *_timeout = nullptr);
    // 添加对应_x的信号事件
    void addSignal(int _x,
                   SignalCallBack _cb,
                   void *_arg = nullptr,
                   timeval *_timeout = nullptr); 
    inline Base* getBase() const {
        return mBasePtr.get();
    }
private:
    EventHandler() noexcept;
    void init(ListenerCallBack _cb);
    std::unique_ptr<sockaddr_in> mSockAddrInPtr;
    std::unique_ptr<Base, decltype(event_base_free)*> mBasePtr;
    std::map<int, std::unique_ptr<event, decltype(event_free)*>> mine_libeventSignals;
    std::unique_ptr<ConListener, decltype(evconnlistener_free)*> mListenerPtr;
};

} // ! namespace mine_libevent
#endif // ! MINE_LIBEVENT_H_