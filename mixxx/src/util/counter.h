#ifndef COUNTER_H
#define COUNTER_H

#include "util/stat.h"

class Counter {
  public:
    Counter(const QString& tag)
    : m_tag(tag) {
    }
    void increment(int by=1) {
        Stat::track(m_tag, Stat::COUNTER,
                    Stat::COUNT | Stat::SUM | Stat::AVERAGE | Stat::SAMPLE_VARIANCE | Stat::MIN | Stat::MAX,
                    by);
    }
  private:
    QString m_tag;
};

#endif /* COUNTER_H */
