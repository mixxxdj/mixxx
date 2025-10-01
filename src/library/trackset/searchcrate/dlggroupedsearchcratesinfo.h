#pragma once
#include "dlgbasesearchcrateinfo.h"

class GroupedSearchCratesFeature;

class dlgGroupedSearchCratesInfo : public dlgBaseSearchCrateInfo {
    Q_OBJECT

  public:
    explicit dlgGroupedSearchCratesInfo(
            GroupedSearchCratesFeature* feature, QWidget* parent = nullptr);
};
