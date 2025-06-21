#pragma once
#include <QString>
class RateStrategy
{
public:
    virtual ~RateStrategy() = default;
    virtual double calculateCharge(int duration, const QString& plan) const = 0;
    virtual QString getCategory() const = 0;
    virtual QString getName() const = 0;
    virtual std::unique_ptr<RateStrategy> clone() const = 0;
};

