#pragma once

#include <QDialog>

#include "aoide/tracksearchoverlayfilter.h"
#include "aoide/ui_tracksearchoverlayfilterdlg.h"

namespace aoide {

class TrackSearchOverlayFilterDlg : public QDialog, public Ui::AoideTrackSearchOverlayFilterDlg {
    Q_OBJECT
  public:
    explicit TrackSearchOverlayFilterDlg(
            TrackSearchOverlayFilter overlayFilter,
            QWidget* parent = nullptr);
    ~TrackSearchOverlayFilterDlg() override = default;

    const TrackSearchOverlayFilter& overlayFilter() const {
        return m_overlayFilter;
    }

  public slots:
    void accept() override;
    void reject() override;

  private:
    void init();
    void reset();
    void apply();

    TrackSearchOverlayFilter m_overlayFilter;
};

} // namespace aoide
