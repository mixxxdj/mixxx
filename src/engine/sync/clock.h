#pragma once
#include "track/bpm.h"

class Clock {
  public:
    virtual ~Clock() = default;

    virtual double getBeatDistance() const = 0;
    virtual void updateMasterBeatDistance(double beatDistance) = 0;

    virtual mixxx::Bpm getBpm() const = 0;
    virtual void updateMasterBpm(mixxx::Bpm bpm) = 0;
};
