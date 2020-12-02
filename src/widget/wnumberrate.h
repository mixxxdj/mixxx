//
// C++ Interface: wnumberpos
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WNUMBERRATE_H
#define WNUMBERRATE_H

#include <QByteArrayData>
#include <QDomNode>
#include <QString>

#include "skin/skincontext.h"
#include "widget/wnumber.h"

class ControlProxy;
class QObject;
class QWidget;

class WNumberRate final : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberRate(const QString& group, QWidget* parent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue) override;

  private:
    ControlProxy* m_pRateRatio;
};

#endif
