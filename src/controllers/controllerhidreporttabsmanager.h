#pragma once

#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QTableWidget>
#include <memory>

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hidreportdescriptor.h"

class ControllerHidReportTabsManager : public QObject {
    Q_OBJECT

  public:
    ControllerHidReportTabsManager(QTabWidget* parentTabWidget, HidController* hidController);

    void createReportTypeTabs();
    void createHidReportTab(QTabWidget* parentTab, hid::reportDescriptor::HidReportType reportType);
    void slotSendReport(QTableWidget* table,
            quint8 reportId,
            hid::reportDescriptor::HidReportType reportType);
    void populateHidReportTable(QTableWidget* table,
            const hid::reportDescriptor::Report& report,
            hid::reportDescriptor::HidReportType reportType);

  private slots:
    void slotReadReport(QTableWidget* table,
            quint8 reportId,
            hid::reportDescriptor::HidReportType reportType);

  public slots:
    void slotProcessInputReport(quint8 reportId, const QByteArray& reportData);

  private:
    void updateTableWithReportData(QTableWidget* table, const QByteArray& reportData);
    QTabWidget* m_pParentControllerTab;
    HidController* m_pHidController;
    std::unordered_map<quint8, QTableWidget*> m_reportIdToTableMap;
};

class ValueItemDelegate : public QStyledItemDelegate {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

    void setModelData(QWidget* editor,
            QAbstractItemModel* model,
            const QModelIndex& index) const override;
};
