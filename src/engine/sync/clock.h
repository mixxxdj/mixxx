#pragma once

class Clock {
  public:
    virtual ~Clock() = default;

    virtual double getBeatDistance() const = 0;
    virtual void updateMasterBeatDistance(double beatDistance) = 0;

    virtual double getBpm() const = 0;
    virtual void updateMasterBpm(double bpm) = 0;
};
