#pragma once

#include <QDialog>
#include <QKeySequence>
#include <QSharedPointer>

#include "preferences/configobject.h"
#include "controllers/keyboard/keyboardactionregistry.h"

class QComboBox;
class QLineEdit;
class QLabel;

class DlgKeyboardMappingEditor : public QDialog {
    Q_OBJECT

  public:
    DlgKeyboardMappingEditor(QWidget* parent,
                           const QKeySequence& keySeq,
                           const ConfigKey& currentMapping,
                           KeyboardActionRegistry* pRegistry,
                           ConfigObject<ConfigValueKbd>* pMapping = nullptr);
    ~DlgKeyboardMappingEditor() override;

    QKeySequence getSelectedKey() const;
    QList<ConfigKey> getSelectedMappings() const;

    void accept() override;
    bool isUnmapped() const { return m_isUnmapped; }

  private slots:
    void slotCategoryChanged(int index);
    void slotActionChanged(int index);
    void slotUnmap();

  private:
    void setupUi();
    void loadActions();

    QKeySequence m_keySeq;
    ConfigKey m_currentMapping;
    KeyboardActionRegistry* m_pRegistry;
    ConfigObject<ConfigValueKbd>* m_pMapping;

    QComboBox* m_pCategorySelector;
    QComboBox* m_pActionSelector;
    QComboBox* m_pDeckSelector;
    QLineEdit* m_pKeyEdit;
    QLabel* m_pDescriptionLabel;
    QPushButton* m_pUnmapButton;

    // Deck Mirroring
    class QCheckBox* m_pMirrorDeck1;
    class QCheckBox* m_pMirrorDeck2;
    class QCheckBox* m_pMirrorDeck3;
    class QCheckBox* m_pMirrorDeck4;
    bool m_isUnmapped;
};
