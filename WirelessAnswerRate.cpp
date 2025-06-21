#include "WirelessAnswerRate.h"

const double WIRELESS_ANSWER_RATE = 0.50; // 元/分钟

double WirelessAnswerRate::calculateCharge(int duration, const QString& plan) const
{
    Q_UNUSED(plan);
    double minutes = std::ceil(duration / 60.0);
    return WIRELESS_ANSWER_RATE * minutes;
}
