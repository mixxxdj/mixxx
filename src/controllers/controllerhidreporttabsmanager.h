#pragma once

#include <QPointer>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QTableWidget>
#include <memory>

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hidreportdescriptor.h"

class ControllerHidReportTabsManager : public QObject {
    Q_OBJECT

  public:
    ControllerHidReportTabsManager(QTabWidget* pParentTabWidget, HidController* pHidController);

    void createReportTypeTabs();
    void createHidReportTab(QTabWidget* pParentTab,
            hid::reportDescriptor::HidReportType reportType);
    void slotSendReport(QTableWidget* pTable,
            quint8 reportId,
            hid::reportDescriptor::HidReportType reportType);
    void populateHidReportTable(QTableWidget* pTable,
            const hid::reportDescriptor::Report& report,
            hid::reportDescriptor::HidReportType reportType);

  private slots:
    void slotReadReport(QTableWidget* pTable,
            quint8 reportId,
            hid::reportDescriptor::HidReportType reportType);

  public slots:
    void slotProcessInputReport(quint8 reportId, const QByteArray& reportData);

  private:
    void updateTableWithReportData(QTableWidget* pTable, const QByteArray& reportData);
    QPointer<QTabWidget> m_pParentControllerTab;
    QPointer<HidController> m_pHidController;
    std::unordered_map<quint8, QPointer<QTableWidget>> m_reportIdToTableMap;
};

class ValueItemDelegate : public QStyledItemDelegate {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* pParent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    void setModelData(QWidget* pEditor,
            QAbstractItemModel* pModel,
            const QModelIndex& index) const override;
};
