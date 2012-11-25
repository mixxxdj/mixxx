#ifndef PREVIEWDECKBUTTONHANDLER_H
#define PREVIEWDECKBUTTONHANDLER_H

#include <QObject>
#include <QTableView>

#include "library/trackmodel.h"

class PreviewDeckButtonHandler : public QObject {
  Q_OBJECT
  public :
    PreviewDeckButtonHandler(QObject *parent,
                             const QModelIndex &index,
                             QTableView *pTableView);
    virtual ~PreviewDeckButtonHandler();

  signals:
    void loadTrackToPlayer(TrackPointer Track, QString group);

  public slots:
    void buttonclicked();

  private:
    QModelIndex m_index;
    QTableView *m_pTableView;
};

#endif
