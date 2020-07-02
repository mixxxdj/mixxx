#ifndef CLOCK_H
#define CLOCK_H

class Clock {
  public:
    virtual ~Clock() = default;

    virtual double getBeatDistance() const = 0;
    virtual void setMasterBeatDistance(double beatDistance) = 0;

    virtual double getBpm() const = 0;
    virtual void setMasterBpm(double bpm) = 0;
};

#endif /* CLOCK_H */
