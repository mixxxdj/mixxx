#pragma once
#include "dlgbasesmartiesinfo.h"

class SmartiesFeature;

class dlgSmartiesInfo : public dlgBaseSmartiesInfo {
    Q_OBJECT

  public:
    explicit dlgSmartiesInfo(SmartiesFeature* feature, QWidget* parent = nullptr);
};
