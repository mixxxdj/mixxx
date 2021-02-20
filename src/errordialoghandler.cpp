#include "errordialoghandler.h"

#include <QCoreApplication>
#include <QMutexLocker>
#include <QScopedPointer>
#include <QThread>
#include <QtDebug>

#include "moc_errordialoghandler.cpp"
#include "util/assert.h"
#include "util/version.h"

ErrorDialogProperties::ErrorDialogProperties()
        : m_title(Version::applicationName()),
          m_detailsUseMonospaceFont(false),
          m_modal(true),
          m_shouldQuit(false),
          m_type(DLG_NONE),
          m_icon(QMessageBox::NoIcon),
          m_defaultButton(QMessageBox::NoButton),
          m_escapeButton(QMessageBox::NoButton) {
}

void ErrorDialogProperties::setTitle(const QString& title) {
    m_title.append(" - ").append(title);
}

void ErrorDialogProperties::setText(const QString& text) {
    // If no key is set, use this window text since it is likely to be unique
    if (m_key.isEmpty()) {
        m_key = text;
    }
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

ErrorDialogHandler* ErrorDialogHandler::s_pInstance = nullptr;
bool ErrorDialogHandler::s_bEnabled = true;

// static
void ErrorDialogHandler::setEnabled(bool enabled) {
    s_bEnabled = enabled;
}

ErrorDialogHandler::ErrorDialogHandler() {
    m_errorCondition = false;
    connect(this, &ErrorDialogHandler::showErrorDialog, this, &ErrorDialogHandler::errorDialog);
}

ErrorDialogHandler::~ErrorDialogHandler() {
    s_pInstance = nullptr;
}

ErrorDialogProperties* ErrorDialogHandler::newDialogProperties() {
    return new ErrorDialogProperties();
}

bool ErrorDialogHandler::requestErrorDialog(
        DialogType type, const QString& message, bool shouldQuit) {
    if (!s_bEnabled) {
        return false;
    }
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
    if (!s_bEnabled) {
        delete props;
        return false;
    }

    // Make sure the minimum items are set
    QString text = props->getText();
    VERIFY_OR_DEBUG_ASSERT(!text.isEmpty()) {
        delete props;
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

    emit showErrorDialog(props);
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

    QMessageBox* pMsgBox = new QMessageBox();
    pMsgBox->setIcon(props->m_icon);
    pMsgBox->setWindowTitle(props->m_title);
    pMsgBox->setText(props->m_text);
    if (!props->m_infoText.isEmpty()) {
        pMsgBox->setInformativeText(props->m_infoText);
    }
    if (!props->m_details.isEmpty()) {
        pMsgBox->setDetailedText(props->m_details);
        if (props->m_detailsUseMonospaceFont) {
            pMsgBox->setStyleSheet("QTextEdit { font-family: monospace; }");
        }
    }

    while (!props->m_buttons.isEmpty()) {
        pMsgBox->addButton(props->m_buttons.takeFirst());
    }
    pMsgBox->setDefaultButton(props->m_defaultButton);
    pMsgBox->setEscapeButton(props->m_escapeButton);
    pMsgBox->setModal(props->m_modal);

    // This deletes the msgBox automatically, avoiding a memory leak
    pMsgBox->setAttribute(Qt::WA_DeleteOnClose, true);

    QMutexLocker locker(&m_mutex);
    // To avoid duplicate dialogs on the same error
    m_dialogKeys.append(props->m_key);

    // Signal mapper calls our slot with the key parameter so it knows which to
    // remove from the list
    QString key = props->m_key;
    connect(pMsgBox,
            &QMessageBox::finished,
            this,
            [this, key, pMsgBox] { boxClosed(key, pMsgBox); });

    locker.unlock();

    if (props->m_modal) {
        // Blocks so the user has a chance to read it before application exit
        pMsgBox->exec();
    } else {
        pMsgBox->show();
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

void ErrorDialogHandler::boxClosed(const QString& key, QMessageBox* msgBox) {
    QMutexLocker locker(&m_mutex);
    locker.unlock();

    QMessageBox::StandardButton whichStdButton = msgBox->standardButton(msgBox->clickedButton());
    emit stdButtonClicked(key, whichStdButton);

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
