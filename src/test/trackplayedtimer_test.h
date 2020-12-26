#pragma once

#include <QObject>

#include "gmock/gmock.h"
#include "track/trackplaytimers.h"

class TimerMock : public TrackTimers::RegularTimer {
    Q_OBJECT
  public:
    ~TimerMock() = default;
    MOCK_METHOD1(start, void(double));
    MOCK_CONST_METHOD0(isActive, bool());
    MOCK_METHOD0(stop, void());
};
