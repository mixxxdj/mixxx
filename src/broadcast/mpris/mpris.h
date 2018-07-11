#ifndef MPRIS_H
#define MPRIS_H

#include <QObject>

class MixxxMainWindow;

class Mpris : public QObject
{
    Q_OBJECT
  public:
    explicit Mpris(MixxxMainWindow* mixxx);
    ~Mpris();
};

#endif // MPRIS_H
