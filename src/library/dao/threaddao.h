#ifndef THREADDAO_H
#define THREADDAO_H

#include <QThread>
#include "dao.h"



// Lambda function
typedef std::function <void ()> func;

class QWidget;

// tro: Class for separate thread. It will do empty cycles until got some work.
//      All database access must be done trough this thread.
class ThreadDAO : public QThread {
    Q_OBJECT

    bool m_haveFunction;
    bool m_callFinished;
    bool m_stop;

    func m_lambda;
public:
    explicit ThreadDAO(QObject *parent = 0);
    void run();
    void callSync(func lambda, QWidget& w);
    void stopThread();
    void setLambda(func lambda);
};

#endif // THREADDAO_H
