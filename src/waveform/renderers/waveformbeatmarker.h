#pragma once

#include <QPainter>

class WaveformBeatMarker final {
  public:
    WaveformBeatMarker();
    void draw(QPainter* painter) const;
    void setPositionPixels(int position) {
        m_position = position;
    }
    void setOrientation(Qt::Orientation orientation) {
        m_orientation = orientation;
    }
    void setLength(int length) {
        m_length = length;
    }

    void setTextDisplayItems(const QStringList& items) {
        m_textDisplayItems = items;
    }

  private:
    Qt::Orientation m_orientation;
    int m_position;
    int m_length;
    QStringList m_textDisplayItems;
};
