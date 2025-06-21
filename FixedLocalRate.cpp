#include "FixedLocalRate.h"

const double FIXED_LOCAL_RATE = 0.06; // 元/分钟

double FixedLocalRate::calculateCharge(int duration, const QString& plan) const
{
    Q_UNUSED(plan);
    double minutes = std::ceil(duration / 60.0); // 不足1分钟按1分钟计
    return FIXED_LOCAL_RATE * minutes;
}