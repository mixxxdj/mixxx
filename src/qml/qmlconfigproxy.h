#pragma once
#include <QColor>
#include <QObject>
#include <QQmlEngine>
#include <QVariantList>

#include "engine/controls/cuecontrol.h"
#include "engine/controls/ratecontrol.h"
#include "engine/sync/enginesync.h"
#include "mixer/basetrackplayer.h"
#include "preferences/constants.h"
#include "preferences/interface.h"
#include "preferences/usersettings.h"
#include "qml/qmlwaveformdisplay.h"

#define PROPERTY_DECL(TYPE, NAME)                                          \
  public:                                                                  \
    Q_PROPERTY(TYPE NAME READ NAME WRITE set_##NAME NOTIFY NAME##Changed); \
    TYPE NAME() const;                                                     \
    void set_##NAME(TYPE value);                                           \
    Q_SIGNAL void NAME##Changed();

namespace mixxx {
namespace qml {

class QmlConfigProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Config)
    QML_SINGLETON
  public:
    explicit QmlConfigProxy(
            UserSettingsPointer pConfig,
            QObject* parent = nullptr);

    // Colors
    Q_PROPERTY(QVariantList hotcueColorPalette READ hotcueColorPalette NOTIFY
                    hotcueColorPaletteChanged);
    Q_PROPERTY(QVariantList trackColorPalette READ trackColorPalette NOTIFY
                    trackColorPaletteChanged);
    Q_PROPERTY(QVariantList keyColorPalette READ keyColorPalette NOTIFY
                    keyColorPaletteChanged); // We use method here instead of
                                             // properties as there is no way to
                                             // achieve property binding
    // with UserSettings, since there is no synchronisation upon mutations.
    QVariantList hotcueColorPalette() const;
    Q_INVOKABLE QVariantList getHotcueColorPalette(const QString& paletteName) const;
    Q_INVOKABLE void setHotcueColorPalette(const QString& paletteName);
    QVariantList trackColorPalette() const;
    Q_INVOKABLE QVariantList getTrackColorPalette(const QString& paletteName) const;
    Q_INVOKABLE void setTrackColorPalette(const QString& paletteName);
    QVariantList keyColorPalette() const;
    Q_INVOKABLE QVariantList getKeyColorPalette(const QString& paletteName) const;
    Q_INVOKABLE void setKeyColorPalette(const QString& paletteName);
    Q_INVOKABLE QVariantList colorPalette(const QString& paletteName) const;

    Q_INVOKABLE QStringList paletteNames() const;
  signals:
    void hotcueColorPaletteChanged();
    void trackColorPaletteChanged();
    void keyColorPaletteChanged();

    // Preference settings
    PROPERTY_DECL(mixxx::preferences::MultiSamplingMode, multiSamplingLevel);
    PROPERTY_DECL(bool, useAcceleration);

    // Waveform settings
    PROPERTY_DECL(bool, waveformZoomSynchronization);
    PROPERTY_DECL(bool, waveformOverviewNormalized);
    // 1..10
    PROPERTY_DECL(double, waveformDefaultZoom);
    // [0..1]
    PROPERTY_DECL(double, waveformPlayMarkerPosition);
    PROPERTY_DECL(bool, waveformUntilMarkShowBeats);
    PROPERTY_DECL(bool, waveformUntilMarkShowTime);
    // {1,2,3}, Qt::AlignTop, Qt::AlignVCenter, Qt::AlignBottom
    PROPERTY_DECL(double, waveformUntilMarkAlign);
    PROPERTY_DECL(int, waveformUntilMarkTextPointSize);
    // [0..1..]
    PROPERTY_DECL(double, waveformVisualGainAll);
    // [0..1..]
    PROPERTY_DECL(double, waveformVisualGainLow);
    // [0..1..]
    PROPERTY_DECL(double, waveformVisualGainMedium);
    // [0..1..]
    PROPERTY_DECL(double, waveformVisualGainHigh);
    // Seconds
    PROPERTY_DECL(int, waveformEndOfTrackWarningTime);
    PROPERTY_DECL(QmlWaveformDisplay::Type, waveformType);
    PROPERTY_DECL(QmlWaveformDisplay::Options, waveformOptions);
    // Percent, 0..100
    PROPERTY_DECL(double, waveformBeatGridAlpha);

    // Library group
    PROPERTY_DECL(mixxx::preferences::Tooltips, libraryTooltips);
    PROPERTY_DECL(mixxx::preferences::ScreenSaver, libraryInhibitScreensaver);
    PROPERTY_DECL(bool, libraryHideMenuBar);
    PROPERTY_DECL(bool, libraryEnableSearchCompletions);
    PROPERTY_DECL(bool, libraryEnableSearchHistoryShortcuts);
    // Decimal count, 0..10
    PROPERTY_DECL(int, libraryBpmColumnPrecision);
    // Pixels
    PROPERTY_DECL(double, libraryRowHeight);

    // Controls group
    // [0..activePaletteColorCount-1]
    PROPERTY_DECL(int, controlHotcueDefaultColorIndex);
    // [0..activePaletteColorCount-1]
    PROPERTY_DECL(double, controlLoopDefaultColorIndex);
    PROPERTY_DECL(CueMode, controlCueDefault);
    PROPERTY_DECL(bool, controlSetIntroStartAtMainCue);
    PROPERTY_DECL(bool, controlCloneDeckOnLoadDoubleTap);
    PROPERTY_DECL(LoadWhenDeckPlaying, controlLoadWhenDeckPlaying);
    PROPERTY_DECL(TrackTime::DisplayFormat, controlTimeFormat);
    PROPERTY_DECL(TrackTime::DisplayMode, controlPositionDisplay);
    PROPERTY_DECL(SeekOnLoadMode, controlCueRecall);
    PROPERTY_DECL(BaseTrackPlayer::TrackLoadReset, controlSpeedAutoReset);
    PROPERTY_DECL(KeylockMode, controlKeylockMode);
    PROPERTY_DECL(KeyunlockMode, controlKeyunlockMode);
    // [100...2500]
    PROPERTY_DECL(double, controlRateRampSensitivity);
    // [0.01..10]
    PROPERTY_DECL(double, controlRateTempCoarse);
    // [0.01..10]
    PROPERTY_DECL(double, controlRateTempFine);
    // [0.01..10]
    PROPERTY_DECL(double, controlRatePermCoarse);
    // [0.01..10]
    PROPERTY_DECL(double, controlRatePermFine);
    // [4, 6, 8, 10, 16, 24, 50, 90, *]
    PROPERTY_DECL(int, controlRateRange);
    // If true, down increases
    PROPERTY_DECL(bool, controlRateDir);
    PROPERTY_DECL(RateControl::RampMode, controlPitchBendBehaviour);

    // Config group
    PROPERTY_DECL(QString, configHotcueColorPalette);
    PROPERTY_DECL(QString, configTrackColorPalette);
    PROPERTY_DECL(QString, configKeyColorPalette);
    PROPERTY_DECL(bool, configKeyColorsEnabled);
    PROPERTY_DECL(bool, configStartInFullscreenKey);

    // BPM group
    PROPERTY_DECL(EngineSync::SyncLockAlgorithm, bpmSyncLockAlgorithm);

    static QmlConfigProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static inline void registerUserSettings(UserSettingsPointer pConfig) {
        s_pUserSettings = std::move(pConfig);
    }

    static UserSettingsPointer get() {
        return s_pUserSettings;
    }

  private:
    static inline UserSettingsPointer s_pUserSettings = nullptr;

    const UserSettingsPointer m_pConfig;
};

} // namespace qml
} // namespace mixxx
