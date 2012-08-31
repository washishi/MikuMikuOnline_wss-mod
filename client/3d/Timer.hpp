#pragma once

#include <memory>

class Timer
{
public:
    Timer();
    Timer& Next();
    int Delta() const;
    double DeltaSec() const;
    int prev_time() const;
    int current_time() const;

private:
    int prev_time_, current_time_;
};

typedef std::shared_ptr<Timer> TimerPtr;
