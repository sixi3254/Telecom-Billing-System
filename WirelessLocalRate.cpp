#include "WirelessLocalRate.h"

const double WIRELESS_LOCAL_RATE = 0.60; // 元/分钟

double WirelessLocalRate::calculateCharge(int duration, const QString& plan) const
{
    Q_UNUSED(plan);
    double minutes = std::ceil(duration / 60.0);
    return WIRELESS_LOCAL_RATE * minutes;
}
