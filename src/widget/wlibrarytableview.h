// wlibrarytableview.h
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTABLEVIEW_H
#define WLIBRARYTABLEVIEW_H

#include <QString>
#include <QTableView>
#include <QFont>

#include "preferences/usersettings.h"
#include "library/libraryview.h"
#include "track/track.h"
#include "library/coverartcache.h"


class WLibraryTableView : public QTableView, public virtual LibraryView {
    Q_OBJECT

  public:
    WLibraryTableView(QWidget* parent,
                      UserSettingsPointer pConfig,
                      ConfigKey vScrollBarPosKey);
    ~WLibraryTableView() override;
    void moveSelection(int delta) override;

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group,
            bool play = false);
    void trackSelected(TrackPointer pTrack);
    void onlyCachedCoverArt(bool);
    void scrollValueChanged(int);

  public slots:
    void saveVScrollBarPos();
    void restoreVScrollBarPos();
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);

  private:
    void loadVScrollBarPosState();
    void saveVScrollBarPosState();

    UserSettingsPointer m_pConfig;
    ConfigKey m_vScrollBarPosKey;
    // The position of the vertical scrollbar slider, eg. before a search is
    // executed
    int m_iSavedVScrollBarPos;
};


#endif /* WLIBRARYTABLEVIEW_H */
