import Mixxx 0.1 as Mixxx
import QtQuick 2.12

ShaderEffect {
    id: root

    property string group // required
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property size framebufferSize: Qt.size(width, height)
    property int waveformLength: root.deckPlayer.waveformLength
    property int textureSize: root.deckPlayer.waveformTextureSize
    property int textureStride: root.deckPlayer.waveformTextureStride
    property real firstVisualIndex: 1
    property real lastVisualIndex: root.deckPlayer.waveformLength / 2
    property color axesColor: "#FFFFFF"
    property color highColor: "#FF0000"
    property color midColor: "#00FF00"
    property color lowColor: "#0000FF"
    property real highGain: filterWaveformEnableControl.value ? (filterHighKillControl.value ? 0 : filterHighControl.value) : 1
    property real midGain: filterWaveformEnableControl.value ? (filterMidKillControl.value ? 0 : filterMidControl.value) : 1
    property real lowGain: filterWaveformEnableControl.value ? (filterLowKillControl.value ? 0 : filterLowControl.value) : 1
    property real allGain: pregainControl.value
    property Image waveformTexture

    fragmentShader: "
#version 120

uniform vec2 framebufferSize;
uniform vec4 axesColor;
uniform vec4 lowColor;
uniform vec4 midColor;
uniform vec4 highColor;

uniform int waveformLength;
uniform int textureSize;
uniform int textureStride;

uniform float allGain;
uniform float lowGain;
uniform float midGain;
uniform float highGain;
uniform float firstVisualIndex;
uniform float lastVisualIndex;

uniform sampler2D waveformTexture;
varying highp vec2 qt_TexCoord0;

vec4 getWaveformData(float index) {
    vec2 uv_data;
    uv_data.y = floor(index / float(textureStride));
    uv_data.x = floor(index - uv_data.y * float(textureStride));
    // Divide again to convert to normalized UV coordinates.
    return texture2D(waveformTexture, uv_data / float(textureStride));
}

void main(void) {
    vec2 uv = qt_TexCoord0.st;
    vec4 pixel = gl_FragCoord;

    float new_currentIndex = floor(firstVisualIndex + uv.x *
                                   (lastVisualIndex - firstVisualIndex)) * 2;

    // Texture coordinates put (0,0) at the bottom left, so show the right
    // channel if we are in the bottom half.
    if (uv.y < 0.5) {
        new_currentIndex += 1;
    }

    vec4 outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    bool showing = false;
    bool showingUnscaled = false;
    vec4 showingColor = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 showingUnscaledColor = vec4(0.0, 0.0, 0.0, 0.0);

    // We don't exit early if the waveform data is not valid because we may want
    // to show other things (e.g. the axes lines) even when we are on a pixel
    // that does not have valid waveform data.
    if (new_currentIndex >= 0 && new_currentIndex <= waveformLength - 1) {
      vec4 new_currentDataUnscaled = getWaveformData(new_currentIndex) * allGain;

      vec4 new_currentData = new_currentDataUnscaled;
      new_currentData.x *= lowGain;
      new_currentData.y *= midGain;
      new_currentData.z *= highGain;

      //(vrince) debug see pre-computed signal
      //gl_FragColor = new_currentData;
      //return;

      // Represents the [-1, 1] distance of this pixel. Subtracting this from
      // the signal data in new_currentData, we can tell if a signal band should
      // show in this pixel if the component is > 0.
      float ourDistance = abs((uv.y - 0.5) * 2.0);

      // Since the magnitude of the (low, mid, high) vector is used as the
      // waveform height, re-scale the maximum height to 1.
      const float scaleFactor = 1.0 / sqrt(3.0);

      float signalDistance = sqrt(new_currentData.x * new_currentData.x +
                                  new_currentData.y * new_currentData.y +
                                  new_currentData.z * new_currentData.z) *
                             scaleFactor;
      showing = (signalDistance - ourDistance) >= 0.0;

      // Linearly combine the low, mid, and high colors according to the low,
      // mid, and high components.
      showingColor = lowColor * new_currentData.x +
                     midColor * new_currentData.y +
                     highColor * new_currentData.z;

      // Re-scale the color by the maximum component.
      float showingMax = max(showingColor.x, max(showingColor.y, showingColor.z));
      showingColor = showingColor / showingMax;
      showingColor.w = 1.0;

      // Now do it all over again for the unscaled version of the waveform,
      // which we will draw at very low opacity.
      float signalDistanceUnscaled = sqrt(new_currentDataUnscaled.x * new_currentDataUnscaled.x +
                                          new_currentDataUnscaled.y * new_currentDataUnscaled.y +
                                          new_currentDataUnscaled.z * new_currentDataUnscaled.z) * scaleFactor;
      showingUnscaled = (signalDistanceUnscaled - ourDistance) >= 0.0;

      // Linearly combine the low, mid, and high colors according to the
      // original low, mid, and high components.
      showingUnscaledColor = lowColor * new_currentDataUnscaled.x +
                             midColor * new_currentDataUnscaled.y +
                             highColor * new_currentDataUnscaled.z;

      // Re-scale the color by the maximum component.
      float showingUnscaledMax = max(showingUnscaledColor.x, max(showingUnscaledColor.y, showingUnscaledColor.z));
      showingUnscaledColor = showingUnscaledColor / showingUnscaledMax;
      showingUnscaledColor.w = 1.0;
    }

    // Draw the axes color as the lowest item on the screen.
    // TODO(owilliams): The 4 in this line makes sure the axis gets
    // rendered even when the waveform is fairly short.  Really this
    // value should be based on the size of the widget.
    if (abs(framebufferSize.y / 2 - pixel.y) <= 4) {
      outputColor.xyz = mix(outputColor.xyz, axesColor.xyz, axesColor.w);
      outputColor.w = 1.0;
    }

    if (showingUnscaled) {
      float alpha = 0.4;
      outputColor.xyz = mix(outputColor.xyz, showingUnscaledColor.xyz, alpha);
      outputColor.w = 1.0;
    }

    if (showing) {
      float alpha = 0.8;
      outputColor.xyz = mix(outputColor.xyz, showingColor.xyz, alpha);
      outputColor.w = 1.0;
    }
    gl_FragColor = outputColor;
}

        "

    Mixxx.ControlProxy {
        id: pregainControl

        group: root.group
        key: "pregain"
    }

    Mixxx.ControlProxy {
        id: filterWaveformEnableControl

        group: root.group
        key: "filterWaveformEnable"
    }

    Mixxx.ControlProxy {
        id: filterHighControl

        group: root.group
        key: "filterHigh"
    }

    Mixxx.ControlProxy {
        id: filterHighKillControl

        group: root.group
        key: "filterHighKill"
    }

    Mixxx.ControlProxy {
        id: filterMidControl

        group: root.group
        key: "filterMid"
    }

    Mixxx.ControlProxy {
        id: filterMidKillControl

        group: root.group
        key: "filterMidKill"
    }

    Mixxx.ControlProxy {
        id: filterLowControl

        group: root.group
        key: "filterLow"
    }

    Mixxx.ControlProxy {
        id: filterLowKillControl

        group: root.group
        key: "filterLowKill"
    }

    waveformTexture: Image {
        visible: false
        layer.enabled: false
        source: root.deckPlayer.waveformTexture
    }

}
