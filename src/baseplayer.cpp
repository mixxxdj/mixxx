#include "baseplayer.h"

BasePlayer::BasePlayer(QObject* pParent, QString group)
        : QObject(pParent),
          m_group(group) {
}

BasePlayer::~BasePlayer() {

}

const StringAtom& BasePlayer::getGroup() {
    return m_group;
}
