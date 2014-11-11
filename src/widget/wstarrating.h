#ifndef WSTARRATING_H
#define WSTARRATING_H

#include <QEvent>
#include <QMouseEvent>

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
    

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    // void updateRating(TrackInfoObject*);
    void updateRating();
    
  protected:
    bool event(QEvent* pEvent);
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
    QString m_property;
    int m_focused;
    QRect m_contentRect;
    
    private:
        int starAtPosition(int x);
};

#endif /* WSTARRATING_H */
