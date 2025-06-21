#pragma once
#include "RateStrategy.h"
class FixedLongRate :public RateStrategy
{
public:
    double calculateCharge(int duration, const QString& plan) const override;
    QString getCategory() const override { return "fixed_long"; }
    QString getName() const override { return "固定长途电话"; }
    std::unique_ptr<RateStrategy> clone() const {
        return std::make_unique<FixedLongRate>(*this);
    }
};

