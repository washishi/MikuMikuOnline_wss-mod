#include "Timer.hpp"
#include <DxLib.h>


Timer::Timer()
    : prev_time_(0), current_time_(GetNowCount())
{}

Timer& Timer::Next()
{
    prev_time_ = current_time_;
    current_time_ = GetNowCount();
    return *this;
}

int Timer::Delta() const
{
    return current_time_ - prev_time_;
}

double Timer::DeltaSec() const
{
    return static_cast<double>(Delta()) / 1000.0;
}

int Timer::prev_time() const
{
    return prev_time_;
}

int Timer::current_time() const
{
    return current_time_;
}
