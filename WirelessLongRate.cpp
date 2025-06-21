#include "WirelessLongRate.h"

const double WIRELESS_LONG_RATE = 1.00; // 元/分钟

double WirelessLongRate::calculateCharge(int duration, const QString& plan) const
{
    Q_UNUSED(plan);
    double minutes = std::ceil(duration / 60.0);
    return WIRELESS_LONG_RATE * minutes;
}
