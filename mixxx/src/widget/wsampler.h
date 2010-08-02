#ifndef WSAMPLER_H
#define WSAMPLER_H

#include "wwidget.h"
#include <qwidget.h>
#include <qlabel.h>
#include <qstring.h>
#include <Q3ValueList>
#include <QList>
#include "trackinfoobject.h"

class ControlObject;
class WSlider;
class WSliderComposed;
class WPushButton;
class WDisplay;
class WKnob;
class WVisual;
class WOverview;
class WNumberPos;
class WNumberBpm;
class QDomNode;
class QDomElement;
class Sampler;
class SamplerManager;
class WaveformRenderer;

class WSampler : public WWidget {
    Q_OBJECT
public:
    WSampler(QWidget* parent, SamplerManager* pSamplerManager);
    virtual ~WSampler();
    void setup(QDomNode node, QList<QObject *> m_qWidgetList);
    WOverview *m_pOverviewCh3;
    WOverview *m_pOverviewCh4;
    WOverview *m_pOverviewCh5;
    WOverview *m_pOverviewCh6;
    WaveformRenderer* m_pWaveformRendererCh3;
    WaveformRenderer* m_pWaveformRendererCh4;
    WaveformRenderer* m_pWaveformRendererCh5;
    WaveformRenderer* m_pWaveformRendererCh6;
    
public slots:
    void slotSetupTrackConnectionsCh3(TrackPointer pTrack);
    void slotSetupTrackConnectionsCh4(TrackPointer pTrack);
    void slotSetupTrackConnectionsCh5(TrackPointer pTrack);
    void slotSetupTrackConnectionsCh6(TrackPointer pTrack);
    
    /** Saves entire sampler bank as XML file **/
    void slotSaveSamplerBank();
    void slotLoadSamplerBank();
private:
    SamplerManager* m_pSamplerManager;
    QFrame* m_pSamplerWindow;
    QAction *saveSamplerBank; 
    QAction *loadSamplerBank;
};

#endif /* WSampler_H */
