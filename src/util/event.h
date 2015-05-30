#ifndef EVENT_H
#define EVENT_H

#include <QtGlobal>
#include <QString>

#include "util/stat.h"
#include "util/duration.h"

class Event {
  public:
    Event()
            : m_type(Stat::UNSPECIFIED) {
    }

    typedef Stat::StatType EventType;

    QString m_tag;
    EventType m_type;
    mixxx::Duration m_time;

    static bool event(const QString& tag, Event::EventType type = Stat::EVENT) {
        return Stat::track(tag, type, Stat::experimentFlags(Stat::COUNT), 0.0);
    }

    static bool start(const QString& tag) {
        return event(tag, Stat::EVENT_START);
    }

    static bool end(const QString& tag) {
        return event(tag, Stat::EVENT_END);
    }

  private:
    static bool event(const char*, Event::EventType) {
        // this should not be uses to avoid unicode encoding and
        // mamory alloc at every call. Use:
        // static const QString tag(const char*);
        return false;
    }

    static bool start(const char*) {
        // this should not be uses to avoid unicode encoding and
        // mamory alloc at every call. Use:
        // static const QString tag(const char*);
        return false;
    }

    static bool end(const char*) {
        // this should not be uses to avoid unicode encoding and
        // mamory alloc at every call. Use:
        // static const QString tag(const char*);
        return false;
    }


};

#endif /* EVENT_H */
