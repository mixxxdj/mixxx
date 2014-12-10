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

#include "proto/headers.pb.h"

class TrackModel;

// Thanks to StackOverflow http://stackoverflow.com/questions/1163030/qt-qtableview-and-horizontalheader-restorestate
// answer with this code snippet: http://codepad.org/2gPIMPYU
class HeaderViewState {
public:
    HeaderViewState() {}

    // Populate the object based on the provided live view.
    explicit HeaderViewState(const QHeaderView& headers);

    // Populate from an existing protobuf, mostly for testing.
    explicit HeaderViewState(const mixxx::library::HeaderViewState& pb)
            : m_view_state(pb) { }

    // Populate the object with the serialized protobuf data provided.
    HeaderViewState(const QString& serialized);

    // Returns a serialized protobuf of the current state.
    QString saveState() const;
    // Apply the state to the provided view.  The data in the object may be
    // changed if the header format has changed.
    void restoreState(QHeaderView* headers);

private:
    mixxx::library::HeaderViewState m_view_state;
};


class WTrackTableViewHeader : public QHeaderView {
    Q_OBJECT
  public:
    WTrackTableViewHeader(Qt::Orientation orientation, QWidget* parent = 0);
    virtual ~WTrackTableViewHeader();

    void contextMenuEvent(QContextMenuEvent* event);
    virtual void setModel(QAbstractItemModel* model);

    void saveHeaderState();
    void restoreHeaderState();
    void loadDefaultHeaderState();
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
