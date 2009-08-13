/***************************************************************************
                          dlgpreferences.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgbpmtap.h"

#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <q3progressbar.h>
#include <qspinbox.h>
#include <qdatetime.h>
//Added by qt3to4:
#include <QEvent>
#include <QtGui>

#include "mixxx.h"
#include "trackinfoobject.h"
#include "bpm/bpmscheme.h"
#include "analyserqueue.h"

#include "xmlparse.h"

DlgBpmTap::DlgBpmTap(QWidget *, TrackInfoObject * tio, 
                    TrackPlaylist * playlist, ConfigObject<ConfigValue> *_config):
                    QDialog(), Ui::DlgBpmTapDlg()
{
    // m_pMixxx = mixxx;
    m_pAnalyserQueue = AnalyserQueue::createBPMAnalyserQueue(_config);
    connect(m_pAnalyserQueue,
            SIGNAL(trackFinished(TrackInfoObject*)),
            this,
            SLOT(slotComplete(TrackInfoObject*)));
    m_CurrentTrack = tio;
    m_TrackPlaylist = playlist;
    config = _config;

    // This must be called before setFocus or setEnabled.
    setupUi(this);

    //Give focus to the tap button so that the tempo can be tapped with
    //the space bar
    btnTap->setFocus();

    //Create time object
    m_Time = new QTime(0,0);
    m_TapCount = 0;

    progressBPMDetect->setEnabled(true);
    progressBPMDetect->setMaximum(100);
    progressBPMDetect->setMinimum(0);
    btnTap->setEnabled(true);

    spinBoxBPMRangeStart->setEnabled(true);
    spinBoxBPMRangeEnd->setEnabled(true);
    btnGo->setEnabled(true);

    //spinBoxBPMRangeStart->setValue(tio->getMinBpm());
    //spinBoxBPMRangeEnd->setValue(tio->getMaxBpm());

    loadTrackInfo();
    toolbox->setCurrentIndex(0);


    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // Connections
    connect(this,        SIGNAL(aboutToShow()),          this,      SLOT(slotLoadDialog()));
    connect(this,        SIGNAL(closeDlg()),             this,      SLOT(slotApply()));
    connect(btnTap,      SIGNAL(clicked()),              this,      SLOT(slotTapBPM()));
    connect(btnGo,       SIGNAL(clicked()),              this,      SLOT(slotDetectBPM()));
    connect(btnOK,       SIGNAL(clicked()),              this,      SLOT(slotOK()));
    connect(btnNext,     SIGNAL(clicked()),              this,      SLOT(slotNext()));
    connect(btnPrev,     SIGNAL(clicked()),              this,      SLOT(slotPrev()));

    connect(txtBPM,         SIGNAL(textChanged(const QString &)), this, SLOT(slotBpmChanged(const QString &)));
    connect(txtTrackName,   SIGNAL(textChanged(const QString &)), this, SLOT(slotTitleChanged(const QString &)));
    connect(txtArtist,      SIGNAL(textChanged(const QString &)), this, SLOT(slotArtistChanged(const QString &)));
    connect(txtComment,     SIGNAL(textChanged()),                this, SLOT(slotCommentChanged()));


    connect(spinBoxBPMRangeStart,   SIGNAL(valueChanged(int)),   this,   SLOT(slotUpdateMinBpm(int)));
    connect(spinBoxBPMRangeEnd,     SIGNAL(valueChanged(int)),   this,   SLOT(slotUpdateMaxBpm(int)));
    
    connect(cboSchemes, SIGNAL(currentIndexChanged(int)), this, SLOT(slotBpmSchemeChanged(int)));
    
    loadBpmSchemes();
    populateBpmSchemeList();

}

DlgBpmTap::~DlgBpmTap()
{
    while (!m_BpmSchemes.isEmpty())
    {
        delete m_BpmSchemes.takeFirst();
    }

    delete m_Time;
}

void DlgBpmTap::loadTrackInfo()
{
    lblSong->setText(m_CurrentTrack->getTitle());
    txtBPM->setText(QString("%1").arg(m_CurrentTrack->getBpm(), 3,'f',1));

    txtTrackName->setText(m_CurrentTrack->getTitle());
    txtDuration->setText(m_CurrentTrack->getDurationStr());
    txtFilepath->setText(m_CurrentTrack->getFilename());
    txtFilepath->setCursorPosition(0);
    txtType->setText(m_CurrentTrack->getType());
    txtArtist->setText(m_CurrentTrack->getArtist());
    txtComment->setText(m_CurrentTrack->getComment());
}

bool DlgBpmTap::eventFilter(QObject * o, QEvent * e)
{
    // Send a close signal if dialog is closing
    /*
       //FIXME: These are borked with QT4 (linker error, wtf?)
       if (e->type() == QEvent::Hide)
        emit(closeDlg());

       if(e->type() == QEvent::Show)
        emit(aboutToShow());
     */
    // Standard event processing
    return QWidget::eventFilter(o,e);
}

void DlgBpmTap::slotTapBPM()
{
    if(btnTap->text() != "Detecting BPM...")
    {
        if(m_Time->elapsed() > 2000)
        {
            m_TapCount = 0;
        }

        if(m_TapCount <=0)
        {
            m_Time->restart();
        }

        if(m_TapCount > 0)
        {
            float elapsedTime = m_Time->elapsed() / (float)60000;

            float bpm = (float)m_TapCount / (float)elapsedTime;
            m_CurrentTrack->setBpm(bpm);
            txtBPM->setText(QString("%1").arg(bpm, 3,'f',1));
        }

        m_TapCount += 1;
    }
}

void DlgBpmTap::slotDetectBPM()
{
    progressBPMDetect->setValue(0);
    progressBPMDetect->setMinimum(0);
    progressBPMDetect->setMaximum(0);
    btnTap->setText("Detecting BPM...");
    //btnTap->setEnabled(false);
    
    BpmScheme *scheme = new BpmScheme();
    
    scheme->setMinBpm(spinBoxBPMRangeStart->value());
    scheme->setMaxBpm(spinBoxBPMRangeEnd->value());
    scheme->setAnalyzeEntireSong(chkAnalyzeEntireSong->isChecked());
    
    //m_CurrentTrack->setBpmConfirm(false);
    //m_CurrentTrack->sendToBpmQueue(this, scheme);

    m_pAnalyserQueue->start();
    m_pAnalyserQueue->queueAnalyseTrack(m_CurrentTrack);
    
}

void DlgBpmTap::slotLoadDialog()
{
    txtBPM->setText(m_CurrentTrack->getBpmStr());
}

void DlgBpmTap::slotOK()
{
    //m_CurrentTrack->setBpm(txtBPM->text().toFloat());
    setHidden(true);
}

void DlgBpmTap::slotNext()
{
    if(m_TrackPlaylist)
    {
        //FIXME
        TrackInfoObject * track = NULL;//m_CurrentTrack->getNext(m_TrackPlaylist);

        if(track)
        {
            m_CurrentTrack = track;
            loadTrackInfo();
        }
    }
}

void DlgBpmTap::slotPrev()
{

    if(m_TrackPlaylist)
    {
        //FIXME
        TrackInfoObject * track = NULL;// m_CurrentTrack->getPrev(m_TrackPlaylist);

        if(track)
        {
            m_CurrentTrack = track;
            loadTrackInfo();

        }
    }
}

void DlgBpmTap::slotUpdateMinBpm(int i)
{
    //m_CurrentTrack->setMinBpm((float)i);
}

void DlgBpmTap::slotUpdateMaxBpm(int i)
{
    //m_CurrentTrack->setMaxBpm((float)i);
}

void DlgBpmTap::slotBpmChanged(const QString & bpm)
{
    m_CurrentTrack->setBpm(bpm.toFloat());
}

void DlgBpmTap::slotTitleChanged(const QString & title)
{
    m_CurrentTrack->setTitle(title);
}

void DlgBpmTap::slotArtistChanged(const QString & artist)
{
    m_CurrentTrack->setArtist(artist);
}

void DlgBpmTap::slotCommentChanged()
{
    m_CurrentTrack->setComment(txtComment->toPlainText());
}

void DlgBpmTap::slotBpmSchemeChanged(int ndx)
{
    if(ndx < m_BpmSchemes.size() && ndx > -1)
    {
        BpmScheme* scheme = m_BpmSchemes.at(ndx);
        chkAnalyzeEntireSong->setChecked(scheme->getAnalyzeEntireSong());
        spinBoxBPMRangeStart->setValue(scheme->getMinBpm());
        spinBoxBPMRangeEnd->setValue(scheme->getMaxBpm());
    }
}

void DlgBpmTap::slotUpdate()
{
}

void DlgBpmTap::slotApply()
{
//    m_pMixxx->grabKeyboard();
}

void DlgBpmTap::setProgress(TrackInfoObject * tio, int progress)
{
// txtBPM->setText(QString("%1").arg(progress));
}

void DlgBpmTap::setComplete(TrackInfoObject * tio, bool failed, float returnBpm)
{
    progressBPMDetect->setMaximum(100);
    progressBPMDetect->setValue(0);
    //progressBPMDetect->reset();
    btnTap->setText("&Push to tap tempo");
    //btnTap->setEnabled(true);
    txtBPM->setText(QString("%1").arg(returnBpm, 3,'f',1));
    this->update();
}

void DlgBpmTap::slotComplete(TrackInfoObject *tio) {
    qDebug() << "DlgBpmTap got complete signal";
    if(tio != m_CurrentTrack)
        return;

    m_pAnalyserQueue->stop();
    float bpm = tio->getBpm();
    
    progressBPMDetect->setMaximum(100);
    progressBPMDetect->setValue(0);
    //progressBPMDetect->reset();
    btnTap->setText("&Push to tap tempo");
    //btnTap->setEnabled(true);
    txtBPM->setText(QString("%1").arg(bpm, 3,'f',1));
    this->update();
    
}

void DlgBpmTap::loadBpmSchemes()
{
    // Verify path for xml track file.
    QString schemeFileName = config->getValueString(ConfigKey("[BPM]","SchemeFile"));
    if (schemeFileName.trimmed().isEmpty() || !QFile(schemeFileName).exists()) {
        schemeFileName = QDir::homePath().append("/").append(SETTINGS_PATH).append(BPMSCHEME_FILE);
        qDebug() << "BPM Scheme File ConfigKey not set or file missing... setting to"<< schemeFileName;
        config->set(ConfigKey("[BPM]","SchemeFile"), schemeFileName);
        config->Save();
    }
    QFile scheme(schemeFileName);
    if (scheme.exists())
    {   
        QString location(config->getValueString(ConfigKey("[BPM]","SchemeFile")));
        qDebug() << "BpmSchemes::readXML" << location;
       
        // Open XML file
        QFile file(location);
        QDomDocument domXML("Mixxx_BPM_Scheme_List");

        // Check if we can open the file
        if (!file.exists())
        {
            qDebug() << "BPM Scheme:" << location <<  "does not exist.";
            file.close();
            return;
        }

        // Check if there is a parsing problem
        QString error_msg;
        int error_line;
        int error_column;
        if (!domXML.setContent(&file, &error_msg, &error_line, &error_column))
        {
            qDebug() << "BPM Scheme Parse error in" << location;
            qDebug() << "Doctype:" << domXML.doctype().name();
            qDebug() << error_msg << "on line" << error_line << ", column" << error_column;
            file.close();
            return;
        }

        file.close();

        // Get the root element
        QDomElement elementRoot = domXML.documentElement();

        // Get version
        //int version = XmlParse::selectNodeInt(elementRoot, "Version");

        // Get all the BPM schemes written in the xml file:
        QDomNode node = XmlParse::selectNode(elementRoot, "Schemes").firstChild();
        BpmScheme* bpmScheme; //Current BPM Scheme
        while (!node.isNull())
        {
            if (node.isElement() && node.nodeName()=="Scheme")
            {
                bpmScheme = new BpmScheme();
                //Create the playlists internally.
                //If the playlist is "Library" or "Play Queue", insert it into
                //a special spot in the list of playlists.
                bpmScheme->setName(XmlParse::selectNodeQString(node, "Name"));
                bpmScheme->setMinBpm(XmlParse::selectNodeQString(node, "MinBpm").toInt());
                bpmScheme->setMaxBpm(XmlParse::selectNodeQString(node, "MaxBpm").toInt());
                bpmScheme->setAnalyzeEntireSong((bool)XmlParse::selectNodeQString(node, 
                                                            "AnalyzeEntireSong").toInt());
                bpmScheme->setComment(XmlParse::selectNodeQString(node, "Comment"));
                
                m_BpmSchemes.push_back(bpmScheme);          
            }       

            node = node.nextSibling();
        }
    }
    
    if(m_BpmSchemes.size() == 0)
    {
        BpmScheme *scheme = new BpmScheme("Default", 70, 140, false);
        m_BpmSchemes.push_back(scheme);
        config->set(ConfigKey("[BPM]","DefaultScheme"), QString("Default"));
        config->set(ConfigKey("[BPM]","BPMRangeEnd"),ConfigValue(scheme->getMaxBpm()));
        config->set(ConfigKey("[BPM]","BPMRangeStart"),ConfigValue(scheme->getMinBpm()));
        config->set(ConfigKey("[BPM]","AnalyzeEntireSong"),ConfigValue(scheme->getAnalyzeEntireSong()));
        
    }
}

void DlgBpmTap::populateBpmSchemeList()
{
    QString defaultscheme = config->getValueString(ConfigKey("[BPM]","DefaultScheme"));
    m_DefaultScheme = 0;

    for(int i=0; i < m_BpmSchemes.size(); ++i)
    {
        cboSchemes->addItem(m_BpmSchemes.at(i)->getName());

        if(m_BpmSchemes.at(i)->getName() == defaultscheme)
        {
            m_DefaultScheme = i;
        } 
    }
    
    cboSchemes->setCurrentIndex(m_DefaultScheme);
}

