#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>

#include "preferences/colorpaletteeditormodel.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class ColorPaletteEditor : public QDialog {
    Q_OBJECT
  public:
    ColorPaletteEditor(QWidget* parent = nullptr, bool showHotcueNumbers = true);
    void initialize(UserSettingsPointer pConfig, const QString& paletteName);

  signals:
    void paletteChanged(const QString& name);
    void paletteRemoved(const QString& name);
    void closeButtonClicked();

  private slots:
    void slotUpdateButtons();
    void slotTableViewDoubleClicked(const QModelIndex& index);
    void slotTableViewContextMenuRequested(const QPoint& pos);
    void slotPaletteNameChanged(const QString& text);
    void slotCloseButtonClicked();
    void slotSaveButtonClicked();
    void slotResetButtonClicked();
    void slotRemoveButtonClicked();

  private:
    bool m_bPaletteExists;
    bool m_bPaletteIsReadOnly;

    UserSettingsPointer m_pConfig;
    parented_ptr<QLineEdit> m_pSaveAsEdit;
    parented_ptr<QTableView> m_pTableView;
    parented_ptr<ColorPaletteEditorModel> m_pModel;
    QPushButton* m_pSaveButton;
    QPushButton* m_pCloseButton;
    QPushButton* m_pRemoveButton;
    QPushButton* m_pResetButton;
    QString m_resetPalette;
};
