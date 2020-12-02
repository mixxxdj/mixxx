#include "waveform/widgets/qtvsynctestwidget.h"

#include <QtCore/qglobal.h>
#include <stddef.h>

#include <QDebug>
#include <QGLContext>
#include <QPainter>
#include <QtCore>

#include "util/performancetimer.h"
#include "waveform/renderers/qtvsynctestrenderer.h"
#include "waveform/sharedglcontext.h"
#include "waveform/widgets/waveformwidgetabstract.h"

class QPaintEvent;
class QWidget;

QtVSyncTestWidget::QtVSyncTestWidget(const QString& group, QWidget* parent)
        : QGLWidget(parent, SharedGLContext::getWidget()),
          WaveformWidgetAbstract(group) {
    qDebug() << "Created QGLWidget. Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }

    addRenderer<QtVSyncTestRenderer>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    m_initSuccess = init();
}

QtVSyncTestWidget::~QtVSyncTestWidget() {
}

void QtVSyncTestWidget::castToQWidget() {
    m_widget = this;
}

void QtVSyncTestWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

mixxx::Duration QtVSyncTestWidget::render() {
    PerformanceTimer timer;
    mixxx::Duration t1;
    //mixxx::Duration t2, t3;
    timer.start();
    // QPainter makes QGLContext::currentContext() == context()
    // this may delayed until previous buffer swap finished
    QPainter painter(this);
    t1 = timer.restart();
    draw(&painter, NULL);
    //t2 = timer.restart();
    //qDebug() << "GLVSyncTestWidget "<< t1 << t2;
    return t1; // return timer for painter setup
}
