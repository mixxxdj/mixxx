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
        QTabWidget* parentTabWidget, HidController* hidController)
        : m_pParentControllerTab(parentTabWidget),
          m_pHidController(hidController) {
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

void ControllerHidReportTabsManager::createHidReportTab(QTabWidget* parentReportTypeTab,
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

            auto* tabWidget = new QWidget(parentReportTypeTab);
            auto* layout = new QVBoxLayout(tabWidget);
            auto* topWidgetRow = new QHBoxLayout();

            // Create buttons
            auto* readButton = new QPushButton(QStringLiteral("Read"), tabWidget);
            auto* sendButton = new QPushButton(QStringLiteral("Send"), tabWidget);

            // Adjust visibility/enable state based on the report type
            if (reportType == hid::reportDescriptor::HidReportType::Input) {
                sendButton->hide();
                readButton->hide();
            } else if (reportType == hid::reportDescriptor::HidReportType::Output) {
                readButton->hide();
            }

            topWidgetRow->addWidget(readButton);
            topWidgetRow->addWidget(sendButton);
            layout->addLayout(topWidgetRow);

            auto* table = new QTableWidget(tabWidget);
            layout->addWidget(table);

            auto report = reportDescriptor.getReport(reportType, reportId);
            if (report) {
                // Show payload size
                auto* sizeLabel = new QLabel(tabWidget);
                sizeLabel->setText(
                        QStringLiteral("Payload Size: <b>%1 bytes</b>")
                                .arg(report->getReportSize()));
                topWidgetRow->insertWidget(0, sizeLabel);

                populateHidReportTable(table, *report, reportType);
            }

            if (reportType != hid::reportDescriptor::HidReportType::Output) {
                connect(readButton,
                        &QPushButton::clicked,
                        this,
                        [this, table, reportId, reportType]() {
                            slotReadReport(table, reportId, reportType);
                        });
                // Read once on tab creation
                slotReadReport(table, reportId, reportType);
            }
            if (reportType != hid::reportDescriptor::HidReportType::Input) {
                connect(sendButton,
                        &QPushButton::clicked,
                        this,
                        [this, table, reportId, reportType]() {
                            slotSendReport(table, reportId, reportType);
                        });
            }

            parentReportTypeTab->addTab(tabWidget, tabName);

            if (reportType == hid::reportDescriptor::HidReportType::Input) {
                // Store the table pointer associated with the reportId
                m_reportIdToTableMap[reportId] = table;
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
        QTableWidget* table,
        const QByteArray& reportData) {
    // Temporarily disable updates to speed up processing
    table->setUpdatesEnabled(false);

    // Process the report data and update the table
    for (int row = 0; row < table->rowCount(); ++row) {
        auto* item = table->item(row, 5); // Value column is at index 5
        if (item) {
            // Retrieve custom data from the first cell
            QVariant customData = table->item(row, 0)->data(Qt::UserRole + 1);
            if (customData.isValid()) {
                auto control =
                        static_cast<const hid::reportDescriptor::Control*>(
                                customData.value<const void*>());
                // Use the custom data as needed
                int64_t controlValue =
                        hid::reportDescriptor::extractLogicalValue(
                                reportData, *control);
                item->setText(QString::number(controlValue));
            }
        }
    }

    table->setUpdatesEnabled(true);
}

void ControllerHidReportTabsManager::slotProcessInputReport(
        quint8 reportId, const QByteArray& data) {
    // Find the table associated with the reportId
    auto it = m_reportIdToTableMap.find(reportId);
    if (it == m_reportIdToTableMap.end()) {
        qWarning() << "No table found for reportId" << reportId;
        return;
    }
    QTableWidget* table = it->second;

    const auto& reportDescriptor = *m_pHidController->getReportDescriptor();
    auto report = reportDescriptor.getReport(hid::reportDescriptor::HidReportType::Input, reportId);
    if (report) {
        updateTableWithReportData(table, data);
    }
}

void ControllerHidReportTabsManager::slotReadReport(QTableWidget* table,
        quint8 reportId,
        hid::reportDescriptor::HidReportType reportType) {
    if (!m_pHidController->isOpen()) {
        qWarning() << "HID controller is not open.";
        return;
    }

    HidControllerJSProxy* jsProxy = static_cast<HidControllerJSProxy*>(m_pHidController->jsProxy());
    VERIFY_OR_DEBUG_ASSERT(jsProxy) {
        return;
    }

    const auto& reportDescriptor = *m_pHidController->getReportDescriptor();
    auto report = reportDescriptor.getReport(reportType, reportId);
    VERIFY_OR_DEBUG_ASSERT(report) {
        return;
    }

    QByteArray reportData;
    if (reportType == hid::reportDescriptor::HidReportType::Input) {
        reportData = jsProxy->getInputReport(reportId);
    } else if (reportType == hid::reportDescriptor::HidReportType::Feature) {
        reportData = jsProxy->getFeatureReport(reportId);
    } else {
        return;
    }

    if (reportData.size() < report->getReportSize()) {
        qWarning() << "Failed to get report. Read only " << reportData.size()
                   << " instead of expected " << report->getReportSize()
                   << " bytes.";
        return;
    }

    updateTableWithReportData(table, reportData);
}

void ControllerHidReportTabsManager::slotSendReport(QTableWidget* table,
        quint8 reportId,
        hid::reportDescriptor::HidReportType reportType) {
    if (!m_pHidController->isOpen()) {
        qWarning() << "HID controller is not open.";
        return;
    }

    HidControllerJSProxy* jsProxy = static_cast<HidControllerJSProxy*>(m_pHidController->jsProxy());
    VERIFY_OR_DEBUG_ASSERT(jsProxy) {
        return;
    }

    const auto& reportDescriptor = *m_pHidController->getReportDescriptor();

    auto report = reportDescriptor.getReport(reportType, reportId);
    VERIFY_OR_DEBUG_ASSERT(report) {
        return;
    }

    // Create a QByteArray of the size of the report
    QByteArray reportData(report->getReportSize(), 0);

    // Iterate through each row in the table
    for (int row = 0; row < table->rowCount(); ++row) {
        auto* item = table->item(row, 5); // Value column is at index 5
        if (item) {
            // Retrieve custom data from the first cell
            QVariant customData = table->item(row, 0)->data(Qt::UserRole + 1);
            if (customData.isValid()) {
                auto control =
                        reinterpret_cast<hid::reportDescriptor::Control*>(
                                customData.value<void*>());
                // Set the control value in the reportData
                bool success = hid::reportDescriptor::applyLogicalValue(
                        reportData, *control, item->text().toLongLong());
                if (!success) {
                    qWarning() << "Failed to set control value for row" << row;
                    continue;
                }
            }
        }
    }

    // Send the reportData
    if (reportType == hid::reportDescriptor::HidReportType::Feature) {
        jsProxy->sendFeatureReport(reportId, reportData);
    } else if (reportType == hid::reportDescriptor::HidReportType::Output) {
        jsProxy->sendOutputReport(reportId, reportData);
    }
}

void ControllerHidReportTabsManager::populateHidReportTable(
        QTableWidget* table,
        const hid::reportDescriptor::Report& report,
        hid::reportDescriptor::HidReportType reportType) {
    // Temporarily disable updates to speed up populating
    table->setUpdatesEnabled(false);

    // Reserve rows up-front
    const auto& controls = report.getControls();
    table->setRowCount(static_cast<int>(controls.size()));

    // Set the delegate once if needed
    if (reportType != hid::reportDescriptor::HidReportType::Input) {
        table->setItemDelegateForColumn(5, new ValueItemDelegate(table));
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

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);

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
    for (const auto& control : controls) {
        // Column 0 - Byte Position
        auto* bytePositionItem = createReadOnlyItem(QStringLiteral("0x%1").arg(QString::number(
                                                            control.m_bytePosition, 16)
                                                                    .rightJustified(2, '0')
                                                                    .toUpper()),
                true);
        table->setItem(row, 0, bytePositionItem);
        // Store custom data for the row in the first cell
        bytePositionItem->setData(Qt::UserRole + 1,
                QVariant::fromValue(reinterpret_cast<void*>(
                        const_cast<hid::reportDescriptor::Control*>(
                                &control))));

        // Column 1 - Bit Position
        table->setItem(row, 1, createReadOnlyItem(QString::number(control.m_bitPosition), true));
        // Column 2 - Bit Size
        table->setItem(row, 2, createReadOnlyItem(QString::number(control.m_bitSize), true));
        // Column 3 - Logical Min
        table->setItem(row, 3, createReadOnlyItem(QString::number(control.m_logicalMinimum), true));
        // Column 4 - Logical Max
        table->setItem(row, 4, createReadOnlyItem(QString::number(control.m_logicalMaximum), true));
        // Column 5 - Value
        table->setItem(row, 5, createValueItem(control.m_logicalMinimum, control.m_logicalMaximum));
        // Column 6 - Physical Min
        table->setItem(row,
                6,
                createReadOnlyItem(
                        QString::number(control.m_physicalMinimum), true));
        // Column 7 - Physical Max
        table->setItem(row,
                7,
                createReadOnlyItem(
                        QString::number(control.m_physicalMaximum), true));
        // Column 8 - Unit Scaling
        table->setItem(row,
                8,
                createReadOnlyItem(control.m_unitExponent != 0
                                ? QStringLiteral("10^%1").arg(
                                          control.m_unitExponent)
                                : QString(),
                        true));
        // Column 9 - Unit
        table->setItem(row,
                9,
                createReadOnlyItem(hid::reportDescriptor::getScaledUnitString(
                        control.m_unit)));
        // Column 10 - Abs/Rel
        table->setItem(row,
                10,
                createReadOnlyItem(control.m_flags.absolute_relative
                                ? QStringLiteral("Relative")
                                : QStringLiteral("Absolute")));
        // Column 11 - Wrap
        table->setItem(row,
                11,
                createReadOnlyItem(control.m_flags.no_wrap_wrap
                                ? QStringLiteral("Wrap")
                                : QStringLiteral("No Wrap")));
        // Column 12 - Linear
        table->setItem(row,
                12,
                createReadOnlyItem(control.m_flags.linear_non_linear
                                ? QStringLiteral("Non Linear")
                                : QStringLiteral("Linear")));
        // Column 13 - Preferred
        table->setItem(row,
                13,
                createReadOnlyItem(control.m_flags.preferred_no_preferred
                                ? QStringLiteral("No Preferred")
                                : QStringLiteral("Preferred")));
        // Column 14 - Null
        table->setItem(row,
                14,
                createReadOnlyItem(control.m_flags.no_null_null
                                ? QStringLiteral("Null")
                                : QStringLiteral("No Null")));

        // Volatile column (if present)
        int volatileIndex = (showVolatileColumn ? 15 : -1);
        if (volatileIndex != -1) {
            table->setItem(row,
                    volatileIndex,
                    createReadOnlyItem(control.m_flags.non_volatile_volatile
                                    ? QStringLiteral("Volatile")
                                    : QStringLiteral("Non Volatile")));
        }

        // Usage Page / Usage
        int usagePageIdx = showVolatileColumn ? 16 : 15;
        int usageDescIdx = showVolatileColumn ? 17 : 16;
        uint16_t usagePage = static_cast<uint16_t>((control.m_usage & 0xFFFF0000) >> 16);
        uint16_t usage = static_cast<uint16_t>(control.m_usage & 0x0000FFFF);

        table->setItem(row,
                usagePageIdx,
                createReadOnlyItem(
                        mixxx::hid::HidUsageTables::getUsagePageDescription(
                                usagePage)));
        table->setItem(row,
                usageDescIdx,
                createReadOnlyItem(
                        mixxx::hid::HidUsageTables::getUsageDescription(
                                usagePage, usage)));

        ++row;
    }

    // Resize columns to contents once, store width and set column width fixed for performance
    for (int colIdx = 0; colIdx < table->columnCount(); ++colIdx) {
        table->horizontalHeader()->setSectionResizeMode(colIdx, QHeaderView::ResizeToContents);
    }
    QVector<int> columnWidths(table->columnCount());
    for (int colIdx = 0; colIdx < table->columnCount(); ++colIdx) {
        columnWidths[colIdx] = table->columnWidth(colIdx);
    }
    // Set the width of the value column (5) to fit 11 digits (int32 minimum in decimal)
    QFontMetrics metrics(table->font());
    int width = metrics.horizontalAdvance(QStringLiteral("0").repeated(11));
    columnWidths[5] = width;
    for (int colIdx = 0; colIdx < table->columnCount(); ++colIdx) {
        table->horizontalHeader()->setSectionResizeMode(colIdx, QHeaderView::Fixed);
        table->setColumnWidth(colIdx, columnWidths[colIdx]);
    }

    table->setUpdatesEnabled(true);
}

QWidget* ValueItemDelegate::createEditor(QWidget* parent,
        const QStyleOptionViewItem&,
        const QModelIndex& index) const {
    // Create a line edit restricted by (logical min, logical max)
    auto dataRange = index.data(Qt::UserRole).value<QPair<int, int>>();
    auto* editor = new QLineEdit(parent);
    editor->setValidator(new QIntValidator(dataRange.first, dataRange.second, editor));
    return editor;
}

void ValueItemDelegate::setModelData(QWidget* editor,
        QAbstractItemModel* model,
        const QModelIndex& index) const {
    auto* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit) {
        return;
    }

    // Confirm the text is an integer within the expected range
    bool ok = false;
    const int value = lineEdit->text().toInt(&ok);
    if (ok) {
        auto dataRange = index.data(Qt::UserRole).value<QPair<int, int>>();
        if (value >= dataRange.first && value <= dataRange.second) {
            model->setData(index, value, Qt::EditRole);
        }
    }
}
