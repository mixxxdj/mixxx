#ifndef BASEPLAYER_H
#define BASEPLAYER_H

#include <QObject>
#include <QString>
#include "control/stringatom.h"

class BasePlayer : public QObject {
    Q_OBJECT
  public:
    BasePlayer(QObject* pParent, QString group);
    virtual ~BasePlayer();

    const StringAtom& getGroup();

  private:
    const StringAtom m_group;
};

#endif /* BASEPLAYER_H */
