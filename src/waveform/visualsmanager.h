#pragma once

#include <memory>
#include <vector>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/duration.h"
#include "util/performancetimer.h"

class VisualPlayPosition;

namespace {

// Rate at which the playpos slider is updated
constexpr int kUpdateRate = 15; // updates per second
// Number of kiUpdateRates that go by before we update BPM.
constexpr int kSlowUpdateDivider = 4; // kUpdateRate / kSlowUpdateDivider = 3.75 updates per sec

} // anonymous namespace

// This class updates the controls used for widgets and
// controller indicator, in a CPU saving way and outside the engine thread
class DeckVisuals {
  public:
    DeckVisuals(const QString& group);
    void process(double remainingTimeTriggerSeconds);

    const QString& getGroup() const {
        return m_group;
    }

  private:
    const QString m_group;
    int m_SlowTickCnt;
    bool m_trackLoaded;

    std::unique_ptr<ControlProxy> m_pPlayButton;
    std::unique_ptr<ControlProxy> m_pLoopEnabled;

    std::unique_ptr<ControlProxy> m_pEngineBpm;
    std::unique_ptr<ControlProxy> m_pVisualBpm;

    std::unique_ptr<ControlProxy> m_pEngineKey;
    std::unique_ptr<ControlProxy> m_pVisualKey;

    std::unique_ptr<ControlProxy> m_pTimeElapsed;
    std::unique_ptr<ControlProxy> m_pTimeRemaining;
    std::unique_ptr<ControlProxy> m_pEndOfTrack;

    QSharedPointer<VisualPlayPosition> m_pVisualPlayPos;
};

class VisualsManager {
  public:
    VisualsManager() {
        m_cpuTimer.start();
    }

    void addDeck(const QString& group) {
        VERIFY_OR_DEBUG_ASSERT(!group.trimmed().isEmpty()) {
            return;
        }
        m_deckVisuals.push_back(
                std::make_unique<DeckVisuals>(group));
    }

    void addDeckIfNotExist(const QString& group) {
        for (auto& pDeckVisuals : m_deckVisuals) {
            if (pDeckVisuals->getGroup() == group) {
                return;
            }
        }
        addDeck(group);
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
    std::vector<std::unique_ptr<DeckVisuals>> m_deckVisuals;
    PerformanceTimer m_cpuTimer;
};
