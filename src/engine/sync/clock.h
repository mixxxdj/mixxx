#pragma once

class Clock {
  public:
    virtual ~Clock() = default;

    virtual double getBeatDistance() const = 0;
    virtual void updateLeaderBeatDistance(double beatDistance) = 0;

    virtual double getBpm() const = 0;
    virtual void updateLeaderBpm(double bpm) = 0;
};
