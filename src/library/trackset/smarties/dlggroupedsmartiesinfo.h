#pragma once
#include "dlgbasesmartiesinfo.h"

class GroupedSmartiesFeature;

class dlgGroupedSmartiesInfo : public dlgBaseSmartiesInfo {
    Q_OBJECT

  public:
    explicit dlgGroupedSmartiesInfo(GroupedSmartiesFeature* feature, QWidget* parent = nullptr);
};
