#pragma once

#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QWidget>

#include "preferences/colorpaletteeditormodel.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

// Widget for viewing, adding, editing and removing color palettes that can be
// used for track/hotcue colors.
class ColorPaletteEditor : public QWidget {
    Q_OBJECT
  public:
    ColorPaletteEditor(QWidget* parent = nullptr);
    void initialize(UserSettingsPointer pConfig);
    void reset();

  signals:
    void paletteChanged(QString name);
    void paletteRemoved(QString name);
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
    parented_ptr<QComboBox> m_pPaletteTemplateComboBox;
    parented_ptr<QComboBox> m_pSaveAsComboBox;
    parented_ptr<QTableView> m_pTableView;
    parented_ptr<ColorPaletteEditorModel> m_pModel;
    QPushButton* m_pSaveButton;
    QPushButton* m_pCloseButton;
    QPushButton* m_pRemoveButton;
    parented_ptr<QPushButton> m_pResetButton;
};
