#pragma once

#include <QPoint>

class WaveformElementRightClickable {
  public:
    virtual bool contains(QPoint point, Qt::Orientation orientation) const = 0;
};
