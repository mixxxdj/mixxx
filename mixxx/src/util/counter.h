#ifndef COUNTER_H
#define COUNTER_H

#include "util/stat.h"

class Counter {
    Counter(const QString& tag)
    : m_tag(tag) {
    }
    void increment() {
        Stat::track(m_tag, Stat::UNSPECIFIED, Stat::COUNT, 1);
    }
  private:
    QString m_tag;
};

#endif /* COUNTER_H */
