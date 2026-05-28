#pragma once

#include "library/librarytablemodel.h"

class AnalysisLibraryTableModel : public LibraryTableModel {
    Q_OBJECT
  public:
    static constexpr unsigned int kDefaultRecentDays = 7;

    AnalysisLibraryTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~AnalysisLibraryTableModel() override = default;

    void setRecentDays(unsigned int days);
    unsigned int recentDays() const {
        return m_recentDays;
    }

    void showRecentSongs();
    void showAllSongs();
    void searchCurrentTrackSet(const QString& text, bool useRecentFilter);

  private:
    unsigned int m_recentDays;
};
