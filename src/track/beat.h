#pragma once

#include <QSharedPointer>

#include "proto/beats.pb.h"

class Beat : public mixxx::track::io::Beat {
  public:
    Beat() = default;
    Beat(const mixxx::track::io::Beat& protoBeat) {
        init(protoBeat);
    }

    Beat& operator=(const mixxx::track::io::Beat& protoBeat) {
        init(protoBeat);
        return *this;
    }

    void init(const mixxx::track::io::Beat& protoBeat);

    bool operator<(const Beat& compBeat) const {
        return frame_position() < compBeat.frame_position();
    }
};

using BeatPointer = QSharedPointer<Beat>;
