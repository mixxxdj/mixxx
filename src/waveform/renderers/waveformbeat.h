#pragma once

#include <QPainter>

#include "preferences/beatgridmode.h"
#include "proto/beats.pb.h"
#include "track/beat.h"
#include "waveform/renderers/waveformelementrightclickable.h"

enum class Direction : int {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

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
    void setBeat(mixxx::Beat beat) {
        m_beat = beat;
    }
    mixxx::Beat getBeat() const {
        return m_beat;
    }
    void setVisible(bool bVisible) {
        m_bVisible = bVisible;
    }
    bool contains(QPoint point, Qt::Orientation orientation) const override;

  private:
    Qt::Orientation m_orientation;
    int m_position;
    int m_length;
    BeatGridMode m_beatGridMode;
    mixxx::Beat m_beat;
    bool m_bVisible;
};

bool operator<(const WaveformBeat& beat1, const WaveformBeat& beat2);
