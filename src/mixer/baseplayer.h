#ifndef MIXER_BASEPLAYER_H
#define MIXER_BASEPLAYER_H

#include <QByteArrayData>
#include <QObject>
#include <QString>

class BasePlayer : public QObject {
    Q_OBJECT
  public:
    BasePlayer(QObject* pParent, const QString& group);
    ~BasePlayer() override = default;

    inline const QString& getGroup() {
        return m_group;
    }

  private:
    const QString m_group;
};

#endif /* MIXER_BASEPLAYER_H */
