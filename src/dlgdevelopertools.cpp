#include "dlgdevelopertools.h"

#include "control/control.h"
#include "util/cmdlineargs.h"
#include "util/statsmanager.h"

DlgDeveloperTools::DlgDeveloperTools(QWidget* pParent,
                                     ConfigObject<ConfigValue>* pConfig)
        : QDialog(pParent) {
    Q_UNUSED(pConfig);
    setupUi(this);

    QList<QSharedPointer<ControlDoublePrivate> > controlsList;
    ControlDoublePrivate::getControls(&controlsList);
    QHash<ConfigKey, ConfigKey> controlAliases =
            ControlDoublePrivate::getControlAliases();

    for (QList<QSharedPointer<ControlDoublePrivate> >::const_iterator it = controlsList.begin();
            it != controlsList.end(); ++it) {
        const QSharedPointer<ControlDoublePrivate>& pControl = *it;
        if (pControl) {
            m_controlModel.addControl(pControl->getKey(), pControl->name(),
                                      pControl->description());

            ConfigKey aliasKey = controlAliases[pControl->getKey()];
            if (!aliasKey.isNull()) {
                m_controlModel.addControl(aliasKey, pControl->name(),
                                          "Alias for " + pControl->getKey().group + pControl->getKey().item);
            }
        }
    }

    m_controlProxyModel.setSourceModel(&m_controlModel);
    m_controlProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_controlProxyModel.setFilterKeyColumn(ControlModel::CONTROL_COLUMN_FILTER);
    controlsTable->setModel(&m_controlProxyModel);
    controlsTable->hideColumn(ControlModel::CONTROL_COLUMN_TITLE);
    controlsTable->hideColumn(ControlModel::CONTROL_COLUMN_DESCRIPTION);
    controlsTable->hideColumn(ControlModel::CONTROL_COLUMN_FILTER);

    StatsManager* pManager = StatsManager::instance();
    if (pManager) {
        connect(pManager, SIGNAL(statUpdated(const Stat&)),
                &m_statModel, SLOT(statUpdated(const Stat&)));
        pManager->emitAllStats();
    }

    m_statProxyModel.setSourceModel(&m_statModel);
    statsTable->setModel(&m_statProxyModel);

    QString logFileName = CmdlineArgs::Instance().getSettingsPath() + "/mixxx.log";
    m_logFile.setFileName(logFileName);
    m_logFile.open(QIODevice::ReadOnly | QIODevice::Text);

    // Connect search box signals to the library
    connect(controlSearch, SIGNAL(search(const QString&)),
            this, SLOT(slotControlSearch(const QString&)));
    connect(controlSearch, SIGNAL(searchCleared()),
            this, SLOT(slotControlSearchClear()));

    // Set up the log search box
    connect(logSearch, SIGNAL(returnPressed()),
            this, SLOT(slotLogSearch()));
    connect(logSearchButton, SIGNAL(clicked()),
            this, SLOT(slotLogSearch()));

    m_logCursor = logTextView->textCursor();

    // Update at 2FPS.
    startTimer(500);

    // Delete this dialog when its closed. We don't want any persistence.
    setAttribute(Qt::WA_DeleteOnClose);
}

DlgDeveloperTools::~DlgDeveloperTools() {
}

void DlgDeveloperTools::timerEvent(QTimerEvent* pEvent) {
    Q_UNUSED(pEvent);
    if (m_logFile.isOpen()) {
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

    // To save on CPU, only update the models when they are visible.
    if (toolTabWidget->currentWidget() == controlsTab) {
        //m_controlModel.updateDirtyRows();
        controlsTable->update();
    } else if (toolTabWidget->currentWidget() == statsTab) {
        StatsManager* pManager = StatsManager::instance();
        if (pManager) {
            pManager->updateStats();
        }
    }
}

void DlgDeveloperTools::slotControlSearch(const QString& search) {
    m_controlProxyModel.setFilterFixedString(search);
}

void DlgDeveloperTools::slotControlSearchClear() {
    m_controlProxyModel.setFilterFixedString(QString());
}

void DlgDeveloperTools::slotLogSearch() {
    QString textToFind = logSearch->text();
    m_logCursor = logTextView->document()->find(textToFind, m_logCursor);
    logTextView->setTextCursor(m_logCursor);
}
