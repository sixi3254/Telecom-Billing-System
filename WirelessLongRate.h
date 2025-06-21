#pragma once
#include "RateStrategy.h"
class WirelessLongRate :public RateStrategy
{
	double calculateCharge(int duration, const QString& plan) const override;
	QString getCategory() const override { return "wireless_long"; }
	QString getName() const override { return "无线长途电话"; }
	std::unique_ptr<RateStrategy> clone() const {
		return std::make_unique<WirelessLongRate>(*this);
	}
};

