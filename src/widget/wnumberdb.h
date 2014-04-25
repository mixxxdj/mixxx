
#ifndef WNUMBERDB_H
#define WNUMBERDB_H

#include <QLabel>

#include "widget/wlabel.h"
#include "skin/skincontext.h"

class WNumberDb : public WLabel  {
    Q_OBJECT
  public:
    WNumberDb(QWidget* pParent=NULL);
    virtual ~WNumberDb();

    virtual void setup(QDomNode node, const SkinContext& context);

    virtual void onConnectedControlChanged(double dParameter, double dValue);

  public slots:
    virtual void setValue(double dValue);

  protected:
    // Number of digits to round to.
    int m_iNoDigits;
};

#endif // WNUMBERDB_H
