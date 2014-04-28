
#ifndef WNUMBERDB_H
#define WNUMBERDB_H

#include <QLabel>

#include "widget/wnumber.h"
#include "skin/skincontext.h"

class WNumberDb : public WNumber {
    Q_OBJECT
  public:
    WNumberDb(QWidget* pParent = NULL);
    virtual ~WNumberDb();

  public slots:
    virtual void setValue(double dValue);
};

#endif // WNUMBERDB_H
