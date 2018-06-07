#pragma once

#include <QObject>
#include "track/trackplaytimers.h"
#include "gmock/gmock.h"

class TimerMock : public TrackTimers::RegularTimer {
    Q_OBJECT
  public:
    ~TimerMock() = default;
    MOCK_METHOD1(start,void(int msec));
    MOCK_CONST_METHOD0(isActive,bool());
    MOCK_METHOD0(stop,void());
};