#pragma once
#include <QTableView>

#include "preferences/colorpaletteeditormodel.h"
#include "util/color/colorpalette.h"

class ColorPaletteEditorTableView : public QTableView {
    Q_OBJECT
  public:
    ColorPaletteEditorTableView(QWidget* parent = nullptr);

    ~ColorPaletteEditorTableView() {
        delete m_model;
    }

    void setDirty(bool bDirty) {
        if (m_bDirty == bDirty) {
            return;
        }
        m_bDirty = bDirty;
        emit dirtyChanged(m_bDirty);
    }

    bool isDirty() {
        return m_bDirty;
    }

    ColorPalette getColorPalette(const QString& name) const;
    void setColorPalette(const ColorPalette& palette);

  signals:
    void dirtyChanged(bool bDirty);

  private:
    bool m_bDirty;
    ColorPaletteEditorModel* m_model;
};
