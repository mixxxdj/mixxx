#include <QGLContext>
#include <QGLFormat>
#include "sharedglcontext.h"

/** Singleton wrapper around QGLContext */

QGLContext* SharedGLContext::s_pSharedGLContext = (QGLContext*)NULL;

QGLContext* SharedGLContext::getContext() {
    if (s_pSharedGLContext == (QGLContext*)NULL) {
        s_pSharedGLContext = new QGLContext(QGLFormat::defaultFormat());
        s_pSharedGLContext->create();
        s_pSharedGLContext->makeCurrent();
    }

    QGLContext* ctxt = new QGLContext(QGLFormat::defaultFormat());
    ctxt->create(s_pSharedGLContext);
    return ctxt;
}
