#ifndef WSTARRATING_H
#define WSTARRATING_H

#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "configobject.h"
#include "skin/skincontext.h"
#include "trackinfoobject.h"

#include "library/stardelegate.h"
#include "widget/wwidget.h"

class WStarRating : public WWidget {
    Q_OBJECT
  public:
    WStarRating(const char* group, ConfigObject<ConfigValue>* pConfig, QWidget* pParent);
    // WStarRating(QWidget* pParent=NULL);
    virtual ~WStarRating();

    virtual void setup(QDomNode node, const SkinContext& context);
    
    // void render (
		// QPainter * painter,
		// const QPoint & targetOffset = QPoint(),
		// const QRegion & sourceRegion = QRegion(),
		// RenderFlags renderFlags = RenderFlags( DrawWindowBackground | DrawChildren ) );

  public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotTrackUnloaded(TrackPointer track);

  private slots:
    void updateRating(TrackInfoObject*);
    
  protected:
    bool event(QEvent* pEvent);
    void fillDebugTooltip(QStringList* debug);
    
    virtual void paintEvent(QPaintEvent* e);
    // virtual void resizeEvent(QResizeEvent* e);
    
    // Foreground and background colors.
    QColor m_qFgColor;
    QColor m_qBgColor;
    
    const char* m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    TrackPointer m_pCurrentTrack;
    QString m_property;
};

#endif /* WSTARRATING_H */
