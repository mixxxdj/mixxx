#pragma once

#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QWidget>

#include "preferences/colorpaletteeditormodel.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class ColorPaletteEditor : public QWidget {
    Q_OBJECT
  public:
    ColorPaletteEditor(QWidget* parent = nullptr);
    void setConfig(UserSettingsPointer pConfig) {
        m_pConfig = pConfig;
        reset();
    }
    void reset();

  signals:
    void paletteChanged(QString name);
    void paletteRemoved(QString name);

  private slots:
    void slotUpdateButtons();

  private:
    bool m_bPaletteExists;
    bool m_bPaletteIsReadOnly;

    UserSettingsPointer m_pConfig;
    parented_ptr<QComboBox> m_pPaletteNameComboBox;
    parented_ptr<QTableView> m_pTableView;
    parented_ptr<ColorPaletteEditorModel> m_pModel;
    QPushButton* m_pSaveButton;
    QPushButton* m_pDiscardButton;
    QPushButton* m_pResetButton;
};
