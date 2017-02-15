#ifndef WNUMBERBEATSIZE_H
#define WNUMBERBEATSIZE_H

#include "widget/wnumber.h"

class ControlProxy;

class WNumberBeatSize : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberBeatSize(QWidget *parent=nullptr);

  private slots:
    void setValue(double dValue) override;

  private:
    QString fractionString(int numerator, int denominator);

    ControlProxy* m_pBeatloopSize;
};

#endif
