#include <QtDebug>
#include <QTouchEvent>
#include <QThreadPool>

#include "mixxxapplication.h"

#include "control/controlproxy.h"
#include "library/crate/crateid.h"
#include "track/track.h"
#include "util/math.h"

// When linking Qt statically on Windows we have to Q_IMPORT_PLUGIN all the
// plugins we link in build/depends.py.
#ifdef QT_NODLL
#include <QtPlugin>
// sqldrivers plugins
Q_IMPORT_PLUGIN(QSQLiteDriverPlugin)
// platform plugins
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
// style plugins
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
// imageformats plugins
Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QSvgIconPlugin)
Q_IMPORT_PLUGIN(QICOPlugin)
Q_IMPORT_PLUGIN(QTgaPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QGifPlugin)
// accessible plugins
// TODO(rryan): This is supposed to exist but does not in our builds.
//Q_IMPORT_PLUGIN(AccessibleFactory)
#endif


MixxxApplication::MixxxApplication(int& argc, char** argv)
        : QApplication(argc, argv),
          m_fakeMouseSourcePointId(0),
          m_fakeMouseWidget(NULL),
          m_activeTouchButton(Qt::NoButton),
          m_pTouchShift(NULL) {
    registerMetaTypes();

    // Increase the size of the global thread pool to at least
    // 4 threads, even if less cores are available. These threads
    // will be used for loading external libraries and other tasks.
    QThreadPool::globalInstance()->setMaxThreadCount(
            math_max(4, QThreadPool::globalInstance()->maxThreadCount()));
}

MixxxApplication::~MixxxApplication() {
}

void MixxxApplication::registerMetaTypes() {
    // Register custom data types for signal processing
    qRegisterMetaType<TrackId>("TrackId");
    qRegisterMetaType<QList<TrackId>>("QList<TrackId>");
    qRegisterMetaType<QSet<TrackId>>("QSet<TrackId>");
    qRegisterMetaType<CrateId>("CrateId");
    qRegisterMetaType<QList<CrateId>>("QList<CrateId>");
    qRegisterMetaType<QSet<CrateId>>("QSet<CrateId>");
    qRegisterMetaType<TrackPointer>("TrackPointer");
    qRegisterMetaType<mixxx::ReplayGain>("mixxx::ReplayGain");
    qRegisterMetaType<mixxx::Bpm>("mixxx::Bpm");
    qRegisterMetaType<mixxx::Duration>("mixxx::Duration");
}

bool MixxxApplication::touchIsRightButton() {
    if (!m_pTouchShift) {
        m_pTouchShift = new ControlProxy(
                "[Controls]", "touch_shift", this);
    }
    return (m_pTouchShift->get() != 0.0);
}
