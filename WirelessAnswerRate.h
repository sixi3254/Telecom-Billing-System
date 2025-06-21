#pragma once
#include "RateStrategy.h"
class WirelessAnswerRate :public RateStrategy
{
	double calculateCharge(int duration, const QString& plan) const override;
	QString getCategory() const override { return "wireless_answer"; }
	QString getName() const override { return "无线接受电话"; }
	std::unique_ptr<RateStrategy> clone() const {
		return std::make_unique<WirelessAnswerRate>(*this);
	}
};

