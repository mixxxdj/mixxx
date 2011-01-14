
#include <QtDebug>
#include <QtGui>
#include <QGLContext>

#include "configobject.h"
#include "waveformviewerfactory.h"

#include "waveform/waveformrenderer.h"
#include "widget/wvisualsimple.h"
#include "widget/wglwaveformviewer.h"
#include "widget/wwaveformviewer.h"


QList<QObject*> WaveformViewerFactory::m_viewers = QList<QObject*>();
QList<WVisualSimple*> WaveformViewerFactory::m_simpleViewers = QList<WVisualSimple*>();
QList<WWaveformViewer*> WaveformViewerFactory::m_visualViewers = QList<WWaveformViewer*>();
QList<WGLWaveformViewer*> WaveformViewerFactory::m_visualGLViewers = QList<WGLWaveformViewer*>();
QGLContext* WaveformViewerFactory::s_pSharedOGLCtxt = (QGLContext *)NULL;


WaveformViewerType WaveformViewerFactory::createWaveformViewer(const char *group, QWidget *parent, ConfigObject<ConfigValue> *pConfig, QObject **target, WaveformRenderer* pWaveformRenderer) {
    
    QGLContext *ctxt;
    
    if ( s_pSharedOGLCtxt == (QGLContext*)NULL ) {
        s_pSharedOGLCtxt = new QGLContext(QGLFormat(QGL::SampleBuffers));
        s_pSharedOGLCtxt->create();
        s_pSharedOGLCtxt->makeCurrent();
    }
    
    ctxt = new QGLContext(QGLFormat(QGL::SampleBuffers));
    ctxt->create(s_pSharedOGLCtxt);

    qDebug() << "createWaveformViewer()";

    bool bVisualWaveform = true;
    WaveformViewerType ret = WAVEFORM_INVALID;

    if(target == NULL) {
        qDebug() << "WaveformViewerFactory::createWaveformViewer -- invalid target pointer";
        return WAVEFORM_INVALID;
    }
    
    if (pConfig->getValueString(ConfigKey("[Controls]", "Visuals")).toInt() == 1) {
        bVisualWaveform = false;
    }

    if(bVisualWaveform) {
        
        qDebug() << "WaveformViewerFactory :: Creating new visual waveform";
        // Support shared GL rendering contexts
        WGLWaveformViewer *other = (m_visualGLViewers.isEmpty() ? NULL : (WGLWaveformViewer*)m_visualGLViewers.first());
        if(other == NULL)
            qDebug() << "WaveformViewerFactory :: Making new GL context.";
        else
            qDebug() << "WaveformViewerFactory :: Sharing existing GL context.";
        
        WGLWaveformViewer *visual = new WGLWaveformViewer(group, pWaveformRenderer, parent, NULL, ctxt);

        if(visual->isValid()) {
            m_visualGLViewers.append(visual);
            m_viewers.append(visual);
            ret = WAVEFORM_GL;
            *target = visual;
        } else {

            // GL doesn't work for some reason, warn them and turn off GL viewers.
            pConfig->set(ConfigKey("[Controls]","Visuals"), ConfigValue(1));
            QMessageBox * mb = new QMessageBox(parent);
            mb->setWindowTitle(QString("Waveform Displays"));
            mb->setIcon(QMessageBox::Information);
            mb->setText("OpenGL cannot be initialized, which means that\nthe waveform displays won't work. A simple\nmode will be used instead where you can still\nuse the mouse to change speed.");
            mb->show();

            bVisualWaveform = false;
            if(visual != NULL)
                delete visual;
            visual = NULL;

            /*qDebug() << "Making a nongl viewer";
            WWaveformViewer *nongl = new WWaveformViewer(group,pWaveformRenderer,parent);
            m_visualViewers.append(nongl);
            m_viewers.append(nongl);
            ret = WAVEFORM_WIDGET;
            *target = nongl;*/
        }
        
    }

    // WTF: Intentionally separate from previous block.
    if(!bVisualWaveform) {
        qDebug() << "WaveformViewerFactory :: Creating new simple waveform";
        // Preference is for simple or regular, for now just simple.
        WVisualSimple *simple = new WVisualSimple(group,parent);
        m_simpleViewers.append(simple);
        m_viewers.append(simple);
        ret = WAVEFORM_SIMPLE;
        *target = simple;
    }

    return ret;
}

void WaveformViewerFactory::destroyWaveformViewer(QObject *pWaveformViewer) {
    qDebug() << "destroyWaveformViewer()";

    if(pWaveformViewer == NULL)
        return;
    
    // Precondition is that we created this waveform viewer.
    if(!m_viewers.contains(pWaveformViewer))
        return;

    
    int index = m_viewers.indexOf(pWaveformViewer);
    //ASSERT(index != -!);
    m_viewers.removeAt(index);

    index = m_simpleViewers.indexOf((WVisualSimple*)pWaveformViewer);
    if(index != -1)
        m_simpleViewers.removeAt(index);

    index = m_visualViewers.indexOf((WWaveformViewer*)pWaveformViewer);
    if(index != -1)
        m_visualViewers.removeAt(index);
    
    index = m_visualGLViewers.indexOf((WGLWaveformViewer*)pWaveformViewer);
    if(index != -1)
        m_visualGLViewers.removeAt(index);

    delete pWaveformViewer;
    
}

WaveformViewerType WaveformViewerFactory::getWaveformViewerType(QObject *pWaveformViewer) {
    if(pWaveformViewer == NULL)
        return WAVEFORM_INVALID;
    if(m_simpleViewers.indexOf((WVisualSimple*)pWaveformViewer) != -1)
        return WAVEFORM_SIMPLE;
    if(m_visualViewers.indexOf((WWaveformViewer*)pWaveformViewer) != -1)
        return WAVEFORM_WIDGET;
    if(m_visualGLViewers.indexOf((WGLWaveformViewer*)pWaveformViewer) != -1)
        return WAVEFORM_GL;
    return WAVEFORM_INVALID;
}
