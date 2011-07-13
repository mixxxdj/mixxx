
#ifndef WAVEFORMVIEWERFACTORY_H
#define WAVEFORMVIEWERFACTORY_H

#include <QList>
#include <QDomNode>
#include <QWidget>
#include <QTimer>
#include <QGLContext>

#include "configobject.h"
#include "widget/wwidget.h"

class WVisualSimple;
class WWaveformViewer;
class WGLWaveformViewer;
class WaveformRenderer;

enum WaveformViewerType {
    WAVEFORM_SIMPLE=0,
    WAVEFORM_WIDGET,
    WAVEFORM_GL,
    WAVEFORM_INVALID
};


class WaveformViewerFactory {
private:
    /* shouldn't be accessible */
    WaveformViewerFactory() {};
    ~WaveformViewerFactory() {};
    static QList<QWidget*> m_viewers;
    static QList<WVisualSimple*> m_simpleViewers;
    static QList<WWaveformViewer*> m_visualViewers;
    static QList<WGLWaveformViewer*> m_visualGLViewers;
    static QTimer s_waveformUpdateTimer;

public:
    static WaveformViewerType createWaveformViewer(const char* group, QWidget *pParent, ConfigObject<ConfigValue> *pConfig, QWidget **target, WaveformRenderer *pWaveformRenderer);
    static void destroyWaveformViewer(QWidget *pWaveformViewer);
    static WaveformViewerType getWaveformViewerType(QWidget *pWaveformViewer);
    static int numViewers(WaveformViewerType type);
    static bool isDirectRenderingEnabled();
};

#endif
