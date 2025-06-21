#include "RateStrategyFactory.h"
#include "FixedLocalRate.h"
#include "FixedLongRate.h"
#include "WirelessLocalRate.h"
#include "WirelessLongRate.h"
#include "WirelessAnswerRate.h"
#include <memory>
std::shared_ptr<RateStrategy> RateStrategyFactory::createStrategy(PhoneNumber::ServiceType service, bool isLongDistance)
{

	switch (service) {
	case PhoneNumber::FIXED:
		if (isLongDistance) {
			return std::make_shared<FixedLongRate>();
		}
		return std::make_shared<FixedLocalRate>();

	case PhoneNumber::WIRELESS:
		if (isLongDistance) {
			return std::make_shared<WirelessLongRate>();
		}
		return std::make_shared<WirelessLocalRate>();

	default:

		return std::make_shared<WirelessLocalRate>();
	}
}

std::shared_ptr<RateStrategy> RateStrategyFactory::createanswerStrategy(PhoneNumber::ServiceType service, bool isAnswer)
{
	if (isAnswer && service == PhoneNumber::WIRELESS)
	{
		return std::make_shared<WirelessAnswerRate>();
	}
	else
	{
		//返回空指针
		return nullptr;
	}
	
}
