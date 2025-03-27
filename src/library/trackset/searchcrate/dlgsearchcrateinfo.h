#pragma once
#include "dlgbasesearchcrateinfo.h"

class SearchCrateFeature;

class dlgSearchCrateInfo : public dlgBaseSearchCrateInfo {
    Q_OBJECT

  public:
    explicit dlgSearchCrateInfo(SearchCrateFeature* feature, QWidget* parent = nullptr);
};
