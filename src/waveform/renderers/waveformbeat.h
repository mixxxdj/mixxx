#pragma once

#include <QPainter>

#include "preferences/beatgridmode.h"
#include "proto/beats.pb.h"

using BeatType = mixxx::track::io::Type;

enum class Direction : int {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

class WaveformBeat final {
  public:
    WaveformBeat();
    void draw(QPainter* painter) const;
    void setPosition(int position) {
        m_position = position;
    }
    void setType(BeatType beatType) {
        m_beatType = beatType;
    }
    void setOrientation(Qt::Orientation orientation) {
        m_orientation = orientation;
    }
    void setLength(int length) {
        m_length = length;
    }
    void setBeatGridMode(BeatGridMode mode) {
        m_beatGridMode = mode;
    }

  private:
    QPolygonF getEquilateralTriangle(float edgeLength,
            QPointF baseMidPoint,
            Direction pointingDirection) const;

    Qt::Orientation m_orientation;
    int m_position;
    int m_length;
    BeatGridMode m_beatGridMode;
    BeatType m_beatType;
};
