#ifndef FACTORY_H
#define FACTORY_H

#include "msg.h"
#include <memory>

namespace transmission_msg {
namespace proto {
namespace factory {
    
class Factory {
public:
    explicit Factory() = default;
    virtual ~Factory() = default;
    inline std::shared_ptr<Msg> CreateMsg() {
        return nullptr;
    }
};

class RequestFactory :
    public Factory {
public:
	explicit RequestFactory(std::string_view enc)
        : Factory{}
        , flag_{false}
        , enc_str_{enc} {}
	explicit RequestFactory(RequestInfo *info)
        : Factory{}
        , flag_{true}
        , info_{info} {}
	virtual ~RequestFactory() = default;
	inline std::shared_ptr<RequestMsg> CreateMsg() {
        if (flag_) {
            return std::make_shared<RequestMsg>(info_);
        } else {
            return std::make_shared<RequestMsg>(enc_str_);
        }
    }

private:
	bool flag_;
	std::string enc_str_;
	RequestInfo *info_ = nullptr;
};

class RespondFactory
    : public Factory {
public:
	RespondFactory(std::string_view enc)
        : Factory{}
        , flag_{false}
        , enc_str_{enc} {}
	RespondFactory(RespondInfo* info)
        : Factory{}
        , flag_{true}
        , info_{info} {}
	virtual ~RespondFactory() = default;
	inline std::shared_ptr<RespondMsg> CreateMsg() {
        if (flag_) {
            return std::make_shared<RespondMsg>(info_);
        } else {
            return std::make_shared<RespondMsg>(enc_str_);
        }
    }


private:
	bool flag_;
	std::string enc_str_;
	RespondInfo* info_ = nullptr;
	// std::shared_ptr<Msg> info_;
};

} // namespace factory
} // namespace proto
} // namespace transmission_msg



#endif // FACTORY_H
