#pragma once
#include "RateStrategy.h"
class WirelessLocalRate :public RateStrategy
{
public:
    double calculateCharge(int duration, const QString& plan) const override;
    QString getCategory() const override { return "wireless_local"; }
    QString getName() const override { return "无线本地电话"; }
    std::unique_ptr<RateStrategy> clone() const {
        return std::make_unique<WirelessLocalRate>(*this);
    }
};

