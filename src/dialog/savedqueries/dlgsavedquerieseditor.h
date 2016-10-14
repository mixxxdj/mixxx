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

  public slots:

    void accept() override;

  private slots:
    
    void removeQuery();

  private:

    TrackCollection* m_pTrackCollection;
    LibraryFeature* m_pFeature;
};

#endif // DLGSAVEDQUERIESEDITOR_H
