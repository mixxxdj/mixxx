#pragma once

#include <QObject>

#include "util/singleton.h"

class DateFormatChangedBroadcaster
        : public QObject,
          public Singleton<DateFormatChangedBroadcaster> {
    Q_OBJECT
  public:
  signals:
    void dateFormatChanged();

  protected:
    DateFormatChangedBroadcaster();
    ~DateFormatChangedBroadcaster() override = default;
    friend class Singleton<DateFormatChangedBroadcaster>;
};
