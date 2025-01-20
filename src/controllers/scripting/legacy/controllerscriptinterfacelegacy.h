#pragma once

#include <QJSValue>
#include <QObject>

#include "controllers/softtakeover.h"
#include "util/alphabetafilter.h"
#include "util/runtimeloggingcategory.h"

#ifdef MIXXX_USE_QML
#include "qml/qmlplayerproxy.h"
#include "qml/qmlplayermanagerproxy.h"
#endif

class ControllerScriptEngineLegacy;
class ControlObjectScript;
class ScriptConnection;
class ConfigKey;

/// ControllerScriptInterfaceLegacy is the legacy API for controller scripts to interact
/// with Mixxx. It is inserted into the JS environment as the "engine" object.
class ControllerScriptInterfaceLegacy : public QObject {
    Q_OBJECT
  public:
    // NOTE: these enumerator names are exposed to the JS engine! Removal/Changing of
    // any name is likely breaking. Only add more and only remove enumerators if
    // they're broken to begin with.
    enum class Charset {
        ASCII,
        UTF_8,
        UTF_16LE,
        UTF_16BE,
        UTF_32LE,
        UTF_32BE,
        CentralEurope,
        Cyrillic,
        Latin1,
        Greek,
        Turkish,
        Hebrew,
        Arabic,
        Baltic,
        Vietnamese,
        Latin9,
        Shift_JIS,
        EUC_JP,
        EUC_KR,
        Big5_HKSCS,
        KOI8_U,
        UCS2,
        SCSU,
        BOCU_1,
        CESU_8
    };
    Q_ENUM(Charset)

#ifdef MIXXX_USE_QML
    class PlayerProxy: public QObject {
        Q_OBJECT
        Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY trackChanged)
        Q_PROPERTY(QString artist READ getArtist NOTIFY artistChanged)
        Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
        Q_PROPERTY(QString album READ getAlbum NOTIFY albumChanged)
        Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                        NOTIFY albumArtistChanged)
        Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
        Q_PROPERTY(QString composer READ getComposer NOTIFY composerChanged)
        Q_PROPERTY(QString grouping READ getGrouping NOTIFY groupingChanged)
        Q_PROPERTY(QString year READ getYear NOTIFY yearChanged)
        Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                        NOTIFY trackNumberChanged)
        Q_PROPERTY(QString trackTotal READ getTrackTotal NOTIFY trackTotalChanged)

      public:
        PlayerProxy(mixxx::qml::QmlPlayerProxy* qmlPlayerProxy): m_pQmlPlayerProxy(qmlPlayerProxy){
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::trackChanged, this, &PlayerProxy::slotTrackChanged);
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::titleChanged, [this](){
                emit titleChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::artistChanged, [this](){
                emit artistChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::albumChanged, [this](){
                emit albumChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::albumArtistChanged, [this](){
                emit albumArtistChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::genreChanged, [this](){
                emit genreChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::composerChanged, [this](){
                emit composerChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::groupingChanged, [this](){
                emit groupingChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::yearChanged, [this](){
                emit yearChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::trackNumberChanged, [this](){
                emit trackNumberChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::trackTotalChanged, [this](){
                emit trackTotalChanged();
            });
            connect(m_pQmlPlayerProxy, &mixxx::qml::QmlPlayerProxy::commentChanged, [this](){
                emit commentChanged();
            });
        }

        bool isLoaded() const;
        QString getTitle() const;
        QString getArtist() const;
        QString getAlbum() const;
        QString getAlbumArtist() const;
        QString getGenre() const;
        QString getComposer() const;
        QString getGrouping() const;
        QString getYear() const;
        QString getTrackNumber() const;
        QString getTrackTotal() const;

      signals:
        void titleChanged();
        void artistChanged();
        void albumChanged();
        void albumArtistChanged();
        void genreChanged();
        void composerChanged();
        void groupingChanged();
        void yearChanged();
        void trackNumberChanged();
        void trackTotalChanged();
        void commentChanged();

      private slots:
        void slotTrackChanged() {
            emit artistChanged();
            emit titleChanged();
            emit albumChanged();
            emit albumArtistChanged();
            emit genreChanged();
            emit composerChanged();
            emit groupingChanged();
            emit yearChanged();
            emit trackNumberChanged();
            emit trackTotalChanged();
            emit commentChanged();
        };

      private:
        mixxx::qml::QmlPlayerProxy* m_pQmlPlayerProxy;
    };
#endif

    ControllerScriptInterfaceLegacy(ControllerScriptEngineLegacy* m_pEngine,
            const RuntimeLoggingCategory& logger);

    virtual ~ControllerScriptInterfaceLegacy();

    Q_INVOKABLE QJSValue getSetting(const QString& name);
    Q_INVOKABLE QObject* getPlayer(const QString& deck);
    Q_INVOKABLE double getValue(const QString& group, const QString& name);
    Q_INVOKABLE void setValue(const QString& group, const QString& name, double newValue);
    Q_INVOKABLE double getParameter(const QString& group, const QString& name);
    Q_INVOKABLE void setParameter(const QString& group, const QString& name, double newValue);
    Q_INVOKABLE double getParameterForValue(
            const QString& group, const QString& name, double value);
    Q_INVOKABLE void reset(const QString& group, const QString& name);
    Q_INVOKABLE double getDefaultValue(const QString& group, const QString& name);
    Q_INVOKABLE double getDefaultParameter(const QString& group, const QString& name);
    Q_INVOKABLE QJSValue makeConnection(const QString& group,
            const QString& name,
            const QJSValue& callback);
    Q_INVOKABLE QJSValue makeUnbufferedConnection(const QString& group,
            const QString& name,
            const QJSValue& callback);
    // DEPRECATED: Use makeConnection instead.
    Q_INVOKABLE QJSValue connectControl(const QString& group,
            const QString& name,
            const QJSValue& passedCallback,
            bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    Q_INVOKABLE void trigger(const QString& group, const QString& name);
    // DEPRECATED: Use console.log instead.
    Q_INVOKABLE void log(const QString& message);
    Q_INVOKABLE int beginTimer(int interval, QJSValue scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck,
            int intervalsPerRev,
            double rpm,
            double alpha,
            double beta,
            bool ramp = true);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck, bool ramp = true);
    Q_INVOKABLE bool isScratching(int deck);
    Q_INVOKABLE void softTakeover(const QString& group, const QString& name, bool set);
    Q_INVOKABLE void softTakeoverIgnoreNextValue(const QString& group, const QString& name);
    Q_INVOKABLE bool softTakeoverWillIgnore(
            const QString& group, const QString& name, double parameter);
    Q_INVOKABLE void brake(const int deck,
            bool activate,
            double factor = 1.0,
            const double rate = 1.0);
    Q_INVOKABLE void spinback(const int deck,
            bool activate,
            double factor = 1.8,
            const double rate = -10.0);
    Q_INVOKABLE void softStart(const int deck, bool activate, double factor = 1.0);

    Q_INVOKABLE QByteArray convertCharset(
            const ControllerScriptInterfaceLegacy::Charset
                    targetCharset,
            const QString& value);

    bool removeScriptConnection(const ScriptConnection& conn);
    /// Execute a ScriptConnection's JS callback
    void triggerScriptConnection(const ScriptConnection& conn);

    /// Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent* event);

  private:
    QJSValue makeConnectionInternal(const QString& group,
            const QString& name,
            const QJSValue& callback,
            bool skipSuperseded = false);

    QByteArray convertCharsetInternal(const QString& targetCharset, const QString& value);

    QHash<ConfigKey, ControlObjectScript*> m_controlCache;
    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);

    SoftTakeoverCtrl m_st;

    struct TimerInfo {
        QJSValue callback;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;

    QVarLengthArray<int> m_intervalAccumulator;
    QVarLengthArray<mixxx::Duration> m_lastMovement;
    QVarLengthArray<double> m_dx, m_rampTo, m_rampFactor;
    QVarLengthArray<bool> m_ramp, m_brakeActive, m_spinbackActive, m_softStartActive;
    QVarLengthArray<AlphaBetaFilter*> m_scratchFilters;
    QHash<int, int> m_scratchTimers;
    /// Applies the accumulated movement to the track speed
    void scratchProcess(int timerId);
    void stopScratchTimer(int timerId);
    bool isDeckPlaying(const QString& group);
    void stopDeck(const QString& group);
    bool isTrackLoaded(const QString& group);
    double getDeckRate(const QString& group);

    ControllerScriptEngineLegacy* m_pScriptEngineLegacy;
    const RuntimeLoggingCategory m_logger;

#ifdef MIXXX_USE_QML
    std::unique_ptr<mixxx::qml::QmlPlayerManagerProxy> m_pPlayerManagerProxy;
#endif
};
