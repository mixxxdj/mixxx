#pragma once

#include "library/tabledelegates/checkboxdelegate.h"

class BPMDelegate : public CheckboxDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);

  private:
    QItemEditorFactory* m_pFactory;
};
