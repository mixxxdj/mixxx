#include "mixer/baseplayer.h"

#include "moc_baseplayer.cpp"

BasePlayer::BasePlayer(QObject* pParent, const QString& group)
        : QObject(pParent),
          m_group(group) {
}
