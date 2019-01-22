
#ifndef IDCMP_JUNCTION_H
#define IDCMP_JUNCTION_H

#include <QThread>

#include "mixer/playermanager.h"
#include "mixer/deck.h"

class PlayerManager;
class Deck;

class IdcmpJunction : public QObject {
    Q_OBJECT
public:
    IdcmpJunction(PlayerManager *playerManager);
    virtual ~IdcmpJunction();

    signals:
    void requestShutdown();

public slots:
        void slotShutdown();

private slots:
        void threadStarted();
        void onEndOfTrackChange(double);

private:
    PlayerManager *m_pPlayerManager;
    QThread* m_pThread;

};

#endif //IDCMP_JUNCTION_H
