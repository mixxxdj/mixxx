#pragma once

#include <proto/beats.pb.h>

#include <QPainter>

using BeatType = mixxx::track::io::Type;

enum Direction {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

class WaveformBeat {
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
    void setBeatGridMode(int mode) {
        m_beatGridMode = mode;
    }

  private:
    QPolygonF getEquilateralTriangle(float edgeLength,
            QPointF baseMidPoint,
            Direction pointingDirection) const;

    Qt::Orientation m_orientation;
    int m_position;
    int m_length;
    int m_beatGridMode;
    BeatType m_beatType;
};
