//
// C++ Interface: wnumberpos
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#pragma once

#include "widget/wnumber.h"

class ControlProxy;

class WNumberRate final : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberRate(const QString& group, QWidget* parent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue) override;

  private:
    ControlProxy* m_pRateRatio;
};
