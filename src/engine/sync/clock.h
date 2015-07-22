#ifndef CLOCK_H
#define CLOCK_H

class Clock {
  public:
    virtual ~Clock() {}

    virtual double getBeatDistance() const = 0;
    virtual void setBeatDistance(double beatDistance) = 0;

    virtual double getBpm() const = 0;
    virtual void setBpm(double bpm) = 0;
};

#endif /* CLOCK_H */
