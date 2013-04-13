/***************************************************************************
                          dlgprefbpm.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qlineedit.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <QtCore>
#include <QMessageBox>

#include "dlgprefbpm.h"
#include "dlgbpmscheme.h"
#include "bpm/bpmscheme.h"
#include "xmlparse.h"
#include "mixxx.h"

#define CONFIG_KEY "[BPM]"

DlgPrefBpm::DlgPrefBpm(QWidget * parent, ConfigObject<ConfigValue> * _config)
        : QWidget(parent) {
    config = _config;

    setupUi(this);

    // Connection
    connect(chkDetectOnImport,      SIGNAL(stateChanged(int)), this, SLOT(slotSetBpmDetectOnImport(int)));
    connect(chkWriteID3,            SIGNAL(stateChanged(int)), this, SLOT(slotSetWriteID3Tag(int)));
    connect(chkEnableBpmDetection,  SIGNAL(stateChanged(int)), this, SLOT(slotSetBpmEnabled(int)));
    connect(chkAboveRange,          SIGNAL(stateChanged(int)), this, SLOT(slotSetAboveRange(int)));

    // TODO: Move this over the the scheme dialog

    connect(btnAdd,        SIGNAL(pressed()),         this, SLOT(slotAddBpmScheme()));
    connect(btnEdit,       SIGNAL(pressed()),         this, SLOT(slotEditBpmScheme()));
    connect(btnDelete,     SIGNAL(pressed()),         this, SLOT(slotDeleteBpmScheme()));
    connect(btnDefault,    SIGNAL(pressed()),         this, SLOT(slotDefaultBpmScheme()));


    // Determine if the config value has already been set. If not, default to enabled
    QString sBpmEnabled = config->getValueString(ConfigKey(CONFIG_KEY,"BPMDetectionEnabled"));
    if(sBpmEnabled.isNull() || sBpmEnabled.isEmpty())
    {
        config->set(ConfigKey(CONFIG_KEY,"BPMDetectionEnabled"), ConfigValue(1));
    }

    // Set default value for analyze mode check box
    int iBpmEnabled = config->getValueString(ConfigKey(CONFIG_KEY,"BPMDetectionEnabled")).toInt();
    if (iBpmEnabled)
        chkEnableBpmDetection->setChecked(true);
    else
        chkEnableBpmDetection->setChecked(false);

    int iBpmAboveRange = config->getValueString(ConfigKey(CONFIG_KEY,"BPMAboveRangeEnabled")).toInt();
    if (iBpmAboveRange)
        chkAboveRange->setChecked(true);
    else
        chkAboveRange->setChecked(false);

    // Set default value for detect BPM on import check box
    int iDetectBpmOnImport = config->getValueString(ConfigKey(CONFIG_KEY,"DetectBPMOnImport")).toInt();
    if (iDetectBpmOnImport)
        chkDetectOnImport->setChecked(true);
    else
        chkDetectOnImport->setChecked(false);

    // Set default value for write ID3 tag check box
    int iWriteID3Tag = config->getValueString(ConfigKey(CONFIG_KEY,"WriteID3Tag")).toInt();
    if (iWriteID3Tag)
        chkWriteID3->setChecked(true);
    else
        chkWriteID3->setChecked(false);

    chkWriteID3->setEnabled(false);
    chkDetectOnImport->setEnabled(false);

    // Load the BPM schemes
    loadBpmSchemes();
    populateBpmSchemeList();

    updateBpmEnabled();


    //Load BPM Range Values
    /*int iRangeStart = config->getValueString(ConfigKey("[BPM]","BPMRangeStart")).toInt();
    if(iRangeStart > 0 && iRangeStart <= 220)
        spinBoxBPMRangeStart->setValue(iRangeStart);
    else
        spinBoxBPMRangeStart->setValue(60);

    int iRangeEnd = config->getValueString(ConfigKey("[BPM]","BPMRangeEnd")).toInt();
    if(iRangeEnd > 0 && iRangeEnd <=220)
        spinBoxBPMRangeEnd->setValue(iRangeEnd);
    else
        spinBoxBPMRangeEnd->setValue(180);*/

}

DlgPrefBpm::~DlgPrefBpm()
{
    saveBpmSchemes();

    while (!m_BpmSchemes.isEmpty())
    {
        delete m_BpmSchemes.takeFirst();
    }
}

void DlgPrefBpm::slotSetBpmDetectOnImport(int)
{
    if (chkDetectOnImport->isChecked())
        config->set(ConfigKey(CONFIG_KEY,"DetectBPMOnImport"), ConfigValue(1));
    else
        config->set(ConfigKey(CONFIG_KEY,"DetectBPMOnImport"), ConfigValue(0));
}

void DlgPrefBpm::slotSetWriteID3Tag(int)
{
    if (chkWriteID3->isChecked())
        config->set(ConfigKey(CONFIG_KEY,"WriteID3Tag"), ConfigValue(1));
    else
        config->set(ConfigKey(CONFIG_KEY,"WriteID3Tag"), ConfigValue(0));
}

void DlgPrefBpm::slotSetBpmEnabled(int)
{
    if (chkEnableBpmDetection->isChecked())
        config->set(ConfigKey(CONFIG_KEY,"BPMDetectionEnabled"), ConfigValue(1));
    else
        config->set(ConfigKey(CONFIG_KEY,"BPMDetectionEnabled"), ConfigValue(0));

    updateBpmEnabled();

}

void DlgPrefBpm::slotSetAboveRange(int) {
    if (chkAboveRange->isChecked())
        config->set(ConfigKey(CONFIG_KEY,"BPMAboveRangeEnabled"), ConfigValue(1));
    else
        config->set(ConfigKey(CONFIG_KEY,"BPMAboveRangeEnabled"), ConfigValue(0));
}

void DlgPrefBpm::slotSetBpmRangeStart(int begin)
{
    Q_UNUSED(begin);
    //config->set(ConfigKey("[BPM]","BPMRangeStart"),ConfigValue(begin));
}

void DlgPrefBpm::slotSetBpmRangeEnd(int end)
{
    Q_UNUSED(end);
    //config->set(ConfigKey("[BPM]","BPMRangeEnd"),ConfigValue(end));
}

void DlgPrefBpm::slotEditBpmScheme()
{
    int row = lstSchemes->currentRow();

    if(row > -1)
    {
        BpmScheme *schemeToEdit = m_BpmSchemes.at(row);
        QString oldname = schemeToEdit->getName();

        // Open the BPM scheme dialog to edit
        DlgBpmScheme* SchemeEdit = new DlgBpmScheme(schemeToEdit);
        SchemeEdit->setModal(true);
        SchemeEdit->exec();

        QListWidgetItem *item = lstSchemes->item(row);
        item->setText(schemeToEdit->getName());

        if(oldname == config->getValueString(ConfigKey("[BPM]","DefaultScheme")))
        {
            config->set(ConfigKey("[BPM]","DefaultScheme"), schemeToEdit->getName());
        }
    }
}

void DlgPrefBpm::slotAddBpmScheme()
{
    BpmScheme *schemeToAdd = NULL;

    // Open the BPM scheme dialog to add
    DlgBpmScheme* SchemeEdit = new DlgBpmScheme(schemeToAdd);
    SchemeEdit->setModal(true);

    if(SchemeEdit->exec() == QDialog::Accepted)
    {
        if(schemeToAdd)
        {
            m_BpmSchemes.push_back(schemeToAdd);
            QListWidgetItem *addScheme = new QListWidgetItem(lstSchemes);
            addScheme->setText(schemeToAdd->getName());
        }
    }
    else
    {
        delete schemeToAdd;
    }


}

void DlgPrefBpm::slotDeleteBpmScheme()
{
    int row = lstSchemes->currentRow();

    if(row > -1)
    {
        qDebug() << "Removing Bpm Scheme at position " << row;
        delete lstSchemes->takeItem(row);
        m_BpmSchemes.removeAt(row);
    }
}

void DlgPrefBpm::slotDefaultBpmScheme()
{
    int row = lstSchemes->currentRow();

    if(row > -1)
    {
        BpmScheme* scheme = m_BpmSchemes.at(row);

        config->set(ConfigKey("[BPM]","BPMRangeEnd"),ConfigValue(scheme->getMaxBpm()));
        config->set(ConfigKey("[BPM]","BPMRangeStart"),ConfigValue(scheme->getMinBpm()));
        config->set(ConfigKey("[BPM]","AnalyzeEntireSong"),ConfigValue(scheme->getAnalyzeEntireSong()));
        config->set(ConfigKey("[BPM]","DefaultScheme"), scheme->getName());

        clearListIcons();

        QListWidgetItem *item = lstSchemes->item(row);
        item->setIcon(QIcon(":/images/preferences/ic_preferences_bpmdetect.png"));
    }
}

void DlgPrefBpm::clearListIcons()
{
    for(int i=0; i < lstSchemes->count(); ++i)
    {
        lstSchemes->item(i)->setIcon(QIcon(""));
    }
}

void DlgPrefBpm::slotApply()
{
    saveBpmSchemes();
}

void DlgPrefBpm::slotUpdate()
{
}

void DlgPrefBpm::updateBpmEnabled()
{
    int iBpmEnabled = config->getValueString(ConfigKey(CONFIG_KEY,"BPMDetectionEnabled")).toInt();
    if (iBpmEnabled)
    {
        chkDetectOnImport->setEnabled(true);
        chkWriteID3->setEnabled(true);
        chkAboveRange->setEnabled(true);
        grpBpmSchemes->setEnabled(true);
    }
    else
    {
        chkDetectOnImport->setEnabled(false);
        chkWriteID3->setEnabled(false);
        chkAboveRange->setEnabled(false);
        grpBpmSchemes->setEnabled(false);
    }

    // These are not implemented yet, so don't enable them
    chkDetectOnImport->setEnabled(false);
    chkWriteID3->setEnabled(false);

}

void DlgPrefBpm::loadBpmSchemes()
{
    // Verify path for xml track file.
    QString schemeFileName = config->getValueString(ConfigKey("[BPM]","SchemeFile"));
    if (schemeFileName.trimmed().isEmpty() | !QFile(schemeFileName).exists() ) {
        schemeFileName = CmdlineArgs::Instance().getSettingsPath() + BPMSCHEME_FILE;
        qDebug() << "BPM Scheme File ConfigKey not set or file missing... setting to"<< schemeFileName;
        config->set(ConfigKey("[BPM]","SchemeFile"), schemeFileName);
        config->Save();
    }

    QString location(config->getValueString(ConfigKey("[BPM]","SchemeFile")));
    qDebug() << "BpmSchemes::readXML" << location;

    // Open XML file
    QFile file(location);

    // Check if we can open the file
    if (!file.exists())
    {
        qDebug() << "BPM Scheme:" << location <<  "does not exist.";
        file.close();
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "BPM Scheme:" << location <<  "can't open file for reading.";
        return;
    }

    QByteArray fileData = file.readAll();
    QByteArray badHeader = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-16\"?>");
    QByteArray goodHeader = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

    // We've been writing UTF-16 as the encoding forever but actually writing
    // the file in UTF-8 (well, latin1 actually). Qt seems to have started
    // caring recently. Manually fix the header if we are dealing with an old
    // file.
    fileData.replace(badHeader, goodHeader);

    QDomDocument domXML("Mixxx_BPM_Scheme_List");



    // Check if there is a parsing problem
    QString error_msg;
    int error_line;
    int error_column;
    if (!domXML.setContent(fileData, &error_msg, &error_line, &error_column))
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

void DlgPrefBpm::saveBpmSchemes()
{
    QString location(config->getValueString(ConfigKey("[BPM]","SchemeFile")));

    // Create the xml document:
    QDomDocument domXML( "Mixxx_BPM_Scheme_List" );

    // Ensure UTF16 encoding
    domXML.appendChild(domXML.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\""));

    // Set the document type
    QDomElement elementRoot = domXML.createElement( "Mixxx_BPM_Scheme_List" );
    domXML.appendChild(elementRoot);

    // Add version information:
    //XmlParse::addElement(domXML, elementRoot, "Version", QString("%1").arg(TRACK_VERSION));

    // Write playlists
    QDomElement schemesroot = domXML.createElement("Schemes");

    QListIterator<BpmScheme*> it(m_BpmSchemes);
    BpmScheme* current;
    while (it.hasNext())
    {
        current = it.next();

        QDomElement elementNew = domXML.createElement("Scheme");
        current->writeXML(domXML, elementNew);
        schemesroot.appendChild(elementNew);

    }
    elementRoot.appendChild(schemesroot);

    // Open the file:
    QFile opmlFile(location);
    if (!opmlFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(0,
                              tr("Error"),
                              tr("Cannot open file %1").arg(location));
        return;
    }
    // QByteArray encoded in UTF-8
    QByteArray ba = domXML.toByteArray();
    opmlFile.write(ba.constData(), ba.size());
    opmlFile.close();
}

void DlgPrefBpm::populateBpmSchemeList()
{
    QString defaultscheme = config->getValueString(ConfigKey("[BPM]","DefaultScheme"));

    for(int i=0; i < m_BpmSchemes.size(); ++i)
    {
        QListWidgetItem* scheme = new QListWidgetItem(lstSchemes);
        scheme->setText(m_BpmSchemes.at(i)->getName());
        if(m_BpmSchemes.at(i)->getName() == defaultscheme)
        {
            scheme->setIcon(QIcon(":/images/preferences/ic_preferences_bpmdetect.png"));
        }
    }
}
