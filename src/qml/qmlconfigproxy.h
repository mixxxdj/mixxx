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

#define PROPERTY_DECL_ACCESSOR(TYPE, NAME)                                  \
  public:                                                                   \
    TYPE NAME() const;                                                      \
    void set_##NAME(                                                        \
            std::conditional<(sizeof(TYPE) <= 16), TYPE, const TYPE&>::type \
                    value);

namespace mixxx {
namespace qml {

class QmlConfigProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Config)
    QML_SINGLETON

    // Preference settings
    Q_PROPERTY(mixxx::preferences::MultiSamplingMode multiSamplingLevel READ
                    multiSamplingLevel WRITE set_multiSamplingLevel NOTIFY
                            multiSamplingLevelChanged);
    Q_PROPERTY(bool useAcceleration READ useAcceleration WRITE
                    set_useAcceleration NOTIFY useAccelerationChanged);

    // Library group
    Q_PROPERTY(mixxx::preferences::Tooltips libraryTooltips READ libraryTooltips
                    WRITE set_libraryTooltips NOTIFY libraryTooltipsChanged);
    Q_PROPERTY(mixxx::preferences::ScreenSaver libraryInhibitScreensaver READ
                    libraryInhibitScreensaver WRITE
                            set_libraryInhibitScreensaver NOTIFY
                                    libraryInhibitScreensaverChanged);
    Q_PROPERTY(bool libraryHideMenuBar READ libraryHideMenuBar WRITE
                    set_libraryHideMenuBar NOTIFY libraryHideMenuBarChanged);
    Q_PROPERTY(bool libraryEnableSearchCompletions READ
                    libraryEnableSearchCompletions WRITE
                            set_libraryEnableSearchCompletions NOTIFY
                                    libraryEnableSearchCompletionsChanged);
    Q_PROPERTY(bool libraryEnableSearchHistoryShortcuts READ
                    libraryEnableSearchHistoryShortcuts WRITE
                            set_libraryEnableSearchHistoryShortcuts NOTIFY
                                    libraryEnableSearchHistoryShortcutsChanged);
    // Decimal count, 0..10
    Q_PROPERTY(int libraryBpmColumnPrecision READ libraryBpmColumnPrecision
                    WRITE set_libraryBpmColumnPrecision NOTIFY
                            libraryBpmColumnPrecisionChanged);
    // Pixels
    Q_PROPERTY(double libraryRowHeight READ libraryRowHeight WRITE
                    set_libraryRowHeight NOTIFY libraryRowHeightChanged);
    // Controls group
    // [0..activePaletteColorCount-1]
    Q_PROPERTY(int controlHotcueDefaultColorIndex READ
                    controlHotcueDefaultColorIndex WRITE
                            set_controlHotcueDefaultColorIndex NOTIFY
                                    controlHotcueDefaultColorIndexChanged);
    // [0..activePaletteColorCount-1]
    Q_PROPERTY(double controlLoopDefaultColorIndex READ
                    controlLoopDefaultColorIndex WRITE
                            set_controlLoopDefaultColorIndex NOTIFY
                                    controlLoopDefaultColorIndexChanged);
    Q_PROPERTY(CueMode controlCueDefault READ controlCueDefault WRITE
                    set_controlCueDefault NOTIFY controlCueDefaultChanged);
    Q_PROPERTY(bool controlSetIntroStartAtMainCue READ
                    controlSetIntroStartAtMainCue WRITE
                            set_controlSetIntroStartAtMainCue NOTIFY
                                    controlSetIntroStartAtMainCueChanged);
    Q_PROPERTY(bool controlCloneDeckOnLoadDoubleTap READ
                    controlCloneDeckOnLoadDoubleTap WRITE
                            set_controlCloneDeckOnLoadDoubleTap NOTIFY
                                    controlCloneDeckOnLoadDoubleTapChanged);
    Q_PROPERTY(LoadWhenDeckPlaying controlLoadWhenDeckPlaying READ
                    controlLoadWhenDeckPlaying WRITE
                            set_controlLoadWhenDeckPlaying NOTIFY
                                    controlLoadWhenDeckPlayingChanged);
    Q_PROPERTY(TrackTime::DisplayFormat controlTimeFormat READ controlTimeFormat
                    WRITE set_controlTimeFormat NOTIFY
                            controlTimeFormatChanged);
    Q_PROPERTY(TrackTime::DisplayMode controlPositionDisplay READ
                    controlPositionDisplay WRITE set_controlPositionDisplay
                            NOTIFY controlPositionDisplayChanged);
    Q_PROPERTY(SeekOnLoadMode controlCueRecall READ controlCueRecall WRITE
                    set_controlCueRecall NOTIFY controlCueRecallChanged);
    Q_PROPERTY(BaseTrackPlayer::TrackLoadReset controlSpeedAutoReset READ
                    controlSpeedAutoReset WRITE set_controlSpeedAutoReset NOTIFY
                            controlSpeedAutoResetChanged);
    Q_PROPERTY(KeylockMode controlKeylockMode READ controlKeylockMode WRITE
                    set_controlKeylockMode NOTIFY controlKeylockModeChanged);
    Q_PROPERTY(KeyunlockMode controlKeyunlockMode READ controlKeyunlockMode
                    WRITE set_controlKeyunlockMode NOTIFY
                            controlKeyunlockModeChanged);
    // [100...2500]
    Q_PROPERTY(double controlRateRampSensitivity READ controlRateRampSensitivity
                    WRITE set_controlRateRampSensitivity NOTIFY
                            controlRateRampSensitivityChanged);
    // [0.01..10]
    Q_PROPERTY(double controlRateTempCoarse READ controlRateTempCoarse WRITE
                    set_controlRateTempCoarse NOTIFY
                            controlRateTempCoarseChanged);
    // [0.01..10]
    Q_PROPERTY(double controlRateTempFine READ controlRateTempFine WRITE
                    set_controlRateTempFine NOTIFY controlRateTempFineChanged);
    // [0.01..10]
    Q_PROPERTY(double controlRatePermCoarse READ controlRatePermCoarse WRITE
                    set_controlRatePermCoarse NOTIFY
                            controlRatePermCoarseChanged);
    // [0.01..10]
    Q_PROPERTY(double controlRatePermFine READ controlRatePermFine WRITE
                    set_controlRatePermFine NOTIFY controlRatePermFineChanged);
    // [4, 6, 8, 10, 16, 24, 50, 90, *]
    Q_PROPERTY(int controlRateRange READ controlRateRange WRITE
                    set_controlRateRange NOTIFY controlRateRangeChanged);
    // If true, down increases
    Q_PROPERTY(bool controlRateDir READ controlRateDir WRITE set_controlRateDir
                    NOTIFY controlRateDirChanged);
    Q_PROPERTY(RateControl::RampMode controlPitchBendBehaviour READ
                    controlPitchBendBehaviour WRITE
                            set_controlPitchBendBehaviour NOTIFY
                                    controlPitchBendBehaviourChanged);
    // Config group
    Q_PROPERTY(QString configHotcueColorPalette READ configHotcueColorPalette
                    WRITE set_configHotcueColorPalette NOTIFY
                            configHotcueColorPaletteChanged);
    Q_PROPERTY(QString configTrackColorPalette READ configTrackColorPalette
                    WRITE set_configTrackColorPalette NOTIFY
                            configTrackColorPaletteChanged);
    Q_PROPERTY(QString configKeyColorPalette READ configKeyColorPalette WRITE
                    set_configKeyColorPalette NOTIFY
                            configKeyColorPaletteChanged);
    Q_PROPERTY(bool configKeyColorsEnabled READ configKeyColorsEnabled WRITE
                    set_configKeyColorsEnabled NOTIFY
                            configKeyColorsEnabledChanged);
    Q_PROPERTY(bool configStartInFullscreenKey READ configStartInFullscreenKey
                    WRITE set_configStartInFullscreenKey NOTIFY
                            configStartInFullscreenKeyChanged);
    // BPM group
    Q_PROPERTY(EngineSync::SyncLockAlgorithm bpmSyncLockAlgorithm READ
                    bpmSyncLockAlgorithm WRITE set_bpmSyncLockAlgorithm NOTIFY
                            bpmSyncLockAlgorithmChanged);

    // Colors
    Q_PROPERTY(QVariantList hotcueColorPalette READ hotcueColorPalette NOTIFY
                    hotcueColorPaletteChanged);
    Q_PROPERTY(QVariantList trackColorPalette READ trackColorPalette NOTIFY
                    trackColorPaletteChanged);
    Q_PROPERTY(QVariantList keyColorPalette READ keyColorPalette NOTIFY
                    keyColorPaletteChanged); // We use method here instead of
                                             // properties as there is no way to
                                             // achieve property binding
  public:
    explicit QmlConfigProxy(
            UserSettingsPointer pConfig,
            QObject* parent = nullptr);

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

  public:
    // Preference settings
    PROPERTY_DECL_ACCESSOR(mixxx::preferences::MultiSamplingMode, multiSamplingLevel);
    PROPERTY_DECL_ACCESSOR(bool, useAcceleration);

    // Waveform settings
    Q_INVOKABLE bool waveformZoomSynchronization();
    Q_INVOKABLE double waveformDefaultZoom();

    // Library group
    PROPERTY_DECL_ACCESSOR(mixxx::preferences::Tooltips, libraryTooltips);
    PROPERTY_DECL_ACCESSOR(mixxx::preferences::ScreenSaver, libraryInhibitScreensaver);
    PROPERTY_DECL_ACCESSOR(bool, libraryHideMenuBar);
    PROPERTY_DECL_ACCESSOR(bool, libraryEnableSearchCompletions);
    PROPERTY_DECL_ACCESSOR(bool, libraryEnableSearchHistoryShortcuts);
    // Decimal count, 0..10
    PROPERTY_DECL_ACCESSOR(int, libraryBpmColumnPrecision);
    // Pixels
    PROPERTY_DECL_ACCESSOR(double, libraryRowHeight);

    // Controls group
    // [0..activePaletteColorCount-1]
    PROPERTY_DECL_ACCESSOR(int, controlHotcueDefaultColorIndex);
    // [0..activePaletteColorCount-1]
    PROPERTY_DECL_ACCESSOR(double, controlLoopDefaultColorIndex);
    PROPERTY_DECL_ACCESSOR(CueMode, controlCueDefault);
    PROPERTY_DECL_ACCESSOR(bool, controlSetIntroStartAtMainCue);
    PROPERTY_DECL_ACCESSOR(bool, controlCloneDeckOnLoadDoubleTap);
    PROPERTY_DECL_ACCESSOR(LoadWhenDeckPlaying, controlLoadWhenDeckPlaying);
    PROPERTY_DECL_ACCESSOR(TrackTime::DisplayFormat, controlTimeFormat);
    PROPERTY_DECL_ACCESSOR(TrackTime::DisplayMode, controlPositionDisplay);
    PROPERTY_DECL_ACCESSOR(SeekOnLoadMode, controlCueRecall);
    PROPERTY_DECL_ACCESSOR(BaseTrackPlayer::TrackLoadReset, controlSpeedAutoReset);
    PROPERTY_DECL_ACCESSOR(KeylockMode, controlKeylockMode);
    PROPERTY_DECL_ACCESSOR(KeyunlockMode, controlKeyunlockMode);
    // [100...2500]
    PROPERTY_DECL_ACCESSOR(double, controlRateRampSensitivity);
    // [0.01..10]
    PROPERTY_DECL_ACCESSOR(double, controlRateTempCoarse);
    // [0.01..10]
    PROPERTY_DECL_ACCESSOR(double, controlRateTempFine);
    // [0.01..10]
    PROPERTY_DECL_ACCESSOR(double, controlRatePermCoarse);
    // [0.01..10]
    PROPERTY_DECL_ACCESSOR(double, controlRatePermFine);
    // [4, 6, 8, 10, 16, 24, 50, 90, *]
    PROPERTY_DECL_ACCESSOR(int, controlRateRange);
    // If true, down increases
    PROPERTY_DECL_ACCESSOR(bool, controlRateDir);
    PROPERTY_DECL_ACCESSOR(RateControl::RampMode, controlPitchBendBehaviour);

    // Config group
    PROPERTY_DECL_ACCESSOR(QString, configHotcueColorPalette);
    PROPERTY_DECL_ACCESSOR(QString, configTrackColorPalette);
    PROPERTY_DECL_ACCESSOR(QString, configKeyColorPalette);
    PROPERTY_DECL_ACCESSOR(bool, configKeyColorsEnabled);
    PROPERTY_DECL_ACCESSOR(bool, configStartInFullscreenKey);

    // BPM group
    PROPERTY_DECL_ACCESSOR(EngineSync::SyncLockAlgorithm, bpmSyncLockAlgorithm);

    static QmlConfigProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static inline void registerUserSettings(UserSettingsPointer pConfig) {
        s_pUserSettings = std::move(pConfig);
    }

    static UserSettingsPointer get() {
        return s_pUserSettings;
    }

  signals:
    void multiSamplingLevelChanged();
    void useAccelerationChanged();
    void libraryTooltipsChanged();
    void libraryInhibitScreensaverChanged();
    void libraryHideMenuBarChanged();
    void libraryEnableSearchCompletionsChanged();
    void libraryEnableSearchHistoryShortcutsChanged();
    void libraryBpmColumnPrecisionChanged();
    void libraryRowHeightChanged();
    void controlHotcueDefaultColorIndexChanged();
    void controlLoopDefaultColorIndexChanged();
    void controlCueDefaultChanged();
    void controlSetIntroStartAtMainCueChanged();
    void controlCloneDeckOnLoadDoubleTapChanged();
    void controlLoadWhenDeckPlayingChanged();
    void controlTimeFormatChanged();
    void controlPositionDisplayChanged();
    void controlCueRecallChanged();
    void controlSpeedAutoResetChanged();
    void controlKeylockModeChanged();
    void controlKeyunlockModeChanged();
    void controlRateRampSensitivityChanged();
    void controlRateTempCoarseChanged();
    void controlRateTempFineChanged();
    void controlRatePermCoarseChanged();
    void controlRatePermFineChanged();
    void controlRateRangeChanged();
    void controlRateDirChanged();
    void controlPitchBendBehaviourChanged();
    void configHotcueColorPaletteChanged();
    void configTrackColorPaletteChanged();
    void configKeyColorPaletteChanged();
    void configKeyColorsEnabledChanged();
    void configStartInFullscreenKeyChanged();
    void bpmSyncLockAlgorithmChanged();

  private:
    static inline UserSettingsPointer s_pUserSettings = nullptr;

    const UserSettingsPointer m_pConfig;
};

} // namespace qml
} // namespace mixxx
