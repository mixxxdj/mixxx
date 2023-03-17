# Mixxx QOpenGL classes and files

This document describes the QOpenGL based WGLWidget that replace the legacy QGLWidget and the GLSL-based waveform widget and renderers and the GLSL-based VU meter and Spinny classes.

## WGLWidget

`WGLWidget` is the replacement for `QGLWidget`, which, depending on the CMake option `QOPENGL` and preprocessor define `MIXX_USE_QOPEN` is either one of:

- a thin wrapper for, and derived directly from, `QGLWidget`. This is offered as a fallback / alternative in case the new QOpenGL classes don't work.

- derived from `QWidget` and encapsulating a `QOpenGLWindow` through `createWindowContainer(...)`

The file to be included from derives classes, which in turn include either one from is src/widget/wglwidget.h

which in turn includes the actual `WGLWidget` from either one of:

- src/widget/wglwidgetqglwidget.h
- src/widget/wglwidgetqopengl.h

The corresponding implementations are in:

- src/widget/wglwidgetqglwidget.cpp
- src/widget/wglwidgetqopengl.cpp

The later uses a helper `OpenGLWindow`, derived from `QOpenGLWindow`, in

- src/widget/openglwindow.h
- src/widget/openglwindow.cpp

## Utility classes

A single `WInitialGLWidget` is created on startup and used to query the QOpenGL information.

- src/widget/winitialglwidget.{h|cpp}

Tooltips don't work for the qopengl-based `WGLWidget` (because the mouse moves over a different window). The singleton `ToolTipQOpenGL` mimics the standard tooltip behaviour, with a timer triggered after mouse not moving.

- src/widget/tooltipqopengl.{h|cpp}

## VU-Meter and Spinny extensions

Additions methods for drawing with QOpenGL shaders are added to `WVuMeterGL` and `WSpinnyGL`:

- src/widget/qopengl/wvumetergl.cpp
- src/widget/qopengl/wspinny.cpp

## Waveform rendering architecture

The following uses the existing waveform architecture as handled by `WaveformWidgetFactory` and triggered by calling rendering and buffer swaps from the `VSyncThread` and using the audio playback synchronized `VisualPlayPosition`.

## Waveform widgets

The following are all using namespace `qopengl`, to distinguish them from the legacy implementations with the same name.

The base-class `qopengl::WaveformWidget`, derived from `WaveformWidgetAbstract`, as it has handled by the `WaveformWidgetFactory`, and from `WGLWidget`, as it will render using `QOpenGLFunctions` in a `QOpenGLWindow`:

- src/waveform/widgets/qopengl/waveformwidget.{h|cpp}

From `qopengl::WaveformWidget` we can derive the actual waveform rendering widgets, currently there is only one, which is listed as "RGB (QOpenGL) (GLSL)", `qopengl::RGBWaveformWidget`:

- src/waveform/widgets/qopengl/rgbwaveformwidget.{h|cpp}

The `qopengl::RGBWaveformWidget` has a stack of `qopengl::IWaveformRenderer` instances, which use GLSL shaders to do the drawing:

## qopengl::IWaveformRenderer derived (indirectly) classes

`opengl::WaveformRenderBackground` - draws a solid color background (TODO (?) draw image)

- src/waveform/renderers/qopengl/waveformrenderbackground.{h|cpp}

`opengl::WaveformRendererEndOfTrack` - draws a blinking gradient when playback reaches end of track

- src/waveform/renderers/qopengl/waveformrendererendoftrack.{h|cpp}

`opengl::WaveformRendererPreroll` - fills the preroll and postroll with a series of triangles

- src/waveform/renderers/qopengl/waveformrendererpreroll.{h|cpp}

`opengl::WaveformRenderMarkRange` - draws ranges with colored rectangles

- src/waveform/renderers/qopengl/waveformrendermarkrange.{h|cpp}

`opengl::WaveformRendererRGB` - draws the actual waveform with lines representing amplitude and color representing low/mid/high filtered content.

- src/waveform/renderers/qopengl/waveformrendererrgb.{h|cpp}

`opengl::WaveformRenderBeat` - draws beat markers with simple lines

- src/waveform/renderers/qopengl/waveformrenderbeat.{h|cpp}

`opengl::WaveformRenderMark` - renders markers (cues, loops, fades) in offscreen images and draws them with textures

- src/waveform/renderers/qopengl/waveformrendermark.{h|cpp}

## Waveform renderer intermediate classes

The waveform renderers listed above derive from either one of the following

`opengl::WaveformShaderRenderer`

- src/waveform/renderers/qopengl/waveformshaderrenderer.{h|cpp}

`opengl::WaveformRenderer`

- src/waveform/renderers/qopengl/waveformrenderer.{h|cpp}

`opengl::WaveformRendererSignalBase`

- src/waveform/renderers/qopengl/waveformrenderersignalbase.{h|cpp}

`opengl::WaveformShaderRenderer` in turn derives from qopengl::ShaderLoader`

- src/waveform/renderers/qopengl/shaderloader.{h|cpp}

which all eventually derive from `qopengl::IWaveformRenderer`

- src/waveform/renderers/qopengl/iwaveformrenderer.h
