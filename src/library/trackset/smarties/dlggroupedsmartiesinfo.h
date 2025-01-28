#pragma once
#include "dlgBaseSmartiesInfo.h"

class GroupedSmartiesFeature;

class dlgGroupedSmartiesInfo : public dlgBaseSmartiesInfo {
    Q_OBJECT

  public:
    explicit dlgGroupedSmartiesInfo(GroupedSmartiesFeature* feature, QWidget* parent = nullptr);
};
