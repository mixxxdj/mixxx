#ifndef EVENT_H
#define EVENT_H

#include <QtGlobal>
#include <QString>

#include "util/stat.h"

class Event {
  public:
    Event()
            : m_type(Stat::UNSPECIFIED),
              m_time(-1) {
    }

    typedef Stat::StatType EventType;

    QString m_tag;
    EventType m_type;
    qint64 m_time;

    static bool event(const QString& tag, Event::EventType type = Stat::EVENT) {
        return Stat::track(tag, type, Stat::experimentFlags(Stat::COUNT), 0.0);
    }

    static bool start(const QString& tag) {
        return event(tag, Stat::EVENT_START);
    }
    static bool end(const QString& tag) {
        return event(tag, Stat::EVENT_END);
    }
};

#endif /* EVENT_H */
