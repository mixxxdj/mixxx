#include "baseplayer.h"

BasePlayer::BasePlayer(QObject* pParent, const StringAtom& group)
        : QObject(pParent),
          m_group(group) {
}

BasePlayer::~BasePlayer() {

}

const StringAtom& BasePlayer::getGroup() {
    return m_group;
}
