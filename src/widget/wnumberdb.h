
#ifndef WNUMBERDB_H
#define WNUMBERDB_H

#include <QByteArrayData>
#include <QLabel>
#include <QString>

#include "skin/skincontext.h"
#include "widget/wnumber.h"

class QObject;
class QWidget;

class WNumberDb : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberDb(QWidget* pParent = nullptr);

  public slots:
    void setValue(double dValue) override;
};

#endif // WNUMBERDB_H
