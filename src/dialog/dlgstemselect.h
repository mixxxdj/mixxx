#pragma once

#include <QDialog>

#include "dialog/ui_dlgstemselect.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class WCoverArtLabel;
class CoverInfo;

class DlgStemSelect : public QDialog, public Ui::DlgStemSelect {
    Q_OBJECT

  public:
    explicit DlgStemSelect(QWidget* parent);
    ~DlgStemSelect() = default;
    void show(TrackPointer pTrack);

  protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

  private slots:
    void slotStemMixToggled(bool checked);
    void slotStemChecked(int state);
    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);

  private:
    TrackPointer m_pTrack;
    parented_ptr<WCoverArtLabel> m_pWCoverArtLabel;
};
