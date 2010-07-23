#include <QtDebug>
#include <QtCore>
#include <QtGui>
#include "wsampler.h"
#include "widget/wwidget.h"
#include "widget/wabstractcontrol.h"
#include "widget/wknob.h"
#include "widget/wpushbutton.h"
#include "widget/wslider.h"
#include "widget/wslidercomposed.h"
#include "widget/wdisplay.h"
#include "widget/wvumeter.h"
#include "widget/wstatuslight.h"
#include "widget/wlabel.h"
#include "widget/wnumber.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberbpm.h"
#include "widget/wnumberrate.h"
#include "widget/wpixmapstore.h"
#include "widget/wsearchlineedit.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrary.h"
#include "widget/wsampler.h"

#include "widget/woverview.h"
#include "mixxxkeyboard.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"
#include "waveformviewerfactory.h"
#include "waveform/waveformrenderer.h"
#include "widget/wvisualsimple.h"
#include "widget/wglwaveformviewer.h"
#include "widget/wwaveformviewer.h"
#include "trackinfoobject.h"
#include "sampler.h"
#include "samplermanager.h"

#include "imgloader.h"
#include "imginvert.h"
#include "imgcolor.h"
#include "widget/wskincolor.h"
#include "mixxx.h"

WSampler::WSampler(QWidget* parent, SamplerManager* pSamplerManager) :     
                    WWidget(parent),
                    m_pSamplerManager(pSamplerManager) {
    
    Sampler* pSampler1 = m_pSamplerManager->getSampler(1);
    m_pWaveformRendererCh3 = new WaveformRenderer("[Channel3]");
    
    
    connect(pSampler1, SIGNAL(newTrackLoaded(TrackInfoObject *)),
            m_pWaveformRendererCh3, SLOT(slotNewTrack(TrackInfoObject *)));
    connect(pSampler1, SIGNAL(unloadingTrack(TrackInfoObject *)),
            m_pWaveformRendererCh3, SLOT(slotNewTrack(TrackInfoObject *)));
            
    m_pOverviewCh3 = 0;
    m_pSamplerWindow = 0;
}

WSampler::~WSampler() {
    
    
    if(m_pWaveformRendererCh3) {
          delete m_pWaveformRendererCh3;
          m_pWaveformRendererCh3 = NULL;
         }

}

void WSampler::setup(QDomNode node, QList<QObject *> m_qWidgetList){
    
    m_pSamplerWindow = new QFrame(this, Qt::Window | Qt::Tool);
    m_pSamplerWindow->resize(800,100);
    
    QPalette palette = m_pSamplerWindow->palette();
    palette.setColor(backgroundRole(), QColor(24, 24, 24));
    m_pSamplerWindow->setPalette(palette);
    m_pSamplerWindow->setAutoFillBackground(true);
    
    QPushButton *saveButton = new QPushButton(m_pSamplerWindow);
    saveButton->setText("Save Sampler Bank");
    saveButton->move(640,70);
    
    QPushButton *loadButton = new QPushButton(m_pSamplerWindow);
    loadButton->setText("Load Sampler Bank");
    loadButton->move(640,0);
    
    saveSamplerBank = new QAction(tr("&Save Sampler Bank"), this);
    connect(saveButton, SIGNAL(clicked()), saveSamplerBank, SIGNAL(activated()));
    connect(saveSamplerBank, SIGNAL(activated()), this, SLOT(slotSaveSamplerBank()));
    
    loadSamplerBank = new QAction(tr("&Load Sampler Bank"), this);
    connect(loadButton, SIGNAL(clicked()), loadSamplerBank, SIGNAL(activated()));
    connect(loadSamplerBank, SIGNAL(activated()), this, SLOT(slotLoadSamplerBank()));
    
    
    QDomNode samplerNode = node.firstChild();
    while (!samplerNode.isNull())
    {
        if (samplerNode.nodeName()=="PushButton")
        {
            WPushButton * p = new WPushButton(m_pSamplerWindow);
            p->setup(samplerNode);
            //p->installEventFilter(m_pKeyboard);
            p->setParent(m_pSamplerWindow);
            m_qWidgetList.append(p);
        } else if (samplerNode.nodeName()=="Overview")
        {
            if (WWidget::selectNodeInt(samplerNode, "Channel")==3)
            {
                if (m_pOverviewCh3 == 0)
                    m_pOverviewCh3 = new WOverview("[Channel3]", m_pSamplerWindow);
                m_pOverviewCh3->setup(samplerNode);
                m_pOverviewCh3->setParent(m_pSamplerWindow);
		        m_pOverviewCh3->show();
            }
            
        } else if (samplerNode.nodeName()=="Knob")
        {
            WKnob * p = new WKnob(this);
            p->setup(samplerNode);
            p->setParent(m_pSamplerWindow);
            //p->installEventFilter(m_pKeyboard);
            m_qWidgetList.append(p);
            //currentControl = qobject_cast<WAbstractControl*>(p);
        }
        samplerNode = samplerNode.nextSibling();
        
    }
    //WWidget::setup(node);
    
    
    
    Sampler* pSampler1 = m_pSamplerManager->getSampler(1);
    
    connect(pSampler1, SIGNAL(newTrackLoaded(TrackInfoObject*)),
        m_pOverviewCh3, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    connect(pSampler1, SIGNAL(unloadingTrack(TrackInfoObject*)),
              m_pOverviewCh3, SLOT(slotUnloadTrack(TrackInfoObject*)));
              
    connect(pSampler1, SIGNAL(newTrackLoaded(TrackInfoObject*)),
     		this, SLOT(slotSetupTrackConnectionsCh3(TrackInfoObject*)));
     		
    m_pSamplerWindow->show();
}

void WSampler::slotSaveSamplerBank() {
    QString s = QFileDialog::getSaveFileName(this, tr("Save Sampler Bank"));
    QFile file(s);
    if(!file.open(IO_WriteOnly)) {
        qDebug("Cannot write to file.");
    };
    QDomDocument samplerBank("samplerbank");
    
    QDomElement sampler1 = samplerBank.createElement( "sampler1" );
    QString loc1 = m_pSamplerManager->getTrackLocation(1);
    sampler1.setAttribute( "location", loc1);
    samplerBank.appendChild(sampler1);
    
    QDomElement sampler2 = samplerBank.createElement( "sampler2" );
    QString loc2 = m_pSamplerManager->getTrackLocation(2);
    sampler2.setAttribute( "location", loc2);
    samplerBank.appendChild(sampler2);
    
    QDomElement sampler3 = samplerBank.createElement( "sampler3" );
    QString loc3 = m_pSamplerManager->getTrackLocation(3);
    sampler3.setAttribute( "location", loc3);
    samplerBank.appendChild(sampler3);
    
    QDomElement sampler4 = samplerBank.createElement( "sampler4" );
    QString loc4 = m_pSamplerManager->getTrackLocation(4);
    sampler4.setAttribute( "location", loc4);
    samplerBank.appendChild(sampler4);
    
    file.write(samplerBank.toString());
}

void WSampler::slotLoadSamplerBank() {
    QString s = QFileDialog::getOpenFileName(this, tr("Load Sampler Bank"));
    QFile file(s);
    if(!file.open(IO_ReadOnly)) {
        qDebug("Cannot read file.");
    };
    QDomDocument doc;
    doc.setContent(file.readAll());
    
    QDomNode n = doc.firstChild();
    qDebug() << n.nodeName();
    while(!n.isNull()) {
        qDebug("In Loop");
        if (n.isElement()) {
            QDomElement e = n.toElement();
            
            qDebug() << e.tagName();
            if(e.tagName() == "sampler1") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 1);
            }
            if(e.tagName() == "sampler2") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 1);
            }
            if(e.tagName() == "sampler3") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 1);
            }
            if(e.tagName() == "sampler4") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 1);
            }
        }
        n = n.nextSibling();
    }
    
    file.close();
    
}

void WSampler::slotSetupTrackConnectionsCh3(TrackInfoObject* pTrack) {
    connect(pTrack, SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
		m_pOverviewCh3, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
}