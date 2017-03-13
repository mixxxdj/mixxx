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

#include <QCoreApplication>
#include <QMutexLocker>
#include <QScopedPointer>
#include <QThread>
#include <QtDebug>

#include "errordialoghandler.h"
#include "util/assert.h"

ErrorDialogProperties::ErrorDialogProperties()
        : m_title("Mixxx"),
          m_modal(true),
          m_shouldQuit(false),
          m_type(DLG_NONE),
          m_icon(QMessageBox::NoIcon) {
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

ErrorDialogHandler::ErrorDialogHandler()
        : m_signalMapper(this) {
    connect(&m_signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(boxClosed(QString)));

    m_errorCondition = false;
    connect(this, SIGNAL(showErrorDialog(ErrorDialogProperties*)),
            this, SLOT(errorDialog(ErrorDialogProperties*)));
}

ErrorDialogHandler::~ErrorDialogHandler() {
    s_pInstance = NULL;
}

ErrorDialogProperties* ErrorDialogHandler::newDialogProperties() {
    return new ErrorDialogProperties();
}

bool ErrorDialogHandler::requestErrorDialog(DialogType type, QString message,
                                            bool shouldQuit) {
    ErrorDialogProperties* props = newDialogProperties();
    props->setType(type);
    props->setText(message);
    if (shouldQuit) {
        props->setShouldQuit(shouldQuit);
    }
    switch (type) {
        case DLG_FATAL:     props->setTitle(tr("Fatal error")); break;
        case DLG_CRITICAL:  props->setTitle(tr("Critical error")); break;
        case DLG_WARNING:   props->setTitle(tr("Warning")); break;
        case DLG_INFO:      props->setTitle(tr("Information")); break;
        case DLG_QUESTION:  props->setTitle(tr("Question")); break;
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
    DEBUG_ASSERT_AND_HANDLE(!text.isEmpty()) {
        return false;
    }

    // Skip if a dialog with the same key is already displayed
    QMutexLocker locker(&m_mutex);
    bool keyExists = m_dialogKeys.contains(props->getKey());
    locker.unlock();
    if (keyExists) {
        delete props;
        return false;
    }

    emit(showErrorDialog(props));
    return true;
}

void ErrorDialogHandler::errorDialog(ErrorDialogProperties* pProps) {
    QScopedPointer<ErrorDialogProperties> props(pProps);
    if (!props) {
        return;
    }

    // Check we are in the main thread.
    if (QThread::currentThread()->objectName() != "Main") {
        qWarning() << "WARNING: errorDialog not called in the main thread. Not showing error dialog.";
        return;
    }

    QMessageBox* msgBox = new QMessageBox();
    msgBox->setIcon(props->m_icon);
    msgBox->setWindowTitle(props->m_title);
    msgBox->setText(props->m_text);
    if (!props->m_infoText.isEmpty()) {
        msgBox->setInformativeText(props->m_infoText);
    }
    if (!props->m_details.isEmpty()) {
        msgBox->setDetailedText(props->m_details);
    }

    while (!props->m_buttons.isEmpty()) {
        msgBox->addButton(props->m_buttons.takeFirst());
    }
    msgBox->setDefaultButton(props->m_defaultButton);
    msgBox->setEscapeButton(props->m_escapeButton);
    msgBox->setModal(props->m_modal);

    // This deletes the msgBox automatically, avoiding a memory leak
    msgBox->setAttribute(Qt::WA_DeleteOnClose, true);

    QMutexLocker locker(&m_mutex);
    // To avoid duplicate dialogs on the same error
    m_dialogKeys.append(props->m_key);

    // Signal mapper calls our slot with the key parameter so it knows which to
    // remove from the list
    connect(msgBox, SIGNAL(finished(int)),
            &m_signalMapper, SLOT(map()));
    m_signalMapper.setMapping(msgBox, props->m_key);

    locker.unlock();

    if (props->m_modal) {
        // Blocks so the user has a chance to read it before application exit
        msgBox->exec();
    } else {
        msgBox->show();
    }

    // If critical/fatal, gracefully exit application if possible
    if (props->m_shouldQuit) {
        m_errorCondition = true;
        if (QCoreApplication::instance()) {
            QCoreApplication::instance()->exit(-1);
        } else {
            qDebug() << "QCoreApplication::instance() is NULL! Abruptly quitting...";
            if (props->m_type==DLG_FATAL) {
                abort();
            } else {
                exit(-1);
            }
        }
    }
}

void ErrorDialogHandler::boxClosed(QString key) {
    QMutexLocker locker(&m_mutex);
    QMessageBox* msgBox = (QMessageBox*)m_signalMapper.mapping(key);
    locker.unlock();

    QMessageBox::StandardButton whichStdButton = msgBox->standardButton(msgBox->clickedButton());
    emit(stdButtonClicked(key, whichStdButton));

    // If the user clicks "Ignore," we leave the key in the list so the same
    // error is not displayed again for the duration of the session
    if (whichStdButton == QMessageBox::Ignore) {
        qWarning() << "Suppressing this" << msgBox->windowTitle()
                   << "error box for the duration of the application.";
        return;
    }

    QMutexLocker locker2(&m_mutex);
    if (m_dialogKeys.contains(key)) {
        if (!m_dialogKeys.removeOne(key)) {
            qWarning() << "Error dialog key removal from list failed!";
        }
    } else {
        qWarning() << "Error dialog key is missing from key list!";
    }
}

bool ErrorDialogHandler::checkError() {
    return m_errorCondition;
}
