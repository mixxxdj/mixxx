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

// Thanks to StackOverflow http://stackoverflow.com/questions/1163030/qt-qtableview-and-horizontalheader-restorestate
// answer with this code snippet: http://codepad.org/2gPIMPYU
class QHeaderViewState {
public:
    QHeaderViewState()
            : m_sort_indicator_shown(true), m_sort_indicator_section(0),
              m_sort_order(Qt::DescendingOrder) {}

    // Populate the object based on the provided live view.
    explicit QHeaderViewState(const QHeaderView& headers);

    // Populate the object with the serialized protobuf data provided.
    QHeaderViewState(const QString& serialized);

    // Returns a serialized protobuf of the current state.
    QString saveState() const;
    // Apply the state to the provided view.
    void restoreState(QHeaderView* headers) const;

private:
    struct HeaderState {
        bool hidden;
        int size;
        int logical_index;
        int visual_index;
        int column_id;

        HeaderState()
                : hidden(false),
                  size(50),
                  logical_index(0),
                  visual_index(0),
                  column_id(0) {}
    };

    // Header information is stored in visual index order
    QList<HeaderState> m_headers;
    bool m_sort_indicator_shown;
    int m_sort_indicator_section;
    Qt::SortOrder m_sort_order;  // iff m_sort_indicator_shown
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
