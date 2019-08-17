#pragma once

#include <QMenu>

#include "util/color/color.h"

class ColorMenu : public QMenu {
    Q_OBJECT
  public:
    ColorMenu(QWidget *parent = nullptr,
            PredefinedColorsRepresentation* pColorRepresentation = nullptr);
    ~ColorMenu() override;

    void useColorSet(PredefinedColorsRepresentation* pColorRepresentation);

  signals:
    void colorPicked(PredefinedColorPointer pColor);

  private:
    void clear();
    QList<QAction*> m_pColorActions;
};
