#include "junction.h"

IdcmpJunction::IdcmpJunction(PlayerManager *pPlayerManager)
        : QObject(),
          m_pPlayerManager(pPlayerManager) {

    for (unsigned int deckNum = 1; deckNum <= pPlayerManager->numberOfDecks(); deckNum++) {
        Deck *pDeck = pPlayerManager->getDeck(deckNum);

        if (pDeck) {
            ControlProxy *eot = new ControlProxy(pDeck->getGroup(), "end_of_track", this);
            eot->connectValueChanged(SLOT(onEndOfTrackChange(double)));
        }
    }

    m_pThread = new QThread;
    m_pThread->setObjectName("IdcmpThread");
    moveToThread(m_pThread);

    connect(this, SIGNAL(requestShutdown()), this, SLOT(slotShutdown()));
    connect(m_pThread, SIGNAL(started()), this, SLOT(threadStarted()));

    m_pThread->start(QThread::LowPriority);

}

// this runs in my own thread
void IdcmpJunction::onEndOfTrackChange(double v) {
    QFile file("/sys/kernel/debug/ec/ec0/io");

    if (!file.open(QIODevice::ReadWrite)) {
        qWarning() << "Could not open file " << file.fileName() << " error code: " << file.error();
        return;
    }

    file.seek(12);
    // thinklight off or thinklight blink
    file.putChar((v == 0) ? 0x0a : 0xca);
    file.flush();
    file.close();
}

IdcmpJunction::~IdcmpJunction() {
    emit(requestShutdown());
    m_pThread->wait();
    delete m_pThread;
}

void IdcmpJunction::threadStarted() {
    // nothing to do for now
}

void IdcmpJunction::slotShutdown() {
    m_pThread->quit();
}
