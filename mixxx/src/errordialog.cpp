/***************************************************************************
                          errordialog.cpp  -  description
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

#include "errordialog.h"
#include <QMessageBox>
#include <QtDebug>
#include <QtCore>

DialogProperties::DialogProperties() {
    type = DLG_NONE;
    icon = QMessageBox::NoIcon;
    m_title = "Mixxx";
    modal = true;
}

void DialogProperties::setTitle(QString title) {
    m_title.append(" - ").append(title);
}

QString DialogProperties::getTitle() {
    return m_title;
}

void DialogProperties::setType(DialogType typeToSet) {
    type = typeToSet;
    switch (type) {
        case DLG_FATAL:     // Fatal uses critical icon
        case DLG_CRITICAL:  icon = QMessageBox::Critical; break;
        case DLG_WARNING:   icon = QMessageBox::Warning; break;
        case DLG_INFO:      icon = QMessageBox::Information; break;
        case DLG_QUESTION:  icon = QMessageBox::Question; break;
        case DLG_NONE:
        default:
            // default is NoIcon
            break;
    }
}

// ErrorDialog begins here
ErrorDialog::ErrorDialog() {
    m_pSignalMapper = new QSignalMapper(this);
    connect(m_pSignalMapper, SIGNAL(mapped(QString)), this, SLOT(boxClosed(QString)));
    
    m_errorCondition=false;
    connect(this, SIGNAL(showErrorDialog(DialogProperties*)),
            this, SLOT(errorDialog(DialogProperties*)));
}

ErrorDialog::~ErrorDialog()
{
    delete m_pSignalMapper;
}

bool ErrorDialog::requestErrorDialog(DialogType type, QString message) {
    DialogProperties* props = new DialogProperties();
    props->setType(type);
    props->text = message;
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

bool ErrorDialog::requestErrorDialog(DialogProperties* props) {
    // If no key was set, use the window text since it is likely to be unique
    if (props->key.isEmpty()) props->key = props->text;
    
    // Skip if a dialog with the same key is already displayed
    if (m_dialogKeys.contains(props->key)) {
        DialogProperties* dlgPropsTemp = props;
        props = NULL;
        delete dlgPropsTemp;
        return false;
    }
    
    emit (showErrorDialog(props));
    return true;
}

void ErrorDialog::errorDialog(DialogProperties* props) {
    
    QMessageBox* msgBox = new QMessageBox();
    
    msgBox->setIcon(props->icon);
    msgBox->setWindowTitle(props->getTitle());
    msgBox->setText(props->text);
    if (!props->infoText.isEmpty()) msgBox->setInformativeText(props->infoText);
    if (!props->details.isEmpty()) msgBox->setDetailedText(props->details);
    
    QPushButton* buttonToSet;
    while(!props->buttons.isEmpty()) {
        if ((props->buttons.first() != props->defaultButton) &&
            (props->buttons.first() != props->escapeButton))
            // If this isn't the default or escape button, we don't care about the returned pointer
            msgBox->addButton(props->buttons.takeFirst());
        else
            // If this is, save the returned pointer to set it to the default and/or escape buttons
            buttonToSet = msgBox->addButton(props->buttons.takeFirst());
    }
    if (buttonToSet == (QPushButton*)msgBox->button(props->defaultButton)) {
        msgBox->setDefaultButton(buttonToSet);
    }
    
    // This is currently ineffective since the GUI takes over Esc at all times,
    //  which shouldn't be, See https://bugs.launchpad.net/mixxx/+bug/565793
    //  Once that's fixed, this should suddenly work.
    if (buttonToSet == (QPushButton*)msgBox->button(props->escapeButton)) {
        msgBox->setEscapeButton(msgBox->button(props->escapeButton));
    }
    
    msgBox->setModal(props->modal);
    
    // This deletes the msgBox automatically, avoiding a memory leak
    msgBox->setAttribute(Qt::WA_DeleteOnClose, true);
    
    m_dialogKeys.append(props->key);    // To avoid duplicate dialogs on the same error
    
    // Signal mapper calls our slot with the key parameter so it knows which to remove from the list
    connect(msgBox, SIGNAL(finished(int)), m_pSignalMapper, SLOT(map()));
    m_pSignalMapper->setMapping(msgBox, props->key);

    if (props->modal) msgBox->exec();    // Blocks so the user has a chance to read it before application exit
    else msgBox->show();

    // If fatal, should we just abort here and not try to exit gracefully?
    
    if (props->type>=DLG_CRITICAL) {  // If critical/fatal, gracefully exit application if possible
        m_errorCondition=true;
        if (QCoreApplication::instance()) {
            QCoreApplication::instance()->exit(-1);
        }
        else {
            qDebug() << "QCoreApplication::instance() is NULL! Abruptly quitting...";
            if (props->type==DLG_FATAL) abort();
            else exit(-1);
        }
    }
    
    DialogProperties* dlgPropsTemp = props;
    props = NULL;
    delete dlgPropsTemp;
}

void ErrorDialog::boxClosed(QString key)
{
    QMessageBox* msgBox = (QMessageBox*)m_pSignalMapper->mapping(key);

    QMessageBox::StandardButton whichStdButton = msgBox->standardButton(msgBox->clickedButton());
    
    emit stdButtonClicked(key, whichStdButton);
    
    // If the user clicks "Ignore," we leave the key in the list so the same
    //  error is not displayed again for the duration of the session
    if (whichStdButton == QMessageBox::Ignore) {
        qDebug() << "Suppressing this" << msgBox->windowTitle() << "error box for the duration of the application.";
        return;
    }
    
    if (m_dialogKeys.contains(key)) {
        if (!m_dialogKeys.removeOne(key)) qWarning() << "Key removal from list failed!";
    }
    else qWarning() << "Key is missing from key list!";
}

bool ErrorDialog::checkError() {
    return m_errorCondition;
}