#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class ElapsedTimer {
  public:
    ElapsedTimer();
    virtual ~ElapsedTimer();
    virtual void invalidate() = 0;
    virtual bool isValid() = 0;
    virtual void start() = 0;
    virtual qint64 elapsed() = 0;
};

class Timer : public QObject {
    Q_OBJECT
  public:
    virtual ~Timer();
    virtual void start(int msec) = 0;
    virtual bool isActive() = 0;
  public slots:
    virtual void stop() = 0;
  signals:
    void timeout();
};

class TimerQt : public Timer {
    Q_OBJECT
  public:
    TimerQt(QObject *parent = nullptr);
    ~TimerQt() override;
    void start(int msec) override;
    bool isActive() override;
    void stop() override;
  private:
    QTimer m_Timer;    
};

class ElapsedTimerQt : public ElapsedTimer {
  public:
    ~ElapsedTimerQt() override;
    void invalidate() override;
    bool isValid() override;
    void start() override;
    qint64 elapsed() override;
  private:
    QElapsedTimer m_elapsedTimer;
};