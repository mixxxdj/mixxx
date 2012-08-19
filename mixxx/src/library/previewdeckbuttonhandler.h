#ifndef PREVIEWDECKBUTTONHANDLER_H
#define PREVIEWDECKBUTTONHANDLER_H

#include <QObject>
#include <QTableView>

#include "library/trackmodel.h"

#define PLAYING 1
#define STOP 0

class PreviewdeckButtonHandler : public QObject {
  Q_OBJECT
  public :
    PreviewdeckButtonHandler(QObject *parent,
                             const QModelIndex &index,
                             QTableView *pTableView);
    virtual ~PreviewdeckButtonHandler();

  signals:
    void loadTrackToPlayer(TrackPointer Track, QString group);

  public slots:
    void buttonclicked();

  private:
    QModelIndex m_index;
    QTableView *m_pTableView;
};

#endif
