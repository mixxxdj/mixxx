#pragma once

#include <QPainter>

#include "preferences/beatgridmode.h"
#include "proto/beats.pb.h"
#include "track/beat.h"
#include "waveform/renderers/waveformelementrightclickable.h"

class WaveformBeat final : WaveformElementRightClickable {
  public:
    WaveformBeat();
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
    void setBeatGridMode(BeatGridMode mode) {
        m_beatGridMode = mode;
    }
    void setBeat(const mixxx::Beat& beat) {
        m_beat = beat;
    }
    mixxx::Beat getBeat() const {
        return m_beat;
    }
    void setVisible(bool bVisible) {
        m_bVisible = bVisible;
    }
    void setAlpha(int alpha) {
        m_iAlpha = alpha;
    }
    bool contains(QPoint point, Qt::Orientation orientation) const override;

  private:
    mixxx::Beat m_beat;
    Qt::Orientation m_orientation;
    int m_position;
    int m_length;
    int m_iAlpha;
    BeatGridMode m_beatGridMode;
    bool m_bVisible;
};
