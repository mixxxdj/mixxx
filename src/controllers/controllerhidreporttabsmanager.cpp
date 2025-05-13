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

ControllerHidReportTabsManager::ControllerHidReportTabsManager(
        QTabWidget* pParentTabWidget, HidController* pHidController)
        : m_pParentControllerTab(pParentTabWidget),
          m_pHidController(pHidController) {
}

void ControllerHidReportTabsManager::createReportTypeTabs() {
    auto reportTypeTabs = std::make_unique<QTabWidget>(m_pParentControllerTab);

    QMetaEnum metaEnum = QMetaEnum::fromType<hid::reportDescriptor::HidReportType>();

    for (int reportTypeIdx = 0; reportTypeIdx < metaEnum.keyCount(); ++reportTypeIdx) {
        auto reportType = static_cast<hid::reportDescriptor::HidReportType>(
                metaEnum.value(reportTypeIdx));
        auto reportTypeTab = std::make_unique<QTabWidget>(reportTypeTabs.get());
        createHidReportTab(reportTypeTab.get(), reportType);
        if (reportTypeTab->count() > 0) {
            QString tabName = QStringLiteral("%1 Reports")
                                      .arg(metaEnum.valueToKey(
                                              static_cast<int>(reportType)));
            m_pParentControllerTab->addTab(reportTypeTab.release(), tabName);
        }
    }
}

void ControllerHidReportTabsManager::createHidReportTab(QTabWidget* pParentReportTypeTab,
        hid::reportDescriptor::HidReportType reportType) {
    const auto& reportDescriptorTemp = m_pHidController->getReportDescriptor();
    if (!reportDescriptorTemp.has_value()) {
        return;
    }
    const auto& reportDescriptor = *reportDescriptorTemp;

    QMetaEnum metaEnum = QMetaEnum::fromType<hid::reportDescriptor::HidReportType>();

    for (const auto& reportInfo : reportDescriptor.getListOfReports()) {
        auto [index, type, reportId] = reportInfo;
        if (type == reportType) {
            QString tabName = QStringLiteral("%1 Report 0x%2")
                                      .arg(metaEnum.valueToKey(static_cast<int>(
                                                   reportType)),
                                              QString::number(reportId, 16)
                                                      .rightJustified(2, '0')
                                                      .toUpper());

            auto* pTabWidget = new QWidget(pParentReportTypeTab);
            auto* pLayout = new QVBoxLayout(pTabWidget);
            auto* pTopWidgetRow = new QHBoxLayout();

            // Create buttons
            auto* pReadButton = new QPushButton(QStringLiteral("Read"), pTabWidget);
            auto* pSendButton = new QPushButton(QStringLiteral("Send"), pTabWidget);

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

            auto* pTable = new QTableWidget(pTabWidget);
            pLayout->addWidget(pTable);

            const auto* pReport = reportDescriptor.getReport(reportType, reportId);
            if (pReport) {
                // Show payload size
                auto* pSizeLabel = new QLabel(pTabWidget);
                pSizeLabel->setText(
                        QStringLiteral("Payload Size: <b>%1 bytes</b>")
                                .arg(pReport->getReportSize()));
                pTopWidgetRow->insertWidget(0, pSizeLabel);

                populateHidReportTable(pTable, *pReport, reportType);
            }

            if (reportType != hid::reportDescriptor::HidReportType::Output) {
                connect(pReadButton,
                        &QPushButton::clicked,
                        this,
                        [this, pTable, reportId, reportType]() {
                            slotReadReport(pTable, reportId, reportType);
                        });
                // Read once on tab creation
                slotReadReport(pTable, reportId, reportType);
            }
            if (reportType != hid::reportDescriptor::HidReportType::Input) {
                connect(pSendButton,
                        &QPushButton::clicked,
                        this,
                        [this, pTable, reportId, reportType]() {
                            slotSendReport(pTable, reportId, reportType);
                        });
            }

            pParentReportTypeTab->addTab(pTabWidget, tabName);

            if (reportType == hid::reportDescriptor::HidReportType::Input) {
                // Store the pTable pointer associated with the reportId
                m_reportIdToTableMap[reportId] = pTable;
            }

            // Connect the signal for the reportId
            HidIoThread* hidIoThread = m_pHidController->getHidIoThread();
            connect(hidIoThread,
                    &HidIoThread::reportReceived,
                    this,
                    &ControllerHidReportTabsManager::slotProcessInputReport);
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
        auto* item = pTable->item(row, 5); // Value column is at index 5
        if (item) {
            // Retrieve custom data from the first cell
            QVariant customData = pTable->item(row, 0)->data(Qt::UserRole + 1);
            if (customData.isValid()) {
                auto pControl =
                        static_cast<const hid::reportDescriptor::Control*>(
                                customData.value<const void*>());
                // Use the custom data as needed
                int64_t controlValue =
                        hid::reportDescriptor::extractLogicalValue(
                                reportData, *pControl);
                item->setText(QString::number(controlValue));
            }
        }
    }

    pTable->setUpdatesEnabled(true);
}

void ControllerHidReportTabsManager::slotProcessInputReport(
        quint8 reportId, const QByteArray& data) {
    // Find the table associated with the reportId
    auto it = m_reportIdToTableMap.find(reportId);
    if (it == m_reportIdToTableMap.end()) {
        qWarning() << "No table found for reportId" << reportId;
        return;
    }
    QTableWidget* pTable = it->second;

    const auto& reportDescriptor = *m_pHidController->getReportDescriptor();
    auto pReport = reportDescriptor.getReport(
            hid::reportDescriptor::HidReportType::Input, reportId);
    if (pReport) {
        updateTableWithReportData(pTable, data);
    }
}

void ControllerHidReportTabsManager::slotReadReport(QTableWidget* pTable,
        quint8 reportId,
        hid::reportDescriptor::HidReportType reportType) {
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

    const auto& reportDescriptor = *m_pHidController->getReportDescriptor();
    const auto* pReport = reportDescriptor.getReport(reportType, reportId);
    VERIFY_OR_DEBUG_ASSERT(pReport) {
        return;
    }
    if (reportData.size() < pReport->getReportSize()) {
        qWarning() << "Failed to get report. Read only " << reportData.size()
                   << " instead of expected " << pReport->getReportSize()
                   << " bytes.";
        return;
    }

    updateTableWithReportData(pTable, reportData);
}

void ControllerHidReportTabsManager::slotSendReport(QTableWidget* pTable,
        quint8 reportId,
        hid::reportDescriptor::HidReportType reportType) {
    if (!m_pHidController->isOpen()) {
        qWarning() << "HID controller is not open.";
        return;
    }

    HidControllerJSProxy* pJsProxy =
            static_cast<HidControllerJSProxy*>(m_pHidController->jsProxy());
    VERIFY_OR_DEBUG_ASSERT(pJsProxy) {
        return;
    }

    const auto& reportDescriptor = *m_pHidController->getReportDescriptor();

    auto pReport = reportDescriptor.getReport(reportType, reportId);
    VERIFY_OR_DEBUG_ASSERT(pReport) {
        return;
    }

    // Create a QByteArray of the size of the report
    QByteArray reportData(pReport->getReportSize(), 0);

    // Iterate through each row in the table
    for (int row = 0; row < pTable->rowCount(); ++row) {
        auto* item = pTable->item(row, 5); // Value column is at index 5
        if (item) {
            // Retrieve custom data from the first cell
            QVariant customData = pTable->item(row, 0)->data(Qt::UserRole + 1);
            if (customData.isValid()) {
                auto pControl =
                        reinterpret_cast<hid::reportDescriptor::Control*>(
                                customData.value<void*>());
                // Set the control value in the reportData
                bool success = hid::reportDescriptor::applyLogicalValue(
                        reportData, *pControl, item->text().toLongLong());
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
        pTable->setItemDelegateForColumn(5, new ValueItemDelegate(pTable));
    }

    bool showVolatileColumn = (reportType == hid::reportDescriptor::HidReportType::Feature ||
            reportType == hid::reportDescriptor::HidReportType::Output);

    // Set headers
    QStringList headers = {QStringLiteral("Byte Position"),
            QStringLiteral("Bit Position"),
            QStringLiteral("Bit Size"),
            QStringLiteral("Logical Min"),
            QStringLiteral("Logical Max"),
            QStringLiteral("Value"),
            QStringLiteral("Physical Min"),
            QStringLiteral("Physical Max"),
            QStringLiteral("Unit Scaling"),
            QStringLiteral("Unit"),
            QStringLiteral("Abs/Rel"),
            QStringLiteral("Wrap"),
            QStringLiteral("Linear"),
            QStringLiteral("Preferred"),
            QStringLiteral("Null")};
    if (showVolatileColumn) {
        headers << QStringLiteral("Volatile");
    }
    headers << QStringLiteral("Usage Page") << QStringLiteral("Usage");

    pTable->setColumnCount(headers.size());
    pTable->setHorizontalHeaderLabels(headers);
    pTable->verticalHeader()->setVisible(false);

    // Helpers
    auto createReadOnlyItem = [](const QString& text, bool rightAlign = false) {
        auto* item = new QTableWidgetItem(text);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        if (rightAlign) {
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        return item;
    };
    auto createValueItem = [reportType](int minVal, int maxVal) {
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
    };

    int row = 0;
    for (const auto& pControl : controls) {
        // Column 0 - Byte Position
        auto* bytePositionItem = createReadOnlyItem(QStringLiteral("0x%1").arg(QString::number(
                                                            pControl.m_bytePosition, 16)
                                                                    .rightJustified(2, '0')
                                                                    .toUpper()),
                true);
        pTable->setItem(row, 0, bytePositionItem);
        // Store custom data for the row in the first cell
        bytePositionItem->setData(Qt::UserRole + 1,
                QVariant::fromValue(reinterpret_cast<void*>(
                        const_cast<hid::reportDescriptor::Control*>(
                                &pControl))));

        // Column 1 - Bit Position
        pTable->setItem(row, 1, createReadOnlyItem(QString::number(pControl.m_bitPosition), true));
        // Column 2 - Bit Size
        pTable->setItem(row, 2, createReadOnlyItem(QString::number(pControl.m_bitSize), true));
        // Column 3 - Logical Min
        pTable->setItem(row,
                3,
                createReadOnlyItem(
                        QString::number(pControl.m_logicalMinimum), true));
        // Column 4 - Logical Max
        pTable->setItem(row,
                4,
                createReadOnlyItem(
                        QString::number(pControl.m_logicalMaximum), true));
        // Column 5 - Value
        pTable->setItem(row,
                5,
                createValueItem(
                        pControl.m_logicalMinimum, pControl.m_logicalMaximum));
        // Column 6 - Physical Min
        pTable->setItem(row,
                6,
                createReadOnlyItem(
                        QString::number(pControl.m_physicalMinimum), true));
        // Column 7 - Physical Max
        pTable->setItem(row,
                7,
                createReadOnlyItem(
                        QString::number(pControl.m_physicalMaximum), true));
        // Column 8 - Unit Scaling
        pTable->setItem(row,
                8,
                createReadOnlyItem(pControl.m_unitExponent != 0
                                ? QStringLiteral("10^%1").arg(
                                          pControl.m_unitExponent)
                                : QString(),
                        true));
        // Column 9 - Unit
        pTable->setItem(row,
                9,
                createReadOnlyItem(hid::reportDescriptor::getScaledUnitString(
                        pControl.m_unit)));
        // Column 10 - Abs/Rel
        pTable->setItem(row,
                10,
                createReadOnlyItem(pControl.m_flags.absolute_relative
                                ? QStringLiteral("Relative")
                                : QStringLiteral("Absolute")));
        // Column 11 - Wrap
        pTable->setItem(row,
                11,
                createReadOnlyItem(pControl.m_flags.no_wrap_wrap
                                ? QStringLiteral("Wrap")
                                : QStringLiteral("No Wrap")));
        // Column 12 - Linear
        pTable->setItem(row,
                12,
                createReadOnlyItem(pControl.m_flags.linear_non_linear
                                ? QStringLiteral("Non Linear")
                                : QStringLiteral("Linear")));
        // Column 13 - Preferred
        pTable->setItem(row,
                13,
                createReadOnlyItem(pControl.m_flags.preferred_no_preferred
                                ? QStringLiteral("No Preferred")
                                : QStringLiteral("Preferred")));
        // Column 14 - Null
        pTable->setItem(row,
                14,
                createReadOnlyItem(pControl.m_flags.no_null_null
                                ? QStringLiteral("Null")
                                : QStringLiteral("No Null")));

        // Volatile column (if present)
        int volatileIndex = (showVolatileColumn ? 15 : -1);
        if (volatileIndex != -1) {
            pTable->setItem(row,
                    volatileIndex,
                    createReadOnlyItem(pControl.m_flags.non_volatile_volatile
                                    ? QStringLiteral("Volatile")
                                    : QStringLiteral("Non Volatile")));
        }

        // Usage Page / Usage
        int usagePageIdx = showVolatileColumn ? 16 : 15;
        int usageDescIdx = showVolatileColumn ? 17 : 16;
        uint16_t usagePage = static_cast<uint16_t>((pControl.m_usage & 0xFFFF0000) >> 16);
        uint16_t usage = static_cast<uint16_t>(pControl.m_usage & 0x0000FFFF);

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
    QVector<int> columnWidths(pTable->columnCount());
    for (int colIdx = 0; colIdx < pTable->columnCount(); ++colIdx) {
        columnWidths[colIdx] = pTable->columnWidth(colIdx);
    }
    // Set the width of the value column (5) to fit 11 digits (int32 minimum in decimal)
    QFontMetrics metrics(pTable->font());
    int width = metrics.horizontalAdvance(QStringLiteral("0").repeated(11));
    columnWidths[5] = width;
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
    auto* pEditor = new QLineEdit(pParent);
    pEditor->setValidator(new QIntValidator(dataRange.first, dataRange.second, pEditor));
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
