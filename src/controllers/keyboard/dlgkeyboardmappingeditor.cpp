#include "controllers/keyboard/dlgkeyboardmappingeditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include "moc_dlgkeyboardmappingeditor.cpp"

#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>

DlgKeyboardMappingEditor::DlgKeyboardMappingEditor(QWidget* parent,
                                                 const QKeySequence& keySeq,
                                                 const ConfigKey& currentMapping,
                                                 KeyboardActionRegistry* pRegistry,
                                                 ConfigObject<ConfigValueKbd>* pMapping)
        : QDialog(parent),
          m_keySeq(keySeq),
          m_currentMapping(currentMapping),
          m_pRegistry(pRegistry),
          m_pMapping(pMapping),
          m_isUnmapped(false) {
    setWindowTitle(tr("Keyboard Mapping Editor"));
    setMinimumWidth(450);
    setupUi();
    loadActions();
}

void DlgKeyboardMappingEditor::accept() {
    QKeySequence newKeySeq = getSelectedKey();

    if (newKeySeq.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Key"), tr("Please enter a valid key shortcut."));
        return;
    }

    // Conflict Check
    if (m_pMapping) {
        QList<ConfigKey> newActions = getSelectedMappings();
        QMultiHash<ConfigValueKbd, ConfigKey> transposed = m_pMapping->transpose();
        ConfigValueKbd val(newKeySeq.toString());
        
        bool conflictFound = false;
        QString conflictList;
        
        if (transposed.contains(val)) {
            QList<ConfigKey> existingActions = transposed.values(val);
            for (const auto& existing : existingActions) {
                // If it's NOT one of the actions we are about to set/keep
                bool willBeOverwritten = true;
                for (const auto& newAction : newActions) {
                    if (existing == newAction) {
                        willBeOverwritten = false;
                        break;
                    }
                }
                
                if (willBeOverwritten) {
                    conflictList += QString(" - %1 (%2)\n").arg(existing.item, existing.group);
                    conflictFound = true;
                }
            }
        }
        
        if (conflictFound) {
            QString msg = tr("The key combination '%1' is already assigned to:\n\n%2\n"
                             "Override these mappings? (Old ones will be removed)")
                             .arg(newKeySeq.toString(), conflictList);
            
            if (QMessageBox::question(this, tr("Mapping Conflict"), msg) == QMessageBox::Yes) {
                // EXPLICIT REMOVAL of all conflicts
                QList<ConfigKey> existingActions = transposed.values(val);
                for (const auto& existing : existingActions) {
                    m_pMapping->remove(existing);
                }
            } else {
                return;
            }
        }
    }
    
    QDialog::accept();
}

DlgKeyboardMappingEditor::~DlgKeyboardMappingEditor() {
}

void DlgKeyboardMappingEditor::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Key sequence recording/input
    QHBoxLayout* keyLayout = new QHBoxLayout();
    keyLayout->addWidget(new QLabel(tr("Key Shortcut:"), this));
    m_pKeyEdit = new QLineEdit(this);
    m_pKeyEdit->setText(m_keySeq.toString());
    m_pKeyEdit->setPlaceholderText(tr("Type shortcut or use Learn mode..."));
    keyLayout->addWidget(m_pKeyEdit, 1);
    mainLayout->addLayout(keyLayout);

    // Deck / Group selection
    QHBoxLayout* deckLayout = new QHBoxLayout();
    deckLayout->addWidget(new QLabel(tr("Target Deck/Group:"), this));
    m_pDeckSelector = new QComboBox(this);
    m_pDeckSelector->addItem(tr("Active Focused Deck"), "[ActiveDeck]");
    m_pDeckSelector->addItem("[Channel1]", "[Channel1]");
    m_pDeckSelector->addItem("[Channel2]", "[Channel2]");
    m_pDeckSelector->addItem("[Channel3]", "[Channel3]");
    m_pDeckSelector->addItem("[Channel4]", "[Channel4]");
    m_pDeckSelector->addItem("[Master]", "[Master]");
    m_pDeckSelector->addItem("[Library]", "[Library]");
    m_pDeckSelector->addItem("[Sampler1]", "[Sampler1]");
    deckLayout->addWidget(m_pDeckSelector, 1);
    mainLayout->addLayout(deckLayout);

    // Category
    QHBoxLayout* catLayout = new QHBoxLayout();
    catLayout->addWidget(new QLabel(tr("Category:"), this));
    m_pCategorySelector = new QComboBox(this);
    m_pCategorySelector->addItems(m_pRegistry->getCategories());
    catLayout->addWidget(m_pCategorySelector, 1);
    mainLayout->addLayout(catLayout);

    // Action
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addWidget(new QLabel(tr("Action:"), this));
    m_pActionSelector = new QComboBox(this);
    actionLayout->addWidget(m_pActionSelector, 1);
    mainLayout->addLayout(actionLayout);

    // Description
    m_pDescriptionLabel = new QLabel(this);
    m_pDescriptionLabel->setWordWrap(true);
    m_pDescriptionLabel->setStyleSheet("color: #0080ff; background: #1a1a1a; padding: 10px; border-radius: 4px;");
    mainLayout->addWidget(m_pDescriptionLabel);

    // Deck Assignment & Mirroring
    QGroupBox* deckGroup = new QGroupBox(tr("Deck Assignment & Mirroring"), this);
    deckGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #333; margin-top: 10px; padding-top: 5px; }");
    QHBoxLayout* deckMirrorLayout = new QHBoxLayout(deckGroup);
    
    m_pMirrorDeck1 = new QCheckBox("Deck 1", this);
    m_pMirrorDeck2 = new QCheckBox("Deck 2", this);
    m_pMirrorDeck3 = new QCheckBox("Deck 3", this);
    m_pMirrorDeck4 = new QCheckBox("Deck 4", this);
    
    deckMirrorLayout->addWidget(m_pMirrorDeck1);
    deckMirrorLayout->addWidget(m_pMirrorDeck2);
    deckMirrorLayout->addWidget(m_pMirrorDeck3);
    deckMirrorLayout->addWidget(m_pMirrorDeck4);
    
    mainLayout->addWidget(deckGroup);

    // Pre-select mirroring based on current group if it's a channel
    if (m_currentMapping.group.contains("Channel")) {
        if (m_currentMapping.group == "[Channel1]") m_pMirrorDeck1->setChecked(true);
        else if (m_currentMapping.group == "[Channel2]") m_pMirrorDeck2->setChecked(true);
        else if (m_currentMapping.group == "[Channel3]") m_pMirrorDeck3->setChecked(true);
        else if (m_currentMapping.group == "[Channel4]") m_pMirrorDeck4->setChecked(true);
    }

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_pUnmapButton = new QPushButton(tr("Clear/Unmap Key"), this);
    m_pUnmapButton->setStyleSheet("QPushButton { color: #ff6666; font-weight: bold; }");
    QPushButton* okButton = new QPushButton(tr("Save Mapping"), this);
    okButton->setDefault(true);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addWidget(m_pUnmapButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_pUnmapButton, &QPushButton::clicked, this, &DlgKeyboardMappingEditor::slotUnmap);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_pCategorySelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DlgKeyboardMappingEditor::slotCategoryChanged);
    connect(m_pActionSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DlgKeyboardMappingEditor::slotActionChanged);

    // Pre-select current mapping
    if (!m_currentMapping.group.isEmpty()) {
        int deckIndex = m_pDeckSelector->findData(m_currentMapping.group);
        if (deckIndex >= 0) m_pDeckSelector->setCurrentIndex(deckIndex);
        
        ControlActionInfo info = m_pRegistry->getActionInfo(m_currentMapping);
        int catIndex = m_pCategorySelector->findText(info.category);
        if (catIndex >= 0) m_pCategorySelector->setCurrentIndex(catIndex);
    }
}

void DlgKeyboardMappingEditor::loadActions() {
    slotCategoryChanged(m_pCategorySelector->currentIndex());
}

void DlgKeyboardMappingEditor::slotCategoryChanged(int index) {
    m_pActionSelector->clear();
    QString category = m_pCategorySelector->itemText(index);
    QList<ControlActionInfo> actions = m_pRegistry->getActionsByCategory(category);
    
    for (const auto& action : actions) {
        m_pActionSelector->addItem(action.displayName, action.key.item);
    }

    if (!m_currentMapping.item.isEmpty()) {
        int actionIndex = m_pActionSelector->findData(m_currentMapping.item);
        if (actionIndex >= 0) m_pActionSelector->setCurrentIndex(actionIndex);
    }
}

void DlgKeyboardMappingEditor::slotActionChanged(int index) {
    if (index < 0) return;
    QString item = m_pActionSelector->itemData(index).toString();
    QString category = m_pCategorySelector->currentText();
    
    QList<ControlActionInfo> actions = m_pRegistry->getActionsByCategory(category);
    for (const auto& action : actions) {
        if (action.key.item == item) {
            m_pDescriptionLabel->setText(action.description);
            break;
        }
    }
}

QList<ConfigKey> DlgKeyboardMappingEditor::getSelectedMappings() const {
    QList<ConfigKey> results;
    QString item = m_pActionSelector->currentData().toString();
    QString baseGroup = m_pDeckSelector->currentData().toString();
    
    // If specific deck is selected in checkboxes, override the deck selector
    bool anyMirror = m_pMirrorDeck1->isChecked() || m_pMirrorDeck2->isChecked() || 
                     m_pMirrorDeck3->isChecked() || m_pMirrorDeck4->isChecked();
    
    if (anyMirror && !baseGroup.startsWith("[Channel") && baseGroup != "[ActiveDeck]") {
        // If user wants to mirror, but selected a non-deck group (like [Library]),
        // we should probably still include the library mapping? 
        // Or just prioritize the decks if they are checked.
        results.append(ConfigKey(baseGroup, item));
    }
    
    if (m_pMirrorDeck1->isChecked()) results.append(ConfigKey("[Channel1]", item));
    if (m_pMirrorDeck2->isChecked()) results.append(ConfigKey("[Channel2]", item));
    if (m_pMirrorDeck3->isChecked()) results.append(ConfigKey("[Channel3]", item));
    if (m_pMirrorDeck4->isChecked()) results.append(ConfigKey("[Channel4]", item));
    
    // Fallback if no mirrors checked
    if (results.isEmpty()) {
        results.append(ConfigKey(baseGroup, item));
    }
    
    return results;
}

QKeySequence DlgKeyboardMappingEditor::getSelectedKey() const {
    return QKeySequence(m_pKeyEdit->text());
}

void DlgKeyboardMappingEditor::slotUnmap() {
    if (QMessageBox::question(this, tr("Unmap Key"), 
                             tr("Are you sure you want to remove the mapping for '%1'?")
                             .arg(m_keySeq.toString())) == QMessageBox::Yes) {
        m_isUnmapped = true;
        QDialog::accept();
    }
}
