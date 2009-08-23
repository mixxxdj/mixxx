// library.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

// A Library class is a container for all the model-side aspects of the library.
// A library widget can be attached to the Library object by calling bindWidget.

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QList>
#include <QObject>
#include <QAbstractItemModel>

class TrackModel;
class TrackCollection;
class SidebarModel;
class LibraryFeature;
class LibraryTableModel;
class WTrackSourcesView;
class WTrackTableView;

class Library : public QObject {
    Q_OBJECT
public:
    Library(QObject* parent = NULL);
    virtual ~Library();
    
    //void bindWidget(LibraryWidget* widget);
    void bindWidget(WTrackSourcesView* sourcesView, WTrackTableView* tableView);
    void addFeature(LibraryFeature* feature);
    
    // TODO(rryan) Transitionary only -- the only reason this is here is so the
    // waveform widgets can signal to a player to load a track. This can be
    // fixed by moving the waveform renderers inside player and connecting the
    // signals directly.
    TrackCollection* getTrackCollection() {
        return m_pTrackCollection;
    }
    
    //static Library* buildDefaultLibrary();

public slots:
    void slotShowTrackModel(QAbstractItemModel* model);
signals:
    void showTrackModel(QAbstractItemModel* model);
    
private:
    SidebarModel* m_pSidebarModel;
    TrackCollection* m_pTrackCollection;
    QList<LibraryFeature*> m_sFeatures;
    LibraryTableModel* m_pLibraryTableModel;
};

#endif /* LIBRARY_H */
