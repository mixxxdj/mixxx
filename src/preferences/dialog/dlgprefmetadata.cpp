#include <QLinkedList>
#include <QCheckBox>
#include "broadcast/listenersfinder.h"
#include "preferences/dialog/dlgprefmetadata.h"
#include "preferences/dialog/ui_dlgfilelistenerbox.h"

DlgPrefMetadata::DlgPrefMetadata(QWidget *pParent,UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          fileListenerBox(new Ui::fileListenerBox){
    setupUi(this);
    setupTableWidget(pSettings);
}

void DlgPrefMetadata::setupTableWidget(UserSettingsPointer pSettings) {
    QLinkedList<ScrobblingServicePtr> listeners =
            ListenersFinder::instance(pSettings).getAllServices();
    setTableParameters(listeners.size()+5);
    fillTableWithServices(listeners);
}

DlgPrefMetadata::~DlgPrefMetadata() {
    delete fileListenerBox;
}

void DlgPrefMetadata::setTableParameters(int numberOfListeners) {
    listenersTableWidget->setColumnCount(2);
    listenersTableWidget->setRowCount(numberOfListeners);
    QStringList headerLabels = {"Enabled","Name"};
    listenersTableWidget->setHorizontalHeaderLabels(headerLabels);
    listenersTableWidget->verticalHeader()->setVisible(false);
    listenersTableWidget->setShowGrid(false);
    listenersTableWidget->horizontalHeader()->setStretchLastSection(true);
    /*connect(listenersTableWidget->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this,SLOT(slotCurrentListenerChanged(const QModelIndex&,const QModelIndex&)));*/
}

void DlgPrefMetadata::fillTableWithServices(const QLinkedList<ScrobblingServicePtr> &listeners) {
    unsigned int currentRow = 0;
    for (const ScrobblingServicePtr &pService : listeners) {
        QTableWidgetItem *enabledItem = new QTableWidgetItem;
        enabledItem->setFlags(Qt::NoItemFlags);
        listenersTableWidget->setItem(currentRow,0,enabledItem);
        listenersTableWidget->setCellWidget(currentRow,0,new QCheckBox);
        QTableWidgetItem *nameItem = new QTableWidgetItem(pService->getName());
        nameItem->setFlags(Qt::ItemIsSelectable);
        listenersTableWidget->setItem(currentRow,1,nameItem);
        ++currentRow;
    }
    for (int i = listeners.size(); i < listeners.size() + 5; ++i) {
        QTableWidgetItem *enabledItem = new QTableWidgetItem;
        enabledItem->setFlags(Qt::NoItemFlags);
        listenersTableWidget->setItem(i,0,enabledItem);
        listenersTableWidget->setCellWidget(i,0,new QCheckBox);
        QTableWidgetItem *nameItem = new QTableWidgetItem("Mock");
        nameItem->setFlags(Qt::ItemIsSelectable);
        listenersTableWidget->setItem(i,1,nameItem);
    }
}

void DlgPrefMetadata::slotCurrentListenerChanged
        (const QModelIndex &previous, const QModelIndex &current) {
    listenersTableWidget->selectionModel()->select(previous,QItemSelectionModel::Clear);
}


