#pragma once

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/duration.h"
#include "util/memory.h"
#include "util/performancetimer.h"
#include "waveform/visualplayposition.h"

#include <vector>

// This calss updates the controls used for widgets and
// controler indicator, in a CPU saving way and outside the engine thread
class DeckVisuals {
  public:
    DeckVisuals(const QString& group);
    void process(double remainingTimeTriggerSeconds);

  private:
    QString m_group;
    PerformanceTimer m_cpuTimer;
    mixxx::Duration m_lastUpdateTime;
    int m_SlowTickCnt;

    std::unique_ptr<ControlObject> m_pTimeElapsed;
    std::unique_ptr<ControlObject> m_pTimeRemaining;
    std::unique_ptr<ControlObject> m_pEndOfTrack;
    std::unique_ptr<ControlObject> m_pVisualBpm;
    std::unique_ptr<ControlObject> m_pVisualKey;

    ControlProxy playButton;
    ControlProxy loopEnabled;
    ControlProxy engineBpm;
    ControlProxy engineKey;

    //std::unique_ptr<ControlObject> m_playposSlider;

    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
};

class VisualsManager {
  public:
    void addDeck(const QString& group) {
        m_deckVisuals.push_back(
                std::make_unique<DeckVisuals>(group));
    }

    void process(double remainingTimeTriggerSeconds) {
        for (const auto& d: m_deckVisuals) {
            d->process(remainingTimeTriggerSeconds);
        }
    }
  private:
    std::vector<std::unique_ptr<DeckVisuals> > m_deckVisuals;
};
