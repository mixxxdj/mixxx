
#ifndef WNUMBERDB_H
#define WNUMBERDB_H

#include <QLabel>

#include "widget/wnumber.h"
#include "skin/skincontext.h"

class WNumberDb : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberDb(QWidget* pParent = nullptr);

  public slots:
    void setValue(double dValue) override;
};

#endif // WNUMBERDB_H
