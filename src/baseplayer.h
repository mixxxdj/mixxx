#ifndef BASEPLAYER_H
#define BASEPLAYER_H

#include <QObject>
#include <QString>

class BasePlayer : public QObject {
    Q_OBJECT
  public:
    BasePlayer(QObject* pParent, QString group);
    virtual ~BasePlayer();

    const QString getGroup() const;

  private:
    const QString m_group;
};

#endif /* BASEPLAYER_H */
