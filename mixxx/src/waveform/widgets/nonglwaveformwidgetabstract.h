#pragma once

#include <QWidget>

#include "waveform/widgets/waveformwidgetabstract.h"

QT_FORWARD_DECLARE_CLASS(QString)

/// NonGLWaveformWidgetAbstract is a WaveformWidgetAbstract & QWidget
class NonGLWaveformWidgetAbstract : public WaveformWidgetAbstract, public QWidget {
  public:
    NonGLWaveformWidgetAbstract(const QString& group, QWidget* parent)
            : WaveformWidgetAbstract(group),
              QWidget(parent) {
    }
};
