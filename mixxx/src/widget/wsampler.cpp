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
    Sampler* pSampler2 = m_pSamplerManager->getSampler(2);
    Sampler* pSampler3 = m_pSamplerManager->getSampler(3);
    Sampler* pSampler4 = m_pSamplerManager->getSampler(4);
    m_pWaveformRendererCh3 = new WaveformRenderer("[Channel3]");
    m_pWaveformRendererCh4 = new WaveformRenderer("[Channel4]");
    m_pWaveformRendererCh5 = new WaveformRenderer("[Channel5]");
    m_pWaveformRendererCh6 = new WaveformRenderer("[Channel6]");
    
    
    connect(pSampler1, SIGNAL(newTrackLoaded(TrackInfoObject *)),
            m_pWaveformRendererCh3, SLOT(slotNewTrack(TrackInfoObject *)));
    connect(pSampler1, SIGNAL(unloadingTrack(TrackInfoObject *)),
            m_pWaveformRendererCh3, SLOT(slotNewTrack(TrackInfoObject *)));

    connect(pSampler2, SIGNAL(newTrackLoaded(TrackInfoObject *)),
            m_pWaveformRendererCh4, SLOT(slotNewTrack(TrackInfoObject *)));
    connect(pSampler2, SIGNAL(unloadingTrack(TrackInfoObject *)),
            m_pWaveformRendererCh4, SLOT(slotNewTrack(TrackInfoObject *)));
            
    connect(pSampler3, SIGNAL(newTrackLoaded(TrackInfoObject *)),
            m_pWaveformRendererCh5, SLOT(slotNewTrack(TrackInfoObject *)));
    connect(pSampler3, SIGNAL(unloadingTrack(TrackInfoObject *)),
            m_pWaveformRendererCh5, SLOT(slotNewTrack(TrackInfoObject *)));
            
    connect(pSampler4, SIGNAL(newTrackLoaded(TrackInfoObject *)),
            m_pWaveformRendererCh6, SLOT(slotNewTrack(TrackInfoObject *)));
    connect(pSampler4, SIGNAL(unloadingTrack(TrackInfoObject *)),
            m_pWaveformRendererCh6, SLOT(slotNewTrack(TrackInfoObject *)));
                
    m_pOverviewCh3 = 0;
    m_pOverviewCh4 = 0;
    m_pOverviewCh5 = 0;
    m_pOverviewCh6 = 0;
    m_pSamplerWindow = 0;
}

WSampler::~WSampler() {
    
    
    if(m_pWaveformRendererCh3) {
          delete m_pWaveformRendererCh3;
          m_pWaveformRendererCh3 = NULL;
    }
    if(m_pWaveformRendererCh4) {
          delete m_pWaveformRendererCh4;
          m_pWaveformRendererCh3 = NULL;
    }
    if(m_pWaveformRendererCh5) {
          delete m_pWaveformRendererCh5;
          m_pWaveformRendererCh3 = NULL;
    }
    if(m_pWaveformRendererCh6) {
          delete m_pWaveformRendererCh6;
          m_pWaveformRendererCh3 = NULL;
    }

}

void WSampler::setup(QDomNode node, QList<QObject *> m_qWidgetList){
    
    m_pSamplerWindow = new QFrame(this, Qt::Window | Qt::Tool);
    m_pSamplerWindow->resize(800,80);
    
    QPalette palette = m_pSamplerWindow->palette();
    palette.setColor(backgroundRole(), QColor(24, 24, 24));
    m_pSamplerWindow->setPalette(palette);
    m_pSamplerWindow->setAutoFillBackground(true);
    
    QPushButton *saveButton = new QPushButton(m_pSamplerWindow);
    saveButton->setText("Save Sampler Bank");
    saveButton->move(640,30);
    
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
            } else if (WWidget::selectNodeInt(samplerNode, "Channel")==4)
            {
                if (m_pOverviewCh4 == 0)
                    m_pOverviewCh4 = new WOverview("[Channel4]", m_pSamplerWindow);
                m_pOverviewCh4->setup(samplerNode);
                m_pOverviewCh4->setParent(m_pSamplerWindow);
    		    m_pOverviewCh4->show();
    		} else if (WWidget::selectNodeInt(samplerNode, "Channel")==5)
            {
                if (m_pOverviewCh5 == 0)
                    m_pOverviewCh5 = new WOverview("[Channel5]", m_pSamplerWindow);
                m_pOverviewCh5->setup(samplerNode);
                m_pOverviewCh5->setParent(m_pSamplerWindow);
        		m_pOverviewCh5->show();
            } else if (WWidget::selectNodeInt(samplerNode, "Channel")==6)
            {
                    if (m_pOverviewCh6 == 0)
                        m_pOverviewCh6 = new WOverview("[Channel6]", m_pSamplerWindow);
                    m_pOverviewCh6->setup(samplerNode);
                    m_pOverviewCh6->setParent(m_pSamplerWindow);
                    m_pOverviewCh6->show();
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
    Sampler* pSampler2 = m_pSamplerManager->getSampler(2);
    Sampler* pSampler3 = m_pSamplerManager->getSampler(3);
    Sampler* pSampler4 = m_pSamplerManager->getSampler(4);
    
    connect(pSampler1, SIGNAL(newTrackLoaded(TrackInfoObject*)),
        m_pOverviewCh3, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    connect(pSampler1, SIGNAL(unloadingTrack(TrackInfoObject*)),
        m_pOverviewCh3, SLOT(slotUnloadTrack(TrackInfoObject*)));
        
    connect(pSampler2, SIGNAL(newTrackLoaded(TrackInfoObject*)),
        m_pOverviewCh4, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    connect(pSampler2, SIGNAL(unloadingTrack(TrackInfoObject*)),
        m_pOverviewCh4, SLOT(slotUnloadTrack(TrackInfoObject*)));

    connect(pSampler3, SIGNAL(newTrackLoaded(TrackInfoObject*)),
        m_pOverviewCh5, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    connect(pSampler3, SIGNAL(unloadingTrack(TrackInfoObject*)),
        m_pOverviewCh5, SLOT(slotUnloadTrack(TrackInfoObject*)));
        
    connect(pSampler4, SIGNAL(newTrackLoaded(TrackInfoObject*)),
        m_pOverviewCh6, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    connect(pSampler4, SIGNAL(unloadingTrack(TrackInfoObject*)),
        m_pOverviewCh6, SLOT(slotUnloadTrack(TrackInfoObject*)));
                      
    connect(pSampler1, SIGNAL(newTrackLoaded(TrackInfoObject*)),
     		this, SLOT(slotSetupTrackConnectionsCh3(TrackInfoObject*)));
    connect(pSampler2, SIGNAL(newTrackLoaded(TrackInfoObject*)),
     		this, SLOT(slotSetupTrackConnectionsCh4(TrackInfoObject*)));
    connect(pSampler3, SIGNAL(newTrackLoaded(TrackInfoObject*)),
     		this, SLOT(slotSetupTrackConnectionsCh5(TrackInfoObject*)));
    connect(pSampler4, SIGNAL(newTrackLoaded(TrackInfoObject*)),
     		this, SLOT(slotSetupTrackConnectionsCh6(TrackInfoObject*)));
     		
    m_pSamplerWindow->show();
}

void WSampler::slotSaveSamplerBank() {
    QString s = QFileDialog::getSaveFileName(this, tr("Save Sampler Bank"));
    QFile file(s);
    if(!file.open(IO_WriteOnly)) {
        qDebug("Cannot write to file.");
    };
    QDomDocument doc("SamplerBank");
    
    QDomElement root = doc.createElement("samplerbank");
    doc.appendChild(root);
    
    QDomElement sampler1 = doc.createElement( "sampler1" );
    QString loc1 = m_pSamplerManager->getTrackLocation(1);
    sampler1.setAttribute( "location", loc1);
    root.appendChild(sampler1);
    
    QDomElement sampler2 = doc.createElement( "sampler2" );
    QString loc2 = m_pSamplerManager->getTrackLocation(2);
    sampler2.setAttribute( "location", loc2);
    root.appendChild(sampler2);
    
    QDomElement sampler3 = doc.createElement( "sampler3" );
    QString loc3 = m_pSamplerManager->getTrackLocation(3);
    sampler3.setAttribute( "location", loc3);
    root.appendChild(sampler3);
    
    QDomElement sampler4 = doc.createElement( "sampler4" );
    QString loc4 = m_pSamplerManager->getTrackLocation(4);
    sampler4.setAttribute( "location", loc4);
    root.appendChild(sampler4);
    
    file.write(doc.toString());
}

void WSampler::slotLoadSamplerBank() {
    QString s = QFileDialog::getOpenFileName(this, tr("Load Sampler Bank"));
    QFile file(s);
    if(!file.open(IO_ReadOnly)) {
        qDebug("Cannot read file.");
    };
    QDomDocument doc;
    doc.setContent(file.readAll());
    
    QDomElement root = doc.documentElement();
    if(root.tagName() != "samplerbank")
        return;
    
    QDomNode n = root.firstChild();
    qDebug() << n.nodeName();
    while(!n.isNull()) {
        qDebug("In Loop");
        QDomElement e = n.toElement();
        if(!e.isNull()) {
            qDebug() << e.tagName();
            if(e.tagName() == "sampler1") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 1);
            } else if(e.tagName() == "sampler2") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 2);
            } else if(e.tagName() == "sampler3") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 3);
            } else if(e.tagName() == "sampler4") {
                QString location = e.attribute("location", "");
                qDebug() << location;
                TrackInfoObject* loadTrack = new TrackInfoObject(location);
                m_pSamplerManager->slotLoadTrackToSampler(loadTrack, 4);
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

void WSampler::slotSetupTrackConnectionsCh4(TrackInfoObject* pTrack) {
    connect(pTrack, SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
		m_pOverviewCh4, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
}

void WSampler::slotSetupTrackConnectionsCh5(TrackInfoObject* pTrack) {
    connect(pTrack, SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
		m_pOverviewCh5, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
}

void WSampler::slotSetupTrackConnectionsCh6(TrackInfoObject* pTrack) {
    connect(pTrack, SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
		m_pOverviewCh6, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
}