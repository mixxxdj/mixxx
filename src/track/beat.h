#pragma once

#include <QDebug>
#include <QSharedPointer>

#include "proto/beats.pb.h"

class Beat : public mixxx::track::io::Beat {
  public:
    Beat() = default;
    Beat(const mixxx::track::io::Beat& protoBeat) {
        init(protoBeat);
    }

    Beat(const QSharedPointer<Beat>& pBeat) {
        set_enabled(pBeat->enabled());
        set_frame_position(pBeat->frame_position());
        set_source(pBeat->source());
        set_type(pBeat->type());
    }

    Beat& operator=(const mixxx::track::io::Beat& protoBeat) {
        init(protoBeat);
        return *this;
    }

    void init(const mixxx::track::io::Beat& protoBeat);
    float sample_position();

    void setIndex(int index) {
        m_iIndex = index;
    }

    int getIndex() {
        return m_iIndex;
    }

    bool operator<(const Beat& compBeat) const {
        return frame_position() < compBeat.frame_position();
    }

  private:
    int m_iIndex;
};

using BeatPointer = QSharedPointer<Beat>;
