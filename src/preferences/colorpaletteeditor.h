#pragma once

#include <QComboBox>
#include <QPushButton>
#include <QWidget>

#include "preferences/colorpaletteeditortableview.h"
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

  private:
    UserSettingsPointer m_pConfig;
    parented_ptr<QComboBox> m_pPaletteNameComboBox;
    parented_ptr<QPushButton> m_pSaveButton;
    parented_ptr<QPushButton> m_pDeleteButton;
    parented_ptr<QPushButton> m_pResetButton;
    parented_ptr<ColorPaletteEditorTableView> m_pTableView;
};
