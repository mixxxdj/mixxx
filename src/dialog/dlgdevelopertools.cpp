#include "dialog/dlgdevelopertools.h"

#include <QDateTime>
#include <QDir>
#include <QKeyEvent>

#include "control/control.h"
#include "moc_dlgdevelopertools.cpp"
#include "util/logging.h"
#include "util/statsmanager.h"

DlgDeveloperTools::DlgDeveloperTools(QWidget* pParent,
                                     UserSettingsPointer pConfig)
        : QDialog(pParent),
          m_pConfig(pConfig) {
    setupUi(this);

    controlsTable->setModel(&m_controlProxyModel);
    controlsTable->hideColumn(ControlModel::CONTROL_COLUMN_TITLE);
    controlsTable->hideColumn(ControlModel::CONTROL_COLUMN_DESCRIPTION);
    controlsTable->hideColumn(ControlModel::CONTROL_COLUMN_FILTER);
    m_controlProxyModel.sort(0, Qt::AscendingOrder);

    StatsManager* pManager = StatsManager::instance();
    if (pManager) {
        connect(pManager,
                &StatsManager::statUpdated,
                &m_statModel,
                &StatModel::statUpdated);
        pManager->emitAllStats();
    }

    m_statProxyModel.setSourceModel(&m_statModel);
    statsTable->setModel(&m_statProxyModel);

    QString logFileName = QDir(pConfig->getSettingsPath()).filePath("mixxx.log");
    m_logFile.setFileName(logFileName);
    if (!m_logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ERROR: Could not open log file:" << logFileName;
    }

    // Set up the control search
    m_controlProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(controlSearch,
            &WSearchLineEdit::search,
            this,
            &DlgDeveloperTools::slotControlSearch);
    connect(controlDump,
            &QPushButton::clicked,
            this,
            &DlgDeveloperTools::slotControlDump);

    // Set up the log search box
    connect(logSearch,
            &QLineEdit::returnPressed,
            this,
            &DlgDeveloperTools::slotLogSearch);
    connect(logSearchButton,
            &QPushButton::clicked,
            this,
            &DlgDeveloperTools::slotLogSearch);

    m_logCursor = logTextView->textCursor();

    // Update at 2FPS.
    startTimer(500);

    // Delete this dialog when its closed. We don't want any persistence.
    setAttribute(Qt::WA_DeleteOnClose);

    // Just to catch Ctrl+F to focus searchbar
    installEventFilter(this);
}

void DlgDeveloperTools::timerEvent(QTimerEvent* pEvent) {
    Q_UNUSED(pEvent);
    if (!isVisible()) {
        // nothing to do if we are not visible
        return;
    }

    // To save on CPU, only update the models when they are visible.
    if (toolTabWidget->currentWidget() == logTab) {
        if (m_logFile.isOpen()) {
            // ensure, everything is in Buffer.
            mixxx::Logging::flushLogFile();

            QStringList newLines;

            while (true) {
                QByteArray line = m_logFile.readLine();
                if (line.isEmpty()) {
                    break;
                }
                newLines.append(QString::fromLocal8Bit(line));
            }

            if (!newLines.isEmpty()) {
                logTextView->append(newLines.join(""));
            }
        }
    } else if (toolTabWidget->currentWidget() == statsTab) {
        StatsManager* pManager = StatsManager::instance();
        if (pManager) {
            pManager->updateStats();
        }
    }
}

void DlgDeveloperTools::slotControlSearch(const QString& search) {
    QStringList words = search.split(' ');
    if (words.size() > 1) {
        QString combo = words.join(QStringLiteral(".*"));
        m_controlProxyModel.setFilterRegularExpression(combo);
    } else {
        m_controlProxyModel.setFilterFixedString(search);
    }
}

void DlgDeveloperTools::slotControlDump() {

    QString timestamp = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd_hh'h'mm'm'ss's'");
    QString dumpFileName = m_pConfig->getSettingsPath() +
            "/co_dump_" + timestamp + ".csv";
    QFile dumpFile;
    // Note: QFile is closed if it falls out of scope
    dumpFile.setFileName(dumpFileName);
    if (!dumpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "open" << dumpFileName << "failed";
        return;
    }

    const QList<QSharedPointer<ControlDoublePrivate>> controlsList =
            ControlDoublePrivate::getAllInstances();
    for (auto it = controlsList.constBegin(); it != controlsList.constEnd(); ++it) {
        const QSharedPointer<ControlDoublePrivate>& pControl = *it;
        if (pControl) {
            QString line = pControl->getKey().group + "," +
                           pControl->getKey().item + "," +
                           QString::number(pControl->get()) + "\n";
            dumpFile.write(line.toLocal8Bit());
        }
    }
}

void DlgDeveloperTools::slotLogSearch() {
    QString textToFind = logSearch->text();
    m_logCursor = logTextView->document()->find(textToFind, m_logCursor);
    logTextView->setTextCursor(m_logCursor);
}

bool DlgDeveloperTools::eventFilter(QObject* pObj, QEvent* pEvent) {
    if (pEvent->type() == QEvent::ShortcutOverride) {
        QKeyEvent* pKE = static_cast<QKeyEvent*>(pEvent);
        VERIFY_OR_DEBUG_ASSERT(pKE) {
            return QDialog::eventFilter(pObj, pEvent);
        }

        if (pKE->key() == Qt::Key_F && (pKE->modifiers() & Qt::ControlModifier)) {
            controlSearch->setFocus(Qt::ShortcutFocusReason);
            pEvent->accept();
            return true;
        }
    }
    return QDialog::eventFilter(pObj, pEvent);
}
