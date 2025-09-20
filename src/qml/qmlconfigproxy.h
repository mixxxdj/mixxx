#pragma once
#include <QColor>
#include <QObject>
#include <QQmlEngine>
#include <QVariantList>

#include "preferences/usersettings.h"

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

    // We use method here instead of properties as there is no way to achieve property binding
    // with UserSettings, since there is no synchronisation upon mutations.
    Q_INVOKABLE QVariantList getHotcueColorPalette();
    Q_INVOKABLE QVariantList getTrackColorPalette();
    Q_INVOKABLE int getMultiSamplingLevel();
    Q_INVOKABLE bool useAcceleration();

    // Waveform settings
    Q_INVOKABLE bool waveformZoomSynchronization();
    Q_INVOKABLE double waveformDefaultZoom();

    // Library group
    PROPERTY_DECL(bool, librarySyncTrackMetadataExport);
    PROPERTY_DECL(bool, librarySeratoMetadataExport);
    PROPERTY_DECL(bool, libraryUseRelativePathOnExport);
    // Count, 0..
    PROPERTY_DECL(int, libraryHistoryMinTracksToKeep);
    // Count, 0..
    PROPERTY_DECL(int, libraryHistoryTrackDuplicateDistance);
    // Percent, 0..1.0
    PROPERTY_DECL(double, librarySearchBpmFuzzyRange);
    // Duration (ms), 100..9999
    PROPERTY_DECL(int, librarySearchDebouncingTimeout);
    PROPERTY_DECL(bool, librarySearchCompletionsEnable);
    PROPERTY_DECL(bool, librarySearchHistoryShortcutsEnable);
    // Integration
    PROPERTY_DECL(bool, libraryRhythmboxEnabled);
    PROPERTY_DECL(bool, libraryBansheeEnabled);
    PROPERTY_DECL(bool, libraryITunesEnabled);
    PROPERTY_DECL(bool, libraryTraktorEnabled);
    PROPERTY_DECL(bool, libraryRekordboxEnabled);
    PROPERTY_DECL(bool, librarySeratoEnabled);

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
