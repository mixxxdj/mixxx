#include "waveform/sharedglcontext.h"

#include <QtDebug>
#include <QGLContext>
#include <QGLFormat>
#include <QGLWidget>

QGLWidget* SharedGLContext::s_pSharedGLWidget = nullptr;

// static
void SharedGLContext::setWidget(QGLWidget* pWidget) {
    s_pSharedGLWidget = pWidget;
    qDebug() << "Set root GL Context widget valid:"
             << pWidget << (pWidget && pWidget->isValid());
    if (pWidget) {
        const QGLContext* pContext = pWidget->context();
        qDebug() << "Created root GL Context valid:" << pContext
                 << (pContext && pContext->isValid());
        QGLFormat format = pWidget->format();
        qDebug() << "Root GL Context format:";
        qDebug() << "Double Buffering:" << format.doubleBuffer();
        qDebug() << "Swap interval:" << format.swapInterval();
        qDebug() << "Depth buffer:" << format.depth();
        qDebug() << "Direct rendering:" << format.directRendering();
        qDebug() << "Has overlay:" << format.hasOverlay();
        qDebug() << "RGBA:" << format.rgba();
        qDebug() << "Sample buffers:" << format.sampleBuffers();
        qDebug() << "Samples:" << format.samples();
        qDebug() << "Stencil buffers:" << format.stencil();
        qDebug() << "Stereo:" << format.stereo();
    }
}

// static
QGLWidget* SharedGLContext::getWidget() {
    return s_pSharedGLWidget;
}
