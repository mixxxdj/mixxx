#include "baseplayer.h"

BasePlayer::BasePlayer(QObject* pParent, QString group)
        : QObject(pParent),
          m_group(group) {
}

BasePlayer::~BasePlayer() {

}

const QString BasePlayer::getGroup() const {
    return m_group;
}
