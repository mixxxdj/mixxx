#include "preferences/dialog/dlgprefmetadata.h"

#include <QCheckBox>
#include <QLinkedList>

#include "broadcast/listenersfinder.h"

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent, UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent) {
    setupUi(this);
    QLinkedList<ScrobblingServicePtr> listeners =
            ListenersFinder::instance(pSettings).getAllServices();
    listenersTableWidget->setColumnCount(2);
    listenersTableWidget->setRowCount(listeners.size());
    QStringList headerLabels = {"Enabled", "Name"};
    listenersTableWidget->setHorizontalHeaderLabels(headerLabels);
    listenersTableWidget->verticalHeader()->setVisible(false);
    listenersTableWidget->setShowGrid(false);
    listenersTableWidget->horizontalHeader()->setStretchLastSection(true);
    unsigned int currentRow = 0;
    for (ScrobblingServicePtr pService : listeners) {
        QTableWidgetItem* enabledItem = new QTableWidgetItem;
        enabledItem->setFlags(Qt::NoItemFlags);
        listenersTableWidget->setItem(currentRow, 0, enabledItem);
        listenersTableWidget->setCellWidget(currentRow, 0, new QCheckBox);
        QTableWidgetItem* nameItem = new QTableWidgetItem(pService->getName());
        nameItem->setFlags(Qt::ItemIsSelectable);
        listenersTableWidget->setItem(currentRow, 1, nameItem);
        ++currentRow;
    }
}

void DlgPrefMetadata::slotApply() {
}

void DlgPrefMetadata::slotCancel() {
}

void DlgPrefMetadata::slotResetToDefaults() {
}

void DlgPrefMetadata::slotUpdate() {
}
