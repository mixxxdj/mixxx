#include "mixer/baseplayer.h"

#include "mixer/playermanager.h"
#include "moc_baseplayer.cpp"

BasePlayer::BasePlayer(PlayerManager* pParent, const QString& group)
        : QObject(pParent),
          m_pPlayerManager(pParent),
          m_group(group) {
}
