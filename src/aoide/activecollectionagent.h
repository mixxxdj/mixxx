#pragma once

#include "util/qt.h"

class TrackCollectionManager;

namespace aoide {

class Subsystem;

// The agent is a bot that listens to collection changes and
// automatically activates one of the available collections.
// TODO: Ask the user for explicitly selecting an active
// collection if multiple collections are available.
class ActiveCollectionAgent : public QObject {
    Q_OBJECT

  public:
    ActiveCollectionAgent(
            Subsystem* subsystem,
            TrackCollectionManager* trackCollectionManager,
            QObject* parent = nullptr);
    ~ActiveCollectionAgent() override = default;

  private slots:
    void /*Subsystem*/ onCollectionsChanged(
            int flags);

  private:
    const mixxx::SafeQPointer<Subsystem> m_subsystem;
    const mixxx::SafeQPointer<TrackCollectionManager> m_trackCollectionManager;
};

} // namespace aoide
