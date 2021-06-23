#pragma once

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

    // Disallow to use this class with implicit converted char strings.
    // This should not be uses to avoid unicode encoding and memory
    // allocation at every call. Use a static tag like this:
    // static const QString tag("TAG TEXT");
    static bool event(const char*, Event::EventType) = delete;
    static bool start(const char*) = delete;
    static bool end(const char*) = delete;
};
