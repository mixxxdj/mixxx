#include "waveform/sharedglcontext.h"

WGLWidget* SharedGLContext::s_pSharedGLWidget = nullptr;

// static
void SharedGLContext::setWidget(WGLWidget* pWidget) {
    s_pSharedGLWidget = pWidget;
}

// static
WGLWidget* SharedGLContext::getWidget() {
    return s_pSharedGLWidget;
}
