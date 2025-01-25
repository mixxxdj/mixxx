#include "mixxxapplication.h"

#include <QThreadPool>
#include <QTouchEvent>
#include <QtDebug>
#include <QtGlobal>

#include "audio/frame.h"
#include "audio/types.h"
#include "control/controlproxy.h"
#include "library/relocatedtrack.h"
#include "library/trackset/crate/crateid.h"
#include "moc_mixxxapplication.cpp"
#include "soundio/soundmanagerutil.h"
#include "track/track.h"
#include "track/trackref.h"
#include "util/assert.h"
#include "util/cache.h"
#include "util/cmdlineargs.h"
#include "util/color/rgbcolor.h"
#include "util/fileinfo.h"
#include "util/math.h"

// When linking Qt statically, the Q_IMPORT_PLUGIN is needed for each linked plugin.
// https://doc.qt.io/qt-5/plugins-howto.html#details-of-linking-static-plugins
#ifdef QT_STATIC
#include <QtPlugin>
#if defined(Q_OS_WASM)
Q_IMPORT_PLUGIN(QWasmIntegrationPlugin)
#elif defined(Q_OS_WIN)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
Q_IMPORT_PLUGIN(QModernWindowsStylePlugin)
#else
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif
#elif defined(Q_OS_IOS)
Q_IMPORT_PLUGIN(QIOSIntegrationPlugin)
#elif defined(Q_OS_MACOS)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
Q_IMPORT_PLUGIN(QMacStylePlugin)
#elif defined(Q_OS_LINUX)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#else
#error "Q_IMPORT_PLUGIN() for the current patform is missing"
#endif
#if !defined(Q_OS_WASM)
Q_IMPORT_PLUGIN(QOffscreenIntegrationPlugin)
Q_IMPORT_PLUGIN(QMinimalIntegrationPlugin)
#endif

Q_IMPORT_PLUGIN(QSQLiteDriverPlugin)
Q_IMPORT_PLUGIN(QTlsBackendOpenSSL)
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QGifPlugin)

#endif // QT_STATIC

namespace {

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
/// This class allows to change the button of a mouse event on the fly.
/// This is required because we want to change the behaviour of Qts mouse
/// buttony synthesizer without duplicate all the code.
class QMouseEventEditable : public QMouseEvent {
  public:
    void setButton(Qt::MouseButton button) {
        b = button;
    }
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 4) && defined(__APPLE__)
    // We also use this class to modify erroneous mouseState. See
    // MixxxApplication::notify(...) for details.
    void setButtons(Qt::MouseButtons mouseState) {
        this->mouseState = mouseState;
    }
#endif
};
#endif

// kEventNotifyExecTimeWarningThreshold defines the threshold duration for event
// processing warnings. If the processing time of an event exceeds this duration
// in developer mode, a warning will be logged. This is used to identify
// potentially slow event processing in the application, which could impact
// performance. With a 60Hz waveform update rate, paint and swap events must be
// processed through the event queue every 16.6ms, to ensure smooth rendering.
// Exceeding this processing time can lead to visible delays, therefore 10ms is a
// reasonable threshold.
constexpr mixxx::Duration kEventNotifyExecTimeWarningThreshold = mixxx::Duration::fromMillis(10);

} // anonymous namespace

MixxxApplication::MixxxApplication(int& argc, char** argv)
        : QApplication(argc, argv),
          m_rightPressedButtons(0),
          m_pTouchShift(nullptr),
          m_isDeveloper(CmdlineArgs::Instance().getDeveloper()) {
    registerMetaTypes();

    // Increase the size of the global thread pool to at least
    // 4 threads, even if less cores are available. These threads
    // will be used for loading external libraries and other tasks.
    QThreadPool::globalInstance()->setMaxThreadCount(
            math_max(4, QThreadPool::globalInstance()->maxThreadCount()));
}

void MixxxApplication::registerMetaTypes() {
    // PCM audio types
    qRegisterMetaType<mixxx::audio::ChannelCount>("mixxx::audio::ChannelCount");
    qRegisterMetaType<mixxx::audio::ChannelLayout>("mixxx::audio::ChannelLayout");
    qRegisterMetaType<mixxx::audio::OptionalChannelLayout>("mixxx::audio::OptionalChannelLayout");
    qRegisterMetaType<mixxx::audio::SampleRate>("mixxx::audio::SampleRate");
    qRegisterMetaType<mixxx::audio::Bitrate>("mixxx::audio::Bitrate");

    // Tracks
    qRegisterMetaType<TrackId>();
    qRegisterMetaType<QSet<TrackId>>();
    qRegisterMetaType<QList<TrackId>>();
    qRegisterMetaType<TrackRef>();
    qRegisterMetaType<QList<TrackRef>>();
    qRegisterMetaType<QList<QPair<TrackRef, TrackRef>>>();
    qRegisterMetaType<TrackPointer>();

    // Crates
    qRegisterMetaType<CrateId>();
    qRegisterMetaType<QSet<CrateId>>();
    qRegisterMetaType<QList<CrateId>>();

    // Sound devices
    qRegisterMetaType<SoundDeviceId>();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMetaType::registerComparators<SoundDeviceId>();
#endif

    // Library Scanner
    qRegisterMetaType<RelocatedTrack>();
    qRegisterMetaType<QList<RelocatedTrack>>();

    // Various custom data types
    qRegisterMetaType<mixxx::ReplayGain>("mixxx::ReplayGain");
    qRegisterMetaType<mixxx::cache_key_t>("mixxx::cache_key_t");
    qRegisterMetaType<mixxx::Bpm>("mixxx::Bpm");
    qRegisterMetaType<mixxx::Duration>("mixxx::Duration");
    qRegisterMetaType<mixxx::audio::FramePos>("mixxx::audio::FramePos");
    qRegisterMetaType<std::optional<mixxx::RgbColor>>("std::optional<mixxx::RgbColor>");
    qRegisterMetaType<mixxx::FileInfo>("mixxx::FileInfo");
}

bool MixxxApplication::notify(QObject* pTarget, QEvent* pEvent) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // All touch events are translated into two simultaneous events: one for
    // the target QWidgetWindow and one for the target QWidget.
    // A second touch becomes a mouse move without additional press and release
    // events.
    switch (pEvent->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEventEditable* pMouseEvent =
                static_cast<QMouseEventEditable*>(pEvent); // clazy:exclude=wrong-qevent-cast
        if (pMouseEvent->source() == Qt::MouseEventSynthesizedByQt &&
                pMouseEvent->button() == Qt::LeftButton &&
                touchIsRightButton()) {
            // Assert the assumption that QT synthesizes only one click at a time
            // = two events (see above)
            VERIFY_OR_DEBUG_ASSERT(m_rightPressedButtons < 2) {
                break;
            }
            pMouseEvent->setButton(Qt::RightButton);
            m_rightPressedButtons++;
        }
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 4) && defined(__APPLE__)
        if (pMouseEvent->button() == Qt::RightButton && pMouseEvent->buttons() == Qt::LeftButton) {
            // Workaround for a bug in Qt 5.12 qnsview_mouse.mm, where the wrong value is
            // assigned to the event's mouseState for simulated rightbutton press events
            // (using ctrl+leftbotton), which results in a missing release event for that
            // press event.
            //
            // Fixed in Qt 5.12.5. See
            // https://github.com/qt/qtbase/commit/9a47768b46f5e5eed407b70dfa9183fa1d21e242
            pMouseEvent->setButtons(Qt::RightButton);
        }
#endif
        break;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEventEditable* pMouseEvent =
                static_cast<QMouseEventEditable*>(pEvent); // clazy:exclude=wrong-qevent-cast
        if (pMouseEvent->source() == Qt::MouseEventSynthesizedByQt &&
                pMouseEvent->button() == Qt::LeftButton &&
                m_rightPressedButtons > 0) {
            pMouseEvent->setButton(Qt::RightButton);
            m_rightPressedButtons--;
        }
        break;
    }
    default:
        break;
    }
#endif

    PerformanceTimer time;

    if (m_isDeveloper) {
        time.start();
    }

    bool ret = QApplication::notify(pTarget, pEvent);

    VERIFY_OR_DEBUG_ASSERT(pTarget != nullptr) {
        qWarning() << "Processed" << pEvent->type() << "for null pointer, this is probably a bug!";
        return ret;
    }

    if (m_isDeveloper &&
            time.elapsed() > kEventNotifyExecTimeWarningThreshold) {
        QDebug debug = qDebug();
        debug << "Processing"
              << pEvent->type()
              << "for object";
        if (pEvent->type() == QEvent::DeferredDelete ||
                pEvent->type() == QEvent::ChildRemoved ||
                pEvent->type() == QEvent::Timer) {
            // pTarget can be already dangling in case of DeferredDelete
            debug << static_cast<void*>(pTarget); // will print dangling address
        } else {
            debug << pTarget // will print address, class and object name
                  << "running in thread:"
                  << pTarget->thread()->objectName();
        }
        debug << "took"
              << time.elapsed().debugMillisWithUnit();
    }

    return ret;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool MixxxApplication::touchIsRightButton() {
    if (!m_pTouchShift) {
        m_pTouchShift = new ControlProxy(
                "[Controls]", "touch_shift", this);
    }
    return m_pTouchShift->toBool();
}
#endif
