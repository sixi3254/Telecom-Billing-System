#include "FixedLongRate.h"

const double FIXED_LONG_RATE = 0.02; // 元/秒

double FixedLongRate::calculateCharge(int duration, const QString& plan) const
{
    Q_UNUSED(plan);
    return FIXED_LONG_RATE * duration; // 按秒计费
}
