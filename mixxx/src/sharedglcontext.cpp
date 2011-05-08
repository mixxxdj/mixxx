#include <QGLContext>
#include <QGLFormat>
#include "sharedglcontext.h"

/** Singleton wrapper around QGLContext */

QGLContext* SharedGLContext::s_pSharedGLContext = (QGLContext*)NULL;

SharedGLContext::SharedGLContext()
{

}

SharedGLContext::~SharedGLContext()
{

}

QGLContext* SharedGLContext::getContext()
{
    QGLContext *ctxt;
    
    if (s_pSharedGLContext == (QGLContext*)NULL) {
        s_pSharedGLContext = new QGLContext(QGLFormat(QGL::SampleBuffers));
        s_pSharedGLContext->create();
        s_pSharedGLContext->makeCurrent();
    }
    
    ctxt = new QGLContext(QGLFormat(QGL::SampleBuffers));
    ctxt->create(s_pSharedGLContext);

    return ctxt;
}
