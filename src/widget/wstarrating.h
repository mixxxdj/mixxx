#ifndef WSTARRATING_H
#define WSTARRATING_H

#include <QEvent>
#include <QMouseEvent>
#include <QStylePainter>

#include "skin/skincontext.h"
#include "trackinfoobject.h"

#include "library/starrating.h"
#include "widget/wbasewidget.h"

class WStarRating : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WStarRating(QString group, QWidget* pParent);
    virtual ~WStarRating();

    virtual void setup(QDomNode node, const SkinContext& context);
    QSize sizeHint() const;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateRating(TrackInfoObject*);
    
  protected:
    virtual void paintEvent(QPaintEvent* e);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);
    
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
