#include "engine/faderstartcontrol.h"

FaderStartControl::FaderStartControl(const QString& group)
        : m_play(group, "play"),
          m_volume(group, "volume") {
}

void FaderStartControl::process() {
    double value = m_volume.get();

    if (value > 0.01) {
        m_play.set(1.0);
    } else {
        m_play.set(0.0);
    }
}
