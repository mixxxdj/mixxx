#include "controllerhidreporttabsmanager.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMetaEnum>
#include <QPushButton>
#include <QVBoxLayout>

#include "controllers/hid/hidusagetables.h"
#include "moc_controllerhidreporttabsmanager.cpp"
#include "util/parented_ptr.h"

namespace {
QTableWidgetItem* createReadOnlyItem(const QString& text, bool rightAlign = false) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    if (rightAlign) {
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    return item;
}

QTableWidgetItem* createValueItem(
        hid::reportDescriptor::HidReportType reportType,
        int minVal,
        int maxVal) {
    auto* item = new QTableWidgetItem;
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);
    if (reportType == hid::reportDescriptor::HidReportType::Input) {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    } else {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setData(Qt::UserRole, QVariant::fromValue(QPair<int, int>(minVal, maxVal)));
    }
    return item;
}
} // anonymous namespace

ControllerHidReportTabsManager::ControllerHidReportTabsManager(
        QTabWidget* pParentTabWidget, HidController* pHidController)
        : m_pParentControllerTab(pParentTabWidget),
          m_pHidController(pHidController) {
}

void ControllerHidReportTabsManager::createReportTypeTabs() {
    VERIFY_OR_DEBUG_ASSERT(m_pParentControllerTab) {
        return;
    }
    auto reportTypeTabs = make_parented<QTabWidget>(m_pParentControllerTab);

    QMetaEnum metaEnum = QMetaEnum::fromType<hid::reportDescriptor::HidReportType>();

    for (int reportTypeIdx = 0; reportTypeIdx < metaEnum.keyCount(); ++reportTypeIdx) {
        auto reportType = static_cast<hid::reportDescriptor::HidReportType>(
                metaEnum.value(reportTypeIdx));
        auto reportTypeTab = make_parented<QTabWidget>(reportTypeTabs.get());
        createHidReportTab(reportTypeTab.get(), reportType);
        if (reportTypeTab->count() > 0) {
            QString tabName = QStringLiteral("%1 Reports")
                                      .arg(metaEnum.key(reportTypeIdx));
            m_pParentControllerTab->addTab(std::move(reportTypeTab), tabName);
        }
    }
}

void ControllerHidReportTabsManager::createHidReportTab(QTabWidget* pParentReportTypeTab,
        hid::reportDescriptor::HidReportType reportType) {
    VERIFY_OR_DEBUG_ASSERT(m_pHidController) {
        return;
    }
    auto reportDescriptor = m_pHidController->getReportDescriptor();
    if (!reportDescriptor) {
        return;
    }

    QMetaEnum metaEnum = QMetaEnum::fromType<hid::reportDescriptor::HidReportType>();

    for (const auto& reportInfo : reportDescriptor->getListOfReports()) {
        auto [index, type, reportId] = reportInfo;
        if (type == reportType) {
            // Report is a fixed HID term and shouldn't be translated
            QString tabName = QStringLiteral("%1 Report 0x%2")
                                      .arg(metaEnum.valueToKey(static_cast<int>(
                                                   reportType)),
                                              QString::number(reportId, 16)
                                                      .rightJustified(2, '0')
                                                      .toUpper());

            auto pTabWidget = make_parented<QWidget>(pParentReportTypeTab);
            auto pLayout = make_parented<QVBoxLayout>(pTabWidget);
            auto* pTopWidgetRow = new QHBoxLayout();

            // Create buttons
            auto pReadButton = make_parented<QPushButton>(tr("Read"), pTabWidget);
            auto pSendButton = make_parented<QPushButton>(tr("Send"), pTabWidget);

            // Adjust visibility/enable state based on the report type
            if (reportType == hid::reportDescriptor::HidReportType::Input) {
                pSendButton->hide();
                pReadButton->hide();
            } else if (reportType == hid::reportDescriptor::HidReportType::Output) {
                pReadButton->hide();
            }

            pTopWidgetRow->addWidget(pReadButton);
            pTopWidgetRow->addWidget(pSendButton);
            pLayout->addLayout(pTopWidgetRow);

            auto pTable = make_parented<QTableWidget>(pTabWidget);
            pLayout->addWidget(pTable);

            auto reportOpt = reportDescriptor->getReport(reportType, reportId);
            if (reportOpt) {
                const auto& report = reportOpt->get();
                // Show payload size
                auto pSizeLabel = make_parented<QLabel>(pTabWidget);
                pSizeLabel->setText(
                        QStringLiteral("%1: <b>%2</b> %3")
                                .arg(tr("Payload Size"))
                                .arg(report.getReportSize())
                                .arg(tr("bytes")));
                pTopWidgetRow->insertWidget(0, pSizeLabel);

                populateHidReportTable(pTable, report, reportType);
            }

            if (reportType != hid::reportDescriptor::HidReportType::Output) {
                connect(pReadButton,
                        &QPushButton::clicked,
                        this,
                        [this, pTable = pTable.get(), reportId, reportType]() {
                            slotReadReport(pTable, reportId, reportType);
                        });
                // Read once on tab creation
                slotReadReport(pTable, reportId, reportType);
            }
            if (reportType != hid::reportDescriptor::HidReportType::Input) {
                connect(pSendButton,
                        &QPushButton::clicked,
                        this,
                        [this, pTable = pTable.get(), reportId, reportType]() {
                            slotSendReport(pTable, reportId, reportType);
                        });
            }

            pParentReportTypeTab->addTab(pTabWidget, tabName);

            if (reportType == hid::reportDescriptor::HidReportType::Input) {
                // Store the pTable pointer associated with the reportId
                m_reportIdToTableMap[reportId] = pTable;
                // Ensure that the table entry gets deleted when the table is destroyed
                connect(pTable, &QObject::destroyed, this, [this, reportId]() {
                    m_reportIdToTableMap.erase(reportId);
                });
            }

            // Connect the signal for the reportId
            HidIoThread* hidIoThread = m_pHidController->getHidIoThread();
            if (hidIoThread) {
                connect(hidIoThread,
                        &HidIoThread::reportReceived,
                        this,
                        &ControllerHidReportTabsManager::slotProcessInputReport);
            }
        }
    }
}

void ControllerHidReportTabsManager::updateTableWithReportData(
        QTableWidget* pTable,
        const QByteArray& reportData) {
    // Temporarily disable updates to speed up processing
    pTable->setUpdatesEnabled(false);

    // Process the report data and update the table
    for (int row = 0; row < pTable->rowCount(); ++row) {
        auto* pItem = pTable->item(row, 5); // Value column is at index 5
        if (pItem) {
            // Retrieve custom data from the first cell
            QVariant customData = pTable->item(row, 0)->data(Qt::UserRole + 1);
            if (customData.isValid()) {
                const auto* pControl = customData.value<const hid::reportDescriptor::Control*>();
                VERIFY_OR_DEBUG_ASSERT(pControl) {
                    continue;
                }
                // Use the custom data as needed
                int64_t controlValue =
                        hid::reportDescriptor::extractLogicalValue(
                                reportData, *pControl);
                pItem->setText(QString::number(controlValue));
            }
        }
    }

    pTable->setUpdatesEnabled(true);
}

void ControllerHidReportTabsManager::slotProcessInputReport(
        quint8 reportId, const QByteArray& data) {
    VERIFY_OR_DEBUG_ASSERT(m_pHidController) {
        return;
    }
    // Find the table associated with the reportId
    auto it = m_reportIdToTableMap.find(reportId);
    if (it == m_reportIdToTableMap.end()) {
        qWarning() << "No table found for reportId" << reportId;
        return;
    }
    QTableWidget* pTable = it->second;

    VERIFY_OR_DEBUG_ASSERT(pTable) {
        return;
    }

    auto reportDescriptor = m_pHidController->getReportDescriptor();
    if (!reportDescriptor) {
        return;
    }

    auto reportOpt = reportDescriptor->getReport(
            hid::reportDescriptor::HidReportType::Input, reportId);
    if (reportOpt) {
        updateTableWithReportData(pTable, data);
    }
}

void ControllerHidReportTabsManager::slotReadReport(QTableWidget* pTable,
        quint8 reportId,
        hid::reportDescriptor::HidReportType reportType) {
    VERIFY_OR_DEBUG_ASSERT(m_pHidController) {
        return;
    }
    if (!m_pHidController->isOpen()) {
        qWarning() << "HID controller is not open.";
        return;
    }

    HidControllerJSProxy* pJsProxy =
            static_cast<HidControllerJSProxy*>(m_pHidController->jsProxy());
    VERIFY_OR_DEBUG_ASSERT(pJsProxy) {
        return;
    }

    QByteArray reportData;
    if (reportType == hid::reportDescriptor::HidReportType::Input) {
        reportData = pJsProxy->getInputReport(reportId);
    } else if (reportType == hid::reportDescriptor::HidReportType::Feature) {
        reportData = pJsProxy->getFeatureReport(reportId);
    } else {
        return;
    }

    auto reportDescriptor = m_pHidController->getReportDescriptor();
    if (!reportDescriptor) {
        return;
    }

    auto reportOpt = reportDescriptor->getReport(reportType, reportId);
    VERIFY_OR_DEBUG_ASSERT(reportOpt) {
        return;
    }
    const auto& report = reportOpt->get();
    if (reportData.size() < report.getReportSize()) {
        qWarning() << "Failed to get report. Read only " << reportData.size()
                   << " instead of expected " << report.getReportSize()
                   << " bytes.";
        return;
    }

    updateTableWithReportData(pTable, reportData);
}

void ControllerHidReportTabsManager::slotSendReport(QTableWidget* pTable,
        quint8 reportId,
        hid::reportDescriptor::HidReportType reportType) {
    VERIFY_OR_DEBUG_ASSERT(m_pHidController) {
        return;
    }
    if (!m_pHidController->isOpen()) {
        qWarning() << "HID controller is not open.";
        return;
    }

    HidControllerJSProxy* pJsProxy =
            static_cast<HidControllerJSProxy*>(m_pHidController->jsProxy());
    VERIFY_OR_DEBUG_ASSERT(pJsProxy) {
        return;
    }

    auto reportDescriptor = m_pHidController->getReportDescriptor();
    if (!reportDescriptor) {
        return;
    }

    auto reportOpt = reportDescriptor->getReport(reportType, reportId);
    VERIFY_OR_DEBUG_ASSERT(reportOpt) {
        return;
    }
    const auto& report = reportOpt->get();

    // Create a QByteArray of the size of the report
    QByteArray reportData(report.getReportSize(), 0);

    // Iterate through each row in the table
    for (int row = 0; row < pTable->rowCount(); ++row) {
        auto* pItem = pTable->item(row, 5); // Value column is at index 5
        if (pItem) {
            // Retrieve custom data from the first cell
            QVariant customData = pTable->item(row, 0)->data(Qt::UserRole + 1);
            if (customData.isValid()) {
                const auto* pControl = customData.value<const hid::reportDescriptor::Control*>();
                VERIFY_OR_DEBUG_ASSERT(pControl) {
                    continue;
                }
                bool success = hid::reportDescriptor::applyLogicalValue(
                        reportData, *pControl, pItem->text().toLongLong());
                if (!success) {
                    qWarning() << "Failed to set control value for row" << row;
                    continue;
                }
            }
        }
    }

    // Send the reportData
    if (reportType == hid::reportDescriptor::HidReportType::Feature) {
        pJsProxy->sendFeatureReport(reportId, reportData);
    } else if (reportType == hid::reportDescriptor::HidReportType::Output) {
        pJsProxy->sendOutputReport(reportId, reportData);
    }
}

void ControllerHidReportTabsManager::populateHidReportTable(
        QTableWidget* pTable,
        const hid::reportDescriptor::Report& pReport,
        hid::reportDescriptor::HidReportType reportType) {
    // Temporarily disable updates to speed up populating
    pTable->setUpdatesEnabled(false);

    // Reserve rows up-front
    const auto& controls = pReport.getControls();
    pTable->setRowCount(static_cast<int>(controls.size()));

    // Set the delegate once if needed
    if (reportType != hid::reportDescriptor::HidReportType::Input) {
        pTable->setItemDelegateForColumn(5, make_parented<ValueItemDelegate>(pTable));
    }

    bool showVolatileColumn = (reportType == hid::reportDescriptor::HidReportType::Feature ||
            reportType == hid::reportDescriptor::HidReportType::Output);

    // Set headers
    QStringList headers = {tr("Byte Position"),
            tr("Bit Position"),
            tr("Bit Size"),
            tr("Logical Min"),
            tr("Logical Max"),
            tr("Value"),
            tr("Physical Min"),
            tr("Physical Max"),
            tr("Unit Scaling"),
            tr("Unit"),
            tr("Abs/Rel"),
            tr("Wrap"),
            tr("Linear"),
            tr("Preferred"),
            tr("Null")};
    if (showVolatileColumn) {
        headers << tr("Volatile");
    }
    headers << tr("Usage Page") << tr("Usage");

    pTable->setColumnCount(headers.size());
    pTable->setHorizontalHeaderLabels(headers);
    pTable->verticalHeader()->setVisible(false);

    int row = 0;
    for (const auto& control : controls) {
        // Column 0 - Byte Position
        QTableWidgetItem* pBytePositionItem = createReadOnlyItem(
                QStringLiteral("0x%1").arg(
                        QString::number(control.m_bytePosition, 16)
                                .rightJustified(2, '0')
                                .toUpper()),
                true);
        pTable->setItem(row, 0, pBytePositionItem);
        // Store custom data for the row in the first cell
        pBytePositionItem->setData(Qt::UserRole + 1,
                QVariant::fromValue(&control));

        // Column 1 - Bit Position
        pTable->setItem(row, 1, createReadOnlyItem(QString::number(control.m_bitPosition), true));
        // Column 2 - Bit Size
        pTable->setItem(row, 2, createReadOnlyItem(QString::number(control.m_bitSize), true));
        // Column 3 - Logical Min
        pTable->setItem(row,
                3,
                createReadOnlyItem(
                        QString::number(control.m_logicalMinimum), true));
        // Column 4 - Logical Max
        pTable->setItem(row,
                4,
                createReadOnlyItem(
                        QString::number(control.m_logicalMaximum), true));
        // Column 5 - Value
        pTable->setItem(row,
                5,
                createValueItem(reportType, control.m_logicalMinimum, control.m_logicalMaximum));
        // Column 6 - Physical Min
        pTable->setItem(row,
                6,
                createReadOnlyItem(
                        QString::number(control.m_physicalMinimum), true));
        // Column 7 - Physical Max
        pTable->setItem(row,
                7,
                createReadOnlyItem(
                        QString::number(control.m_physicalMaximum), true));
        // Column 8 - Unit Scaling
        pTable->setItem(row,
                8,
                createReadOnlyItem(control.m_unitExponent != 0
                                ? QStringLiteral("10^%1").arg(
                                          control.m_unitExponent)
                                : QString(),
                        true));
        // Column 9 - Unit
        pTable->setItem(row,
                9,
                createReadOnlyItem(hid::reportDescriptor::getScaledUnitString(
                        control.m_unit)));
        // Column 10 - Abs/Rel
        pTable->setItem(row,
                10,
                createReadOnlyItem(control.m_flags.absolute_relative
                                ? tr("Relative")
                                : tr("Absolute")));
        // Column 11 - Wrap
        pTable->setItem(row,
                11,
                createReadOnlyItem(control.m_flags.no_wrap_wrap
                                ? tr("Wrap")
                                : tr("No Wrap")));
        // Column 12 - Linear
        pTable->setItem(row,
                12,
                createReadOnlyItem(control.m_flags.linear_non_linear
                                ? tr("Non Linear")
                                : tr("Linear")));
        // Column 13 - Preferred
        pTable->setItem(row,
                13,
                createReadOnlyItem(control.m_flags.preferred_no_preferred
                                ? tr("No Preferred")
                                : tr("Preferred")));
        // Column 14 - Null
        pTable->setItem(row,
                14,
                createReadOnlyItem(control.m_flags.no_null_null
                                ? tr("Null")
                                : tr("No Null")));

        // Volatile column (if present)
        int volatileIndex = (showVolatileColumn ? 15 : -1);
        if (volatileIndex != -1) {
            pTable->setItem(row,
                    volatileIndex,
                    createReadOnlyItem(control.m_flags.non_volatile_volatile
                                    ? tr("Volatile")
                                    : tr("Non Volatile")));
        }

        // Usage Page / Usage
        int usagePageIdx = showVolatileColumn ? 16 : 15;
        int usageDescIdx = showVolatileColumn ? 17 : 16;
        uint16_t usagePage = static_cast<uint16_t>((control.m_usage & 0xFFFF0000) >> 16);
        uint16_t usage = static_cast<uint16_t>(control.m_usage & 0x0000FFFF);

        pTable->setItem(row,
                usagePageIdx,
                createReadOnlyItem(
                        mixxx::hid::HidUsageTables::getUsagePageDescription(
                                usagePage)));
        pTable->setItem(row,
                usageDescIdx,
                createReadOnlyItem(
                        mixxx::hid::HidUsageTables::getUsageDescription(
                                usagePage, usage)));

        ++row;
    }

    // Resize columns to contents once, store width and set column width fixed for performance
    for (int colIdx = 0; colIdx < pTable->columnCount(); ++colIdx) {
        pTable->horizontalHeader()->setSectionResizeMode(colIdx, QHeaderView::ResizeToContents);
    }
    QVector<int> columnWidths;
    columnWidths.reserve(pTable->columnCount());
    for (int colIdx = 0; colIdx < pTable->columnCount(); ++colIdx) {
        columnWidths.push_back(pTable->columnWidth(colIdx));
    }
    // Set the width of the "Value" column (5) to fit 11 digits (int32 minimum in decimal)
    // This is the only column in the table with dynamic content, therefore we need to
    // set the width with enough reserved space.
    QFontMetrics metrics(pTable->font());
    int width = metrics.horizontalAdvance(QStringLiteral("0").repeated(11));
    columnWidths[5] = width; // The column "Value" is at index 5
    for (int colIdx = 0; colIdx < pTable->columnCount(); ++colIdx) {
        pTable->horizontalHeader()->setSectionResizeMode(colIdx, QHeaderView::Fixed);
        pTable->setColumnWidth(colIdx, columnWidths[colIdx]);
    }

    pTable->setUpdatesEnabled(true);
}

QWidget* ValueItemDelegate::createEditor(QWidget* pParent,
        const QStyleOptionViewItem&,
        const QModelIndex& index) const {
    // Create a line edit restricted by (logical min, logical max)
    auto dataRange = index.data(Qt::UserRole).value<QPair<int, int>>();
    auto pEditor = make_parented<QLineEdit>(pParent);
    pEditor->setValidator(make_parented<QIntValidator>(dataRange.first, dataRange.second, pEditor));
    return pEditor;
}

void ValueItemDelegate::setModelData(QWidget* pEditor,
        QAbstractItemModel* pModel,
        const QModelIndex& index) const {
    auto* pLineEdit = qobject_cast<QLineEdit*>(pEditor);
    if (!pLineEdit) {
        return;
    }

    // Confirm the text is an integer within the expected range
    bool ok = false;
    const int value = pLineEdit->text().toInt(&ok);
    if (ok) {
        auto dataRange = index.data(Qt::UserRole).value<QPair<int, int>>();
        if (value >= dataRange.first && value <= dataRange.second) {
            pModel->setData(index, value, Qt::EditRole);
        }
    }
}
