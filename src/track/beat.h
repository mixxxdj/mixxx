#pragma once

#include <QDebug>
#include <QSharedPointer>

#include "proto/beats.pb.h"

class Beat {
  public:
    Beat();
    explicit Beat(const mixxx::track::io::Beat& protoBeat);

    int getIndex() const {
        return m_iIndex;
    }

    int getFramePosition() const {
        return m_beat.frame_position();
    }

    int getSamplePosition() const;

    mixxx::track::io::Type getType() const {
        return m_beat.type();
    }

    mixxx::track::io::Source getSource() const {
        return m_beat.source();
    }

    // Return the underlying proto object.
    mixxx::track::io::Beat getProto() const {
        return m_beat;
    }

    void setIndex(int index) {
        m_iIndex = index;
    }

    void setFramePosition(int framePosition) {
        m_beat.set_frame_position(framePosition);
    }

    void setSamplePosition(int samplePosition);

    void setType(const mixxx::track::io::Type& type) {
        m_beat.set_type(type);
    }

    void setSource(const mixxx::track::io::Source& source) {
        m_beat.set_source(source);
    }

  private:
    int m_iIndex;
    mixxx::track::io::Beat m_beat;
};

bool operator<(const Beat& beat1, const Beat& beat2);
