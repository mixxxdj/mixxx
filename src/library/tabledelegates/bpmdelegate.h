#pragma once

#include <QItemEditorFactory>
#include <memory>

#include "library/tabledelegates/checkboxdelegate.h"

class BPMDelegate : public CheckboxDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);
    ~BPMDelegate() override = default;

  private:
    std::unique_ptr<QItemEditorFactory> m_pFactory;
};
