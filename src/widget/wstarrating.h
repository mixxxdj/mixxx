#ifndef WSTARRATING_H
#define WSTARRATING_H

#include <QEvent>
#include <QMouseEvent>
#include <QStylePainter>

#include "configobject.h"
#include "skin/skincontext.h"
#include "trackinfoobject.h"

#include "library/starrating.h"
#include "widget/wbasewidget.h"

class WStarRating : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WStarRating(QString group, ConfigObject<ConfigValue>* pConfig, QWidget* pParent);
    virtual ~WStarRating();

    virtual void setup(QDomNode node, const SkinContext& context);
    
    QSize sizeHint() const;

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateRating(TrackInfoObject*);
    
  protected:
    // bool event(QEvent* pEvent);
    void fillDebugTooltip(QStringList* debug);
    
    virtual void paintEvent(QPaintEvent* e);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);
    
    // virtual void resizeEvent(QResizeEvent* e);
    
    StarRating m_starRating;
    QString m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pCurrentTrack;
    bool m_focused;
    QRect m_contentRect;
    
    private:
        void updateRating();
        int starAtPosition(int x);
        // QSize m_widgetSize;
};

#endif /* WSTARRATING_H */
