#pragma once
#include "RateStrategy.h"
class FixedLocalRate :public RateStrategy
{
public:
    double calculateCharge(int duration, const QString& plan) const override;
    QString getCategory() const override { return "fixed_local"; }
    QString getName() const override { return "固定本地电话"; }
    std::unique_ptr<RateStrategy> clone() const {
        return std::make_unique<FixedLocalRate>(*this);
    }   
};

