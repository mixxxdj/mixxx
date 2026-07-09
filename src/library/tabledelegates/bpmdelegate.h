#pragma once
#include <memory>

#include "library/tabledelegates/checkboxdelegate.h"

class QItemEditorFactory;

class BPMDelegate : public CheckboxDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);
    ~BPMDelegate() override;

  private:
    std::unique_ptr<QItemEditorFactory> m_pFactory;
};
