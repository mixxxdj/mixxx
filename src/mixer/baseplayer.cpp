#include "mixer/baseplayer.h"

BasePlayer::BasePlayer(QObject* pParent, QString group)
        : QObject(pParent),
          m_group(group) {
}
