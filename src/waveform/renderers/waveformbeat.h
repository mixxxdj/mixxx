#pragma once

#include <QPainter>

class WaveformBeat {
  public:
    WaveformBeat();
    void draw(QPainter* painter);
    void setPosition(int position) {
        m_position = position;
    }
    void setAsDownbeat() {
        m_bDownbeat = true;
    }
    void setOrientation(Qt::Orientation orientation) {
        m_orientation = orientation;
    }
    void setLength(int length) {
        m_length = length;
    }

  private:
    Qt::Orientation m_orientation;
    int m_position;
    bool m_bDownbeat;
    int m_length;
};
