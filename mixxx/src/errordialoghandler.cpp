/***************************************************************************
                          errordialoghandler.cpp  -  description
                             -------------------
    begin                : Sun Feb 22 2009
    copyright            : (C) 2009 by Sean M. Pappalardo
    email                : pegasus@c64.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "errordialoghandler.h"
#include <QMessageBox>
#include <QtDebug>
#include <QtCore>

ErrorDialogProperties::ErrorDialogProperties() {
    m_type = DLG_NONE;
    m_icon = QMessageBox::NoIcon;
    m_title = "Mixxx";
    m_modal = true;
}

void ErrorDialogProperties::setTitle(QString title) {
    m_title.append(" - ").append(title);
}

void ErrorDialogProperties::setText(QString text) {
    // If no key is set, use this window text since it is likely to be unique
    if (m_key.isEmpty()) m_key = text;
    m_text = text;
}

void ErrorDialogProperties::setType(DialogType typeToSet) {
    m_type = typeToSet;
    switch (m_type) {
        case DLG_FATAL:     // Fatal uses critical icon
        case DLG_CRITICAL:  m_icon = QMessageBox::Critical; break;
        case DLG_WARNING:   m_icon = QMessageBox::Warning; break;
        case DLG_INFO:      m_icon = QMessageBox::Information; break;
        case DLG_QUESTION:  m_icon = QMessageBox::Question; break;
        case DLG_NONE:
        default:
            // default is NoIcon
            break;
    }
}

void ErrorDialogProperties::addButton(QMessageBox::StandardButton button) {
    m_buttons.append(button);
}


// ----------------------------------------------------
// ---------- ErrorDialogHandler begins here ----------

ErrorDialogHandler* ErrorDialogHandler::s_pInstance = NULL;

ErrorDialogHandler::ErrorDialogHandler() {
    m_pSignalMapper = new QSignalMapper(this);
    connect(m_pSignalMapper, SIGNAL(mapped(QString)), this, SLOT(boxClosed(QString)));

    m_errorCondition=false;
    connect(this, SIGNAL(showErrorDialog(ErrorDialogProperties*)),
            this, SLOT(errorDialog(ErrorDialogProperties*)));
}

ErrorDialogHandler::~ErrorDialogHandler()
{
    delete m_pSignalMapper;
    delete s_pInstance;
}

ErrorDialogProperties* ErrorDialogHandler::newDialogProperties() {
    ErrorDialogProperties* props = new ErrorDialogProperties();
    return props;
}

bool ErrorDialogHandler::requestErrorDialog(DialogType type, QString message) {
    ErrorDialogProperties* props = newDialogProperties();
    props->setType(type);
    props->setText(message);
    switch (type) {
        case DLG_FATAL:     props->setTitle("Fatal error"); break;
        case DLG_CRITICAL:  props->setTitle("Critical error"); break;
        case DLG_WARNING:   props->setTitle("Warning"); break;
        case DLG_INFO:      props->setTitle("Information"); break;
        case DLG_QUESTION:  props->setTitle("Question"); break;
        case DLG_NONE:
        default:
            // Default title & (lack of) icon is fine
            break;
    }
    return requestErrorDialog(props);
}

bool ErrorDialogHandler::requestErrorDialog(ErrorDialogProperties* props) {

    // Make sure the minimum items are set
    QString text = props->getText();
    Q_ASSERT(!text.isEmpty());

    // Skip if a dialog with the same key is already displayed
    m_mutex.lock();
    bool keyExists = m_dialogKeys.contains(props->getKey());
    m_mutex.unlock();
    if (keyExists) {
        ErrorDialogProperties* dlgPropsTemp = props;
        props = NULL;
        delete dlgPropsTemp;
        return false;
    }

    emit (showErrorDialog(props));
    return true;
}

void ErrorDialogHandler::errorDialog(ErrorDialogProperties* props) {

    // Jest makin' sho' this is only run in the main (GUI) thread
    Q_ASSERT(QThread::currentThread()->objectName() == "Main");

    QMessageBox* msgBox = new QMessageBox();

    msgBox->setIcon(props->m_icon);
    msgBox->setWindowTitle(props->m_title);
    msgBox->setText(props->m_text);
    if (!props->m_infoText.isEmpty()) msgBox->setInformativeText(props->m_infoText);
    if (!props->m_details.isEmpty()) msgBox->setDetailedText(props->m_details);

    QPushButton* buttonToSet;
    bool setDefault;
    while(!props->m_buttons.isEmpty()) {
        setDefault = false;
        if (props->m_buttons.first() == props->m_defaultButton) setDefault = true;

        buttonToSet = msgBox->addButton(props->m_buttons.takeFirst());

        if (setDefault) msgBox->setDefaultButton(buttonToSet);
    }

    if (props->m_escapeButton) msgBox->setEscapeButton(msgBox->button(props->m_escapeButton));

    msgBox->setModal(props->m_modal);

    // This deletes the msgBox automatically, avoiding a memory leak
    msgBox->setAttribute(Qt::WA_DeleteOnClose, true);

    m_mutex.lock();
    m_dialogKeys.append(props->m_key);    // To avoid duplicate dialogs on the same error
    m_mutex.unlock();

    // Signal mapper calls our slot with the key parameter so it knows which to remove from the list
    connect(msgBox, SIGNAL(finished(int)), m_pSignalMapper, SLOT(map()));
    m_pSignalMapper->setMapping(msgBox, props->m_key);

    if (props->m_modal) msgBox->exec();    // Blocks so the user has a chance to read it before application exit
    else msgBox->show();

    // If fatal, should we just abort here and not try to exit gracefully?

    if (props->m_type>=DLG_CRITICAL) {  // If critical/fatal, gracefully exit application if possible
        m_errorCondition=true;
        if (QCoreApplication::instance()) {
            QCoreApplication::instance()->exit(-1);
        }
        else {
            qDebug() << "QCoreApplication::instance() is NULL! Abruptly quitting...";
            if (props->m_type==DLG_FATAL) abort();
            else exit(-1);
        }
    }

    ErrorDialogProperties* dlgPropsTemp = props;
    props = NULL;
    delete dlgPropsTemp;
}

void ErrorDialogHandler::boxClosed(QString key)
{
    QMessageBox* msgBox = (QMessageBox*)m_pSignalMapper->mapping(key);

    QMessageBox::StandardButton whichStdButton = msgBox->standardButton(msgBox->clickedButton());

    emit stdButtonClicked(key, whichStdButton);

    // If the user clicks "Ignore," we leave the key in the list so the same
    //  error is not displayed again for the duration of the session
    if (whichStdButton == QMessageBox::Ignore) {
        qWarning() << "Suppressing this" << msgBox->windowTitle() << "error box for the duration of the application.";
        return;
    }

    m_mutex.lock();
    if (m_dialogKeys.contains(key)) {
        if (!m_dialogKeys.removeOne(key)) qWarning() << "Error dialog key removal from list failed!";
    }
    else qWarning() << "Error dialog key is missing from key list!";
    m_mutex.unlock();
}

bool ErrorDialogHandler::checkError() {
    return m_errorCondition;
}
