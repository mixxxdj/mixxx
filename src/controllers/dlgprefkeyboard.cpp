#include "controllers/dlgprefkeyboard.h"

#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QtDebug>

#include <QFileDialog>
#include "controllers/keyboard/dlgkeyboardvisuallayout.h"
#include "controllers/keyboard/dlgkeyboardmappingeditor.h"
#include "coreservices.h"
#include "moc_dlgprefkeyboard.cpp"
#include "defs_urls.h"

DlgPrefKeyboard::DlgPrefKeyboard(QWidget* parent,
                                 UserSettingsPointer pConfig,
                                 KeyboardEventFilter* pKeyboardEventFilter)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_pKeyboardEventFilter(pKeyboardEventFilter),
          m_bChanged(false) {
    
    
    m_pMappingManager = QSharedPointer<KeyboardMappingManager>::create(pConfig);
    m_pActionRegistry = QSharedPointer<KeyboardActionRegistry>::create();
    
    setupUi();
    
    // Connect keyboard event filter signal
    connect(m_pKeyboardEventFilter, &KeyboardEventFilter::keyCaptured,
            this, &DlgPrefKeyboard::slotKeyCaptured);
    
    // Load default mapping
    slotUpdate();
}

DlgPrefKeyboard::~DlgPrefKeyboard() {
    // Disable learning mode if active
    if (m_pKeyboardEventFilter->isLearningMode()) {
        m_pKeyboardEventFilter->setLearningMode(false);
    }
}

void DlgPrefKeyboard::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Top section: Mapping selector and buttons
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    QLabel* mappingLabel = new QLabel(tr("Keyboard Mapping:"), this);
    topLayout->addWidget(mappingLabel);
    
    m_pMappingSelector = new QComboBox(this);
    topLayout->addWidget(m_pMappingSelector, 1);
    
    m_pSaveButton = new QPushButton(tr("Save As..."), this);
    m_pSaveButton->setToolTip(tr("Save current mapping as a custom mapping"));
    connect(m_pSaveButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotSaveMapping);
    topLayout->addWidget(m_pSaveButton);
    
    m_pExportJsonButton = new QPushButton(tr("JSON"), this);
    m_pExportJsonButton->setToolTip(tr("Export current mapping to JSON format"));
    connect(m_pExportJsonButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotExportJson);
    topLayout->addWidget(m_pExportJsonButton);
    
    m_pExportXmlButton = new QPushButton(tr("XML"), this);
    m_pExportXmlButton->setToolTip(tr("Export current mapping to XML format"));
    connect(m_pExportXmlButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotExportXml);
    topLayout->addWidget(m_pExportXmlButton);
    
    m_pVisualLayoutButton = new QPushButton(tr("Visual Layout"), this);
    m_pVisualLayoutButton->setToolTip(tr("Show a graphical representation of the keyboard mapping"));
    connect(m_pVisualLayoutButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotShowVisualLayout);
    topLayout->addWidget(m_pVisualLayoutButton);

    m_pLoadFileButton = new QPushButton(tr("Load from File..."), this);
    m_pLoadFileButton->setToolTip(tr("Load a keyboard mapping directly from a file"));
    connect(m_pLoadFileButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotLoadMappingFromFile);
    topLayout->addWidget(m_pLoadFileButton);
    
    m_pDocsButton = new QPushButton(tr("Documentation"), this);
    m_pDocsButton->setToolTip(tr("Open Mixxx keyboard mapping documentation"));
    connect(m_pDocsButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotOpenDocumentation);
    topLayout->addWidget(m_pDocsButton);
    
    mainLayout->addLayout(topLayout);
    
    // Status label
    m_pStatusLabel = new QLabel(this);
    m_pStatusLabel->setStyleSheet("QLabel { color: #0080ff; font-weight: bold; }");
    mainLayout->addWidget(m_pStatusLabel);
    
    // Mapping table
    m_pMappingTable = new QTableWidget(this);
    m_pMappingTable->setColumnCount(4);
    m_pMappingTable->setHorizontalHeaderLabels(
        QStringList() << tr("Key") << tr("Action") << tr("Group") << tr("Description"));
    m_pMappingTable->horizontalHeader()->setStretchLastSection(true);
    m_pMappingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pMappingTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pMappingTable->setAlternatingRowColors(true);
    m_pMappingTable->setSortingEnabled(true);
    
    // Set column widths
    m_pMappingTable->setColumnWidth(0, 150);
    m_pMappingTable->setColumnWidth(1, 200);
    m_pMappingTable->setColumnWidth(2, 150);
    
    mainLayout->addWidget(m_pMappingTable);
    
    // Bottom buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_pAddButton = new QPushButton(tr("Add Mapping"), this);
    m_pAddButton->setToolTip(tr("Add a new keyboard mapping"));
    connect(m_pAddButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotAddMapping);
    buttonLayout->addWidget(m_pAddButton);
    
    m_pEditButton = new QPushButton(tr("Edit Mapping"), this);
    m_pEditButton->setToolTip(tr("Edit the selected mapping"));
    m_pEditButton->setEnabled(false);
    connect(m_pEditButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotEditMapping);
    buttonLayout->addWidget(m_pEditButton);
    
    m_pRemoveButton = new QPushButton(tr("Remove Mapping"), this);
    m_pRemoveButton->setToolTip(tr("Remove the selected mapping"));
    m_pRemoveButton->setEnabled(false);
    connect(m_pRemoveButton, &QPushButton::clicked, this, &DlgPrefKeyboard::slotRemoveMapping);
    buttonLayout->addWidget(m_pRemoveButton);
    
    buttonLayout->addStretch();
    
    m_pLearnButton = new QPushButton(tr("Key Learn Mode"), this);
    m_pLearnButton->setCheckable(true);
    m_pLearnButton->setToolTip(tr("Enable key learning mode to capture keyboard shortcuts"));
    connect(m_pLearnButton, &QPushButton::toggled, this, &DlgPrefKeyboard::slotToggleLearning);
    buttonLayout->addWidget(m_pLearnButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect table selection
    connect(m_pMappingTable, &QTableWidget::itemSelectionChanged,
            this, [this]() {
                bool hasSelection = !m_pMappingTable->selectedItems().isEmpty();
                m_pEditButton->setEnabled(hasSelection);
                m_pRemoveButton->setEnabled(hasSelection);
            });
    
    // Connect mapping selector
    connect(m_pMappingSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DlgPrefKeyboard::slotMappingSelected);
    
    setLayout(mainLayout);
}

void DlgPrefKeyboard::slotUpdate() {
    // Populate mapping selector
    m_pMappingSelector->clear();
    QList<KeyboardMappingInfo> mappings = m_pMappingManager->getAvailableMappings();
    
    for (const KeyboardMappingInfo& info : mappings) {
        QString displayName = info.name;
        if (info.isDefault) {
            displayName += tr(" (Default)");
        }
        m_pMappingSelector->addItem(displayName, info.filePath);
    }
    
    // Load default mapping
    QString defaultPath = m_pMappingManager->getDefaultMappingPath();
    int defaultIndex = m_pMappingSelector->findData(defaultPath);
    if (defaultIndex >= 0) {
        m_pMappingSelector->setCurrentIndex(defaultIndex);
    }
}

void DlgPrefKeyboard::slotApply() {
    if (m_bChanged && m_pCurrentMapping) {
        QString currentPath = m_currentMappingPath;
        KeyboardMappingInfo info = m_pMappingManager->getMappingInfo(currentPath);

        if (info.isReadOnly || info.isDefault || currentPath.startsWith(":")) {
            // Create a custom clone if we're editing a read-only mapping
            QString newName = info.name;
            if (!newName.endsWith("(Custom)")) newName += " (Custom)";
            QString newPath = m_pMappingManager->createCustomMapping(newName, "User", tr("Created via Preferences"));
            
            if (!newPath.isEmpty()) {
                currentPath = newPath;
                m_currentMappingPath = newPath;
                info = m_pMappingManager->getMappingInfo(currentPath);
            }
        }

        if (m_pMappingManager->saveMapping(currentPath, info, m_pCurrentMapping.data())) {
            // Apply to keyboard event filter
            m_pKeyboardEventFilter->setKeyboardConfig(m_pCurrentMapping.data());
            
            // Reload mappings to show new clone if created
            slotUpdate();
            // Select the current one
            int index = m_pMappingSelector->findData(currentPath);
            if (index >= 0) m_pMappingSelector->setCurrentIndex(index);

            m_bChanged = false;
            m_pStatusLabel->setText(tr("Mapping saved successfully"));
        } else {
            m_pStatusLabel->setText(tr("Failed to save mapping!"));
        }
    }
}

void DlgPrefKeyboard::slotResetToDefaults() {
    QString defaultPath = m_pMappingManager->getDefaultMappingPath();
    int defaultIndex = m_pMappingSelector->findData(defaultPath);
    if (defaultIndex >= 0) {
        m_pMappingSelector->setCurrentIndex(defaultIndex);
    }
    m_bChanged = false;
}

void DlgPrefKeyboard::slotCancel() {
    slotUpdate();
    m_bChanged = false;
}

QUrl DlgPrefKeyboard::helpUrl() const {
    return QUrl(MIXXX_MANUAL_CONTROLLERS_URL);
}

void DlgPrefKeyboard::slotMappingSelected(int index) {
    if (index < 0) {
        return;
    }
    
    QString filePath = m_pMappingSelector->itemData(index).toString();
    m_currentMappingPath = filePath;
    
    // Load the mapping
    m_pCurrentMapping = m_pMappingManager->loadMapping(filePath);
    
    // Display in table
    loadMappingToTable(m_pCurrentMapping);
    
    m_pStatusLabel->setText(tr("Loaded mapping: %1").arg(
        m_pMappingManager->getMappingInfo(filePath).name));
}

void DlgPrefKeyboard::loadMappingToTable(const QSharedPointer<ConfigObject<ConfigValueKbd>>& pMapping) {
    m_pMappingTable->setRowCount(0);
    m_pMappingTable->setSortingEnabled(false);
    
    if (!pMapping) {
        return;
    }
    
    // Get all groups
    QList<QString> groups = pMapping->getGroups().values();
    std::sort(groups.begin(), groups.end());
    
    int row = 0;
    for (const QString& group : groups) {
        if (group == "Metadata") {
            continue;
        }
        
        // Get all keys in this group
        QList<ConfigKey> keys = pMapping->getKeysWithGroup(group);
        for (const ConfigKey& configKey : keys) {
            QString keySequence = pMapping->getValueString(configKey);
            if (keySequence.isEmpty()) {
                continue;
            }
            
            m_pMappingTable->insertRow(row);
            
            // Key column
            QTableWidgetItem* keyItem = new QTableWidgetItem(keySequence);
            keyItem->setFlags(keyItem->flags() & ~Qt::ItemIsEditable);
            m_pMappingTable->setItem(row, 0, keyItem);
            
            // Action column
            QTableWidgetItem* actionItem = new QTableWidgetItem(configKey.item);
            actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
            m_pMappingTable->setItem(row, 1, actionItem);
            
            // Group column
            QTableWidgetItem* groupItem = new QTableWidgetItem(configKey.group);
            groupItem->setFlags(groupItem->flags() & ~Qt::ItemIsEditable);
            
            // Color code by deck
            QString color = getDeckColor(configKey.group);
            if (!color.isEmpty()) {
                groupItem->setBackground(QColor(color));
            }
            
            m_pMappingTable->setItem(row, 2, groupItem);
            
            // Description column
            ControlActionInfo actionInfo = m_pActionRegistry->getActionInfo(configKey);
            QString description = actionInfo.description.isEmpty() ? 
                                  configKey.item : actionInfo.description;
            QTableWidgetItem* descItem = new QTableWidgetItem(description);
            descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
            m_pMappingTable->setItem(row, 3, descItem);
            
            row++;
        }
    }
    
    m_pMappingTable->setSortingEnabled(true);
}

void DlgPrefKeyboard::saveMappingFromTable() {
    // This would save table contents back to the mapping object
    // For now, mappings are read-only in the table
    // Full implementation would allow inline editing
}

QString DlgPrefKeyboard::getDeckColor(const QString& group) {
    if (group.contains("Channel1")) {
        return "#E3F2FD"; // Light blue for Deck 1
    } else if (group.contains("Channel2")) {
        return "#FFEBEE"; // Light red for Deck 2
    } else if (group.contains("Channel3")) {
        return "#F3E5F5"; // Light purple for Deck 3
    } else if (group.contains("Channel4")) {
        return "#FFF3E0"; // Light orange for Deck 4
    } else if (group.contains("PreviewDeck")) {
        return "#E8F5E9"; // Light green for Preview
    }
    return QString();
}

void DlgPrefKeyboard::slotAddMapping() {
    if (!m_pCurrentMapping) return;

    DlgKeyboardMappingEditor editor(this, QKeySequence(), ConfigKey(), m_pActionRegistry.data(), m_pCurrentMapping.data());
    if (editor.exec() == QDialog::Accepted) {
        QList<ConfigKey> newKeys = editor.getSelectedMappings();
        QKeySequence seq = editor.getSelectedKey();
        
        if (!seq.isEmpty()) {
            for (const auto& newKey : newKeys) {
                m_pCurrentMapping->set(newKey, ConfigValueKbd(seq.toString()));
            }
            m_bChanged = true;
            m_pStatusLabel->setText(tr("Added mapping for %1").arg(seq.toString()));
            loadMappingToTable(m_pCurrentMapping);
        }
    }
}

void DlgPrefKeyboard::slotEditMapping() {
    QList<QTableWidgetItem*> selected = m_pMappingTable->selectedItems();
    if (selected.isEmpty()) return;

    int row = selected.first()->row();
    QKeySequence seq(m_pMappingTable->item(row, 0)->text());
    ConfigKey currentKey(m_pMappingTable->item(row, 2)->text(), 
                         m_pMappingTable->item(row, 1)->text());

    DlgKeyboardMappingEditor editor(this, seq, currentKey, m_pActionRegistry.data(), m_pCurrentMapping.data());
    if (editor.exec() == QDialog::Accepted) {
        if (editor.isUnmapped()) {
             QMultiHash<ConfigValueKbd, ConfigKey> transposedEx = m_pCurrentMapping->transpose();
             ConfigValueKbd valEx(seq.toString());
             QList<ConfigKey> toRemove = transposedEx.values(valEx);
             for (const auto& exKey : toRemove) {
                 m_pCurrentMapping->remove(exKey);
             }
        } else {
            QList<ConfigKey> newKeys = editor.getSelectedMappings();
            QKeySequence newSeq = editor.getSelectedKey();

            // If key changed, remove old one
            if (newSeq != seq) {
                 m_pCurrentMapping->remove(currentKey);
            }

            for (const auto& newKey : newKeys) {
                m_pCurrentMapping->set(newKey, ConfigValueKbd(newSeq.toString()));
            }
            m_pStatusLabel->setText(tr("Updated mapping for %1").arg(newSeq.toString()));
        }
        
        m_bChanged = true;
        loadMappingToTable(m_pCurrentMapping);
    }
}

void DlgPrefKeyboard::slotRemoveMapping() {
    QList<QTableWidgetItem*> selected = m_pMappingTable->selectedItems();
    if (selected.isEmpty()) {
        return;
    }
    
    int row = selected.first()->row();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Remove Mapping"),
        tr("Are you sure you want to remove this mapping?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_pMappingTable->removeRow(row);
        m_bChanged = true;
        m_pStatusLabel->setText(tr("Mapping removed (save to apply)"));
    }
}

void DlgPrefKeyboard::slotToggleLearning(bool enabled) {
    m_pKeyboardEventFilter->setLearningMode(enabled);
    
    if (enabled) {
        m_pStatusLabel->setText(tr("Key learning mode ACTIVE - Press any key combination..."));
        m_pStatusLabel->setStyleSheet("QLabel { color: #ff0000; font-weight: bold; }");
    } else {
        m_pStatusLabel->setText(tr("Key learning mode disabled"));
        m_pStatusLabel->setStyleSheet("QLabel { color: #0080ff; font-weight: bold; }");
    }
}

void DlgPrefKeyboard::slotSaveMapping() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save Custom Mapping"),
                                        tr("Mapping name:"), QLineEdit::Normal,
                                        tr("My Custom Mapping"), &ok);
    
    if (ok && !name.isEmpty()) {
        QString author = QInputDialog::getText(this, tr("Save Custom Mapping"),
                                              tr("Author (optional):"), QLineEdit::Normal,
                                              QString(), &ok);
        
        if (ok) {
            QString description = QInputDialog::getText(this, tr("Save Custom Mapping"),
                                                       tr("Description (optional):"), QLineEdit::Normal,
                                                       QString(), &ok);
            
            if (ok) {
                QString filePath = m_pMappingManager->createCustomMapping(name, author, description);
                
                if (!filePath.isEmpty()) {
                    // Reload mappings
                    slotUpdate();
                    
                    // Select the new mapping
                    int index = m_pMappingSelector->findData(filePath);
                    if (index >= 0) {
                        m_pMappingSelector->setCurrentIndex(index);
                    }
                    
                    m_pStatusLabel->setText(tr("Custom mapping created: %1").arg(name));
                } else {
                    QMessageBox::warning(this, tr("Error"),
                        tr("Failed to create custom mapping"));
                }
            }
        }
    }
}

void DlgPrefKeyboard::slotExportJson() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export JSON Mapping"),
                                                  QString(), tr("JSON Files (*.json)"));
    if (!fileName.isEmpty()) {
        KeyboardMappingInfo info = m_pMappingManager->getMappingInfo(m_currentMappingPath);
        if (m_pMappingManager->exportToJson(fileName, info, m_pCurrentMapping.data())) {
            m_pStatusLabel->setText(tr("Exported to JSON: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Export Error"), tr("Failed to export to JSON"));
        }
    }
}

void DlgPrefKeyboard::slotExportXml() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export XML Mapping"),
                                                  QString(), tr("XML Files (*.xml)"));
    if (!fileName.isEmpty()) {
        KeyboardMappingInfo info = m_pMappingManager->getMappingInfo(m_currentMappingPath);
        if (m_pMappingManager->exportToXml(fileName, info, m_pCurrentMapping.data())) {
            m_pStatusLabel->setText(tr("Exported to XML: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("Export Error"), tr("Failed to export to XML"));
        }
    }
}

void DlgPrefKeyboard::slotShowVisualLayout() {
    if (!m_pCurrentMapping) {
        // Try to load the currently selected item in the dropdown
        int index = m_pMappingSelector->currentIndex();
        if (index >= 0) {
            slotMappingSelected(index);
        }
    }
    
    if (!m_pCurrentMapping) {
        m_pStatusLabel->setText(tr("Please select a mapping first"));
        return;
    }
    
    DlgKeyboardVisualLayout dlg(this, m_pCurrentMapping.data(), m_pKeyboardEventFilter);
    connect(&dlg, &DlgKeyboardVisualLayout::keyClicked,
            this, &DlgPrefKeyboard::slotVisualKeyClicked);
    dlg.exec();
}

void DlgPrefKeyboard::slotVisualKeyClicked(QKeySequence seq) {
    if (!m_pCurrentMapping) return;

    // Find current mapping for this key if it exists
    ConfigKey currentKey;
    QMultiHash<ConfigValueKbd, ConfigKey> transposed = m_pCurrentMapping->transpose();
    ConfigValueKbd val(seq.toString());
    if (transposed.contains(val)) {
        currentKey = transposed.value(val);
    }

    DlgKeyboardMappingEditor editor(this, seq, currentKey, m_pActionRegistry.data(), m_pCurrentMapping.data());
    if (editor.exec() == QDialog::Accepted) {
        if (editor.isUnmapped()) {
             QMultiHash<ConfigValueKbd, ConfigKey> transposedEx = m_pCurrentMapping->transpose();
             ConfigValueKbd valEx(seq.toString());
             QList<ConfigKey> toRemove = transposedEx.values(valEx);
             for (const auto& exKey : toRemove) {
                 m_pCurrentMapping->remove(exKey);
             }
        } else {
            QList<ConfigKey> newKeys = editor.getSelectedMappings();
            for (const auto& newKey : newKeys) {
                m_pCurrentMapping->set(newKey, ConfigValueKbd(seq.toString()));
            }
        }

        m_bChanged = true;
        loadMappingToTable(m_pCurrentMapping); // Refresh table
        slotUpdate(); // Refresh selector
    }
}

void DlgPrefKeyboard::slotKeyCaptured(QKeySequence keySequence) {
    m_pStatusLabel->setText(tr("Captured key: %1").arg(keySequence.toString()));
    
    // Auto-disable learning mode after capture
    m_pLearnButton->setChecked(false);
    
    // Open editor with this key
    DlgKeyboardMappingEditor editor(this, keySequence, ConfigKey(), m_pActionRegistry.data(), m_pCurrentMapping.data());
    if (editor.exec() == QDialog::Accepted) {
        if (editor.isUnmapped()) {
             QMultiHash<ConfigValueKbd, ConfigKey> transposedEx = m_pCurrentMapping->transpose();
             ConfigValueKbd valEx(keySequence.toString());
             QList<ConfigKey> toRemove = transposedEx.values(valEx);
             for (const auto& exKey : toRemove) {
                 m_pCurrentMapping->remove(exKey);
             }
        } else {
            QList<ConfigKey> newKeys = editor.getSelectedMappings();
            for (const auto& newKey : newKeys) {
                m_pCurrentMapping->set(newKey, ConfigValueKbd(keySequence.toString()));
            }
        }
        m_bChanged = true;
        loadMappingToTable(m_pCurrentMapping);
    }
}

void DlgPrefKeyboard::slotLoadMappingFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Keyboard Mapping"),
                                                   QString(), tr("Mixxx Keyboard Mapping (*.kbd.cfg)"));
    if (!fileName.isEmpty()) {
        m_pMappingManager->addExternalMapping(fileName);
        
        // Refresh selector
        slotUpdate();
        
        // Select the new mapping
        int index = m_pMappingSelector->findData(fileName);
        if (index >= 0) {
            m_pMappingSelector->setCurrentIndex(index);
        }
        
        m_pStatusLabel->setText(tr("Loaded external mapping: %1").arg(fileName));
    }
}

void DlgPrefKeyboard::slotOpenDocumentation() {
    QDesktopServices::openUrl(QUrl("https://github.com/mixxxdj/mixxx/wiki/Keyboard-Mapping"));
}
