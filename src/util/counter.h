#ifndef COUNTER_H
#define COUNTER_H

#include "util/stat.h"

class Counter {
  public:
    Counter(const QString& tag)
    : m_tag(tag) {
    }
    void increment(int by=1) {
        Stat::ComputeFlags flags = Stat::experimentFlags(
            Stat::COUNT | Stat::SUM | Stat::AVERAGE |
            Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX);
        Stat::track(m_tag, Stat::COUNTER, flags, by);
    }
    Counter& operator+=(int by) {
        this->increment(by);
        return *this;
    }
    inline Counter operator++(int) { // postfix
        Counter result = *this;
        increment(1);
        return result;
    }
  private:
    QString m_tag;
};

#endif /* COUNTER_H */
