#include <QtDebug>
#include <QGLContext>
#include <QGLFormat>
#include <QGLWidget>

#include "sharedglcontext.h"

const QGLWidget* SharedGLContext::s_pSharedGLWidget = NULL;

// static
void SharedGLContext::setWidget(const QGLWidget* pWidget) {
    s_pSharedGLWidget = pWidget;
    qDebug() << "Set root GL Context widget valid:"
             << pWidget << (pWidget && pWidget->isValid());
    const QGLContext* pContext = pWidget->context();
    qDebug() << "Created root GL Context valid:" << pContext
             << (pContext && pContext->isValid());
}

// static
const QGLWidget* SharedGLContext::getWidget() {
    return s_pSharedGLWidget;
}
