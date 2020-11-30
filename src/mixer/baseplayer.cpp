#include "mixer/baseplayer.h"

BasePlayer::BasePlayer(QObject* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
}
