#ifndef DLGSAVEDQUERIESEDITOR_H
#define DLGSAVEDQUERIESEDITOR_H

#include "dialog/savedqueries/ui_dlgsavedquerieseditor.h"
#include "library/dao/savedqueriesdao.h"

class TrackCollection;

class DlgSavedQueriesEditor : public QDialog, private Ui::DlgSavedQueriesEditor
{
    Q_OBJECT

  public:
    explicit DlgSavedQueriesEditor(LibraryFeature* pFeature, 
                                   TrackCollection* pTrackCollection, 
                                   QWidget* parent = nullptr);
    
    TrackCollection* m_pTrackCollection;
    SavedQueriesDAO& m_savedDAO;
    LibraryFeature* m_pFeature;
};

#endif // DLGSAVEDQUERIESEDITOR_H
