#pragma once

#include <QMenu>

#include "util/color/color.h"

class ColorMenu : public QMenu {
    Q_OBJECT
  public:
    ColorMenu(QWidget *parent = nullptr);
    ~ColorMenu() override;

  signals:
    void colorPicked(PredefinedColorPointer pColor);

  private:
    QList<QAction*> m_pColorActions;
};
