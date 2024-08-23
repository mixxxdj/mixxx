#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "control/controlproxy.h"
#include "engine/channels/enginechannel.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/class.h"
#include "util/duration.h"

class ControlPushButton;
class TrackCollectionManager;
class PlayerManagerInterface;
class BaseTrackPlayer;
class PlaylistTableModel;
typedef QList<QModelIndex> QModelIndexList;

class AutoFadeController : public QObject {
    Q_OBJECT
  public:
    enum AutoFaderState {
        Idle,
        FadingOut,
        FadingIn
    };

    AutoFadeController(BaseTrackPlayer* pPlayer);
    virtual ~AutoFadeController();

  private:
    void cancelAndResetFade();
    void startFade(AutoFaderState direction);
    void setState(AutoFaderState newState);

    double getVolumeFader();
    void setVolumeFader(double value);
    void togglePlay(bool play);

  private slots:
    void playerPositionChanged(double position);
    void playerPlayingChanged(bool playing);
    void playerVolumeChanged(double volume);

    void controlFadeNow(double value);
    void controlFadeOutNow(double value);
    void controlFadeInNow(double value);

  private:
    AutoFaderState m_state;
    double m_transitionProgress;
    double m_transitionTime;
    double m_fadeBeginPos;
    double m_fadeEndPos;
    double m_faderTarget;
    double m_faderOriginal;
    bool m_isVolumeChangedTriggeredByMe;
    bool m_isPlayingChangeTriggeredByMe;

    QString m_group;
    ControlProxy m_orientation;
    ControlProxy m_playPos;
    ControlProxy m_play;
    ControlProxy m_duration;
    ControlProxy m_volume;
    ControlProxy m_fadeTime;
    BaseTrackPlayer* m_pPlayer;

    ControlObject* m_pIsFading;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pFadeOutNow;
    ControlPushButton* m_pFadeInNow;

    DISALLOW_COPY_AND_ASSIGN(AutoFadeController);
};
