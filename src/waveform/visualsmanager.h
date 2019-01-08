#pragma once

#include <vector>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/duration.h"
#include "util/memory.h"
#include "util/performancetimer.h"
#include "waveform/visualplayposition.h"

namespace {

// Rate at which the playpos slider is updated
const int kUpdateRate = 15; // updates per second
// Number of kiUpdateRates that go by before we update BPM.
const int kSlowUpdateDivider = 4; // kUpdateRate / kSlowUpdateDivider = 3.75 updates per sec

} // anonymous namespace

// This class updates the controls used for widgets and
// controler indicator, in a CPU saving way and outside the engine thread
class DeckVisuals {
  public:
    DeckVisuals(const QString& group);
    void process(double remainingTimeTriggerSeconds);

  private:
    QString m_group;
    int m_SlowTickCnt;
    bool m_trackLoaded;

    std::unique_ptr<ControlObject> m_pTimeElapsed;
    std::unique_ptr<ControlObject> m_pTimeRemaining;
    std::unique_ptr<ControlObject> m_pEndOfTrack;
    std::unique_ptr<ControlObject> m_pVisualBpm;
    std::unique_ptr<ControlObject> m_pVisualKey;

    ControlProxy playButton;
    ControlProxy loopEnabled;
    ControlProxy engineBpm;
    ControlProxy engineKey;

    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
};

class VisualsManager {
  public:
    VisualsManager() {
        m_cpuTimer.start();
    }

    void addDeck(const QString& group) {
        m_deckVisuals.push_back(
                std::make_unique<DeckVisuals>(group));
    }

    void process(double remainingTimeTriggerSeconds) {
        if (m_cpuTimer.elapsed() >= mixxx::Duration::fromMillis(1000 / kUpdateRate)) {
            m_cpuTimer.restart();
            for (const auto& d: m_deckVisuals) {
                d->process(remainingTimeTriggerSeconds);
            }
        }
    }
  private:
    std::vector<std::unique_ptr<DeckVisuals> > m_deckVisuals;
    PerformanceTimer m_cpuTimer;
};
