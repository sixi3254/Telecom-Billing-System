#pragma once
#include "RateStrategy.h"
#include "PhoneNumber.h"
#include <memory>
class RateStrategyFactory
{
public:
	static std::shared_ptr<RateStrategy> createStrategy(PhoneNumber::ServiceType service, bool isLongDistance);
	static std::shared_ptr<RateStrategy> createanswerStrategy(PhoneNumber::ServiceType service,  bool isAnswer);
};

