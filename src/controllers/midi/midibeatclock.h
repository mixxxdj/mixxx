#include <QString>

#include "control/controlobject.h"
#include "controllers/midi/midibeatclockreceiver.h"
#include "engine/sync/controllersyncable.h"

namespace mixxx {

class MidiBeatClock : public ControllerSyncable, public MidiBeatClockReceiver {
  public:
    MidiBeatClock(const QString& group);

    void receive(unsigned char status, Duration timestamp);

    /// Notify a Syncable that it is now the only currently-playing syncable.
    void notifyOnlyPlayingSyncable() override{};

    /// Notify a Syncable that they should sync phase.
    void requestSync() override{};

    /// Only relevant for player Syncables.
    bool isPlaying() const override {
        return MidiBeatClockReceiver::isPlaying();
    };

    /// Gets the current speed of the syncable in bpm (bpm * rate slider), doesn't
    /// include scratch or FF/REW values.
    double getBpm() const override {
        return bpm().getValue();
    };

    /// Gets the speed of the syncable if it was playing at 1.0 rate.
    double getBaseBpm() const override {
        return bpm().getValue();
    };

    /// Gets the beat distance as a fraction from 0 to 1
    double getBeatDistance() const override {
        return beatDistance();
    };

    /// The following functions are used to tell syncables about the state of the
    /// current Sync Master.
    /// Must never result in a call to
    /// SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    void setMasterBeatDistance(double beatDistance) override;

    /// Must never result in a call to SyncableListener::notifyBpmChanged or
    /// signal loops could occur.
    void setMasterBpm(double bpm) override;

    /// Combines the above three calls into one, since they are often set
    /// simultaneously.  Avoids redundant recalculation that would occur by
    /// using the three calls separately.
    void setMasterParams(double beatDistance, double baseBpm, double bpm) override;

    /// Must never result in a call to
    /// SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    /// occur.
    void setInstantaneousBpm(double bpm) override;

  private slots:
    void slotSyncLeaderEnabledChangeRequest(double value);

  private:
    QScopedPointer<ControlObject> m_pClockBpm;
    QScopedPointer<ControlObject> m_pClockBeatDistance;
};

} // namespace mixxx
