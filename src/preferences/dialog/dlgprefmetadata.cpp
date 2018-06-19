#include "preferences/dialog/dlgprefmetadata.h"

#include <QCheckBox>
#include <QLinkedList>
#include <QTableWidgetItem>

#include "broadcast/listenersfinder.h"
#include "preferences/dialog/dlgprefmetadata.h"
#include "preferences/dialog/ui_dlgfilelistenerbox.h"
#include "moc_dlgprefmetadata.cpp"

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent, UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          fileListenerBox(new Ui::fileListenerBox) {
    setupUi(this);
    setupTableWidget(pSettings);
}

void DlgPrefMetadata::setupTableWidget(UserSettingsPointer pSettings) {
    Q_UNUSED(pSettings);
}

DlgPrefMetadata::~DlgPrefMetadata() {
    delete fileListenerBox;
}

void DlgPrefMetadata::slotCurrentListenerChanged(
        const QModelIndex& previous,
        const QModelIndex& current) {
    Q_UNUSED(previous);
    Q_UNUSED(current);
}

void DlgPrefMetadata::slotApply() {
}

void DlgPrefMetadata::slotCancel() {
}

void DlgPrefMetadata::slotResetToDefaults() {
}

void DlgPrefMetadata::slotUpdate() {
}

