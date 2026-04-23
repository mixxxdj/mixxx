#pragma once

#include <QChar>
#include <QMutex>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QString>
#include <array>
#include <atomic>
#include <tuple>

#include "lo/lo.h"
#include "preferences/usersettings.h"

extern QMutex s_configMutex;
extern QReadWriteLock g_oscTrackTableLock;
extern std::atomic<bool> s_oscEnabled;
extern int s_ckOscPortOutInt;
extern QList<std::pair<bool, QString>> s_receiverConfigs;
extern std::atomic<bool> s_configLoaded1stTimeFromFile;

constexpr int kMaxOscTracks = 70;
extern std::array<std::tuple<QString, QString, QString>, kMaxOscTracks> g_oscTrackTable;
extern QMutex g_oscTrackTableMutex;

extern bool s_oscSendSyncTriggers;
extern int s_oscSendSyncTriggersInterval;
extern int s_lastCheckStamp;
extern std::atomic<qint64> s_lastTriggerTime;

const bool sDebugOSCFunctions = false;

enum class DefOscBodyType {
    STRINGBODY,
    INTBODY,
    DOUBLEBODY,
    FLOATBODY
};

class OscFunctions {
  public:
    explicit OscFunctions(UserSettingsPointer pConfig);

    QString escapeStringToJsonUnicode(const QString& input);
    void oscFunctionsSendPtrType(
            const QString& oscGroup,
            const QString& oscKey,
            enum DefOscBodyType oscBodyType,
            const QString& oscMessageBodyQString,
            int oscMessageBodyInt,
            double oscMessageBodyDouble,
            float oscMessageBodyFloat);
    void reloadOscConfiguration();
    void storeTrackInfo(const QString& oscGroup,
            const QString& trackArtist,
            const QString& trackTitle);
    QString getTrackInfo(const QString& oscGroup, const QString& oscKey);
    void sendNoTrackLoadedToOscClients(const QString& oscGroup);
    void sendTrackInfoToOscClients(
            const QString& oscGroup,
            const QString& trackArtist,
            const QString& trackTitle,
            const QString& trackLocation,
            const QString& trackAlbum,
            float trackBPM,
            float track_loaded,
            float duration,
            float playposition);
    void oscChangedPlayState(
            const QString& oscGroup,
            float playstate);
    QString translatePath(const QString& inputPath);

  private:
    UserSettingsPointer m_pConfig;
};
