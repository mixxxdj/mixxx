// wtracktableviewheader.h
// Created 1/2/2010 by RJ Ryan (rryan@mit.edu)

#ifndef WTRACKTABLEVIEWHEADER_H
#define WTRACKTABLEVIEWHEADER_H

#include <QAction>
#include <QHeaderView>
#include <QMap>
#include <QMenu>
#include <QSignalMapper>
#include <QWidget>
#include <QContextMenuEvent>

class TrackModel;

class WTrackTableViewHeader : public QHeaderView {
    Q_OBJECT
  public:
    WTrackTableViewHeader(Qt::Orientation orientation, QWidget* parent = 0);
    virtual ~WTrackTableViewHeader();

    void contextMenuEvent(QContextMenuEvent* event);
    virtual void setModel(QAbstractItemModel* model);

    void saveHeaderState();
    void restoreHeaderState();
     /** returns false if the header state is stored in the database (on first time usgae) **/
    bool hasPersistedHeaderState();

  private slots:
    void showOrHideColumn(int);

  private:
    int hiddenCount();
    void clearActions();
    TrackModel* getTrackModel();

    QMenu m_menu;
    QMap<int, QAction*> m_columnActions;
    QSignalMapper m_signalMapper;
};

#endif /* WTRACKTABLEVIEWHEADER_H */
