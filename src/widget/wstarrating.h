#ifndef WSTARRATING_H
#define WSTARRATING_H

#include <QEvent>
#include <QMouseEvent>
#include <QStylePainter>

#include "skin/skincontext.h"
#include "track/track.h"

#include "library/starrating.h"
#include "widget/wwidget.h"

class WStarRating : public WWidget {
    Q_OBJECT
  public:
    WStarRating(QString group, QWidget* pParent);

    virtual void setup(const QDomNode& node, const SkinContext& context);
    QSize sizeHint() const override;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack = TrackPointer());

  private slots:
    void updateRating(Track*);

  protected:
    void paintEvent(QPaintEvent* e) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent * /*unused*/) override;
    void fillDebugTooltip(QStringList* debug) override;

    StarRating m_starRating;
    QString m_pGroup;
    TrackPointer m_pCurrentTrack;
    bool m_focused;
    mutable QRect m_contentRect;

    private:
        void updateRating();
        int starAtPosition(int x);
};

#endif /* WSTARRATING_H */
