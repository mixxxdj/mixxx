#pragma once

#include <QLabel>

#include "widget/wnumber.h"
#include "skin/legacy/skincontext.h"

class WNumberDb : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberDb(QWidget* pParent = nullptr);

  public slots:
    void setValue(double dValue) override;
};
