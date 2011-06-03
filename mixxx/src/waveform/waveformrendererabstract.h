#ifndef WAVEFORMRENDERERABSTRACT_H
#define WAVEFORMRENDERERABSTRACT_H

class WaveformWidgetRenderer;
class QPainter;
class QPaintEvent;
class QDomNode;

class WaveformRendererAbstract
{
public:
    WaveformRendererAbstract( WaveformWidgetRenderer* waveformWidgetRenderer);

    bool isDirty() const { return m_dirty;}
    void setDirty( bool dirty = true){ m_dirty = dirty;}

    //those can even be protected since the main renderer is a friend class

    virtual void init() = 0;
    virtual void setup( const QDomNode& node) = 0;
    virtual void draw( QPainter* painter, QPaintEvent* event) = 0;

    virtual void onResize() {}

protected:
    WaveformWidgetRenderer* m_waveformWidget;
    bool m_dirty;

    friend class WaveformWidgetRenderer;
};

#endif // WAVEFORMRENDERERABSTRACT_H
