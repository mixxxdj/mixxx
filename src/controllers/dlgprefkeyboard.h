#pragma once

#include <QWidget>
#include <QSharedPointer>

#include "controllers/keyboard/keyboardmappingmanager.h"
#include "controllers/keyboard/keyboardactionregistry.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"

class QTableWidget;
class QPushButton;
class QComboBox;
class QLabel;

/// Preferences page for keyboard mapping
class DlgPrefKeyboard : public DlgPreferencePage {
    Q_OBJECT

  public:
    DlgPrefKeyboard(QWidget* parent,
                    UserSettingsPointer pConfig,
                    KeyboardEventFilter* pKeyboardEventFilter);
    ~DlgPrefKeyboard() override;

    QUrl helpUrl() const override;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;
    void slotCancel() override;

  private slots:
    void slotAddMapping();
    void slotRemoveMapping();
    void slotEditMapping();
    void slotToggleLearning(bool enabled);
    void slotMappingSelected(int index);
    void slotSaveMapping();
    void slotExportJson();
    void slotExportXml();
    void slotShowVisualLayout();
    void slotVisualKeyClicked(QKeySequence seq);
    void slotKeyCaptured(QKeySequence keySequence);
    void slotOpenDocumentation();
    void slotLoadMappingFromFile();

  private:
    void setupUi();
    void loadMappingToTable(const QSharedPointer<ConfigObject<ConfigValueKbd>>& pMapping);
    void saveMappingFromTable();
    QString getDeckColor(const QString& group);

    UserSettingsPointer m_pConfig;
    KeyboardEventFilter* m_pKeyboardEventFilter;
    QSharedPointer<KeyboardMappingManager> m_pMappingManager;
    QSharedPointer<KeyboardActionRegistry> m_pActionRegistry;
    QSharedPointer<ConfigObject<ConfigValueKbd>> m_pCurrentMapping;

    // UI elements
    QTableWidget* m_pMappingTable;
    QPushButton* m_pAddButton;
    QPushButton* m_pRemoveButton;
    QPushButton* m_pEditButton;
    QPushButton* m_pLearnButton;
    QPushButton* m_pSaveButton;
    QPushButton* m_pExportJsonButton;
    QPushButton* m_pExportXmlButton;
    QPushButton* m_pVisualLayoutButton;
    QPushButton* m_pLoadFileButton;
    QPushButton* m_pDocsButton;
    QComboBox* m_pMappingSelector;
    QLabel* m_pStatusLabel;

    bool m_bChanged;
    QString m_currentMappingPath;
};
