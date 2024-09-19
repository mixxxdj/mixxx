import QtQuick 2.15
import QtQuick.Window 2.3

import QtQuick.Controls 2.15
import QtQuick.Shapes 1.11
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import Qt5Compat.GraphicalEffects

import "." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

import S4MK3 as S4MK3

Mixxx.ControllerScreen {
    id: root

    required property string screenId
    property color fontColor: Qt.rgba(242/255,242/255,242/255, 1)
    property color smallBoxBorder: Qt.rgba(44/255,44/255,44/255, 1)

    property string group: screenId == "rightdeck" ? "[Channel2]" : "[Channel1]"
    property string theme: engine.getSetting("theme") || "stock"

    readonly property bool isStockTheme: theme == "stock"

    property var lastFrame: null

    init: function(_controllerName, isDebug) {
        console.log(`Screen ${root.screenId} has started with theme ${root.theme}`)
        root.state = "Live"
    }

    shutdown: function() {
        console.log(`Screen ${root.screenId} is stopping`)
        root.state = "Stop"
    }

    transformFrame: function(input, timestamp) {
        let updated = new Uint8Array(320*240);
        updated.fill(0)

        let updatedPixelCount = 0;
        let updated_zones = [];

        if (!root.lastFrame) {
            root.lastFrame = new ArrayBuffer(input.byteLength);
            updatedPixelCount = input.byteLength / 2;
            updated_zones.push({
                    x: 0,
                    y: 0,
                    width: 320,
                    height: 240,
            })
        } else {
            const view_input = new Uint8Array(input);
            const view_last = new Uint8Array(root.lastFrame);

            for (let i = 0; i < 320 * 240; i++) {
            }

            let current_rect = null;

            for (let y = 0; y < 240; y++) {
                let line_changed = false;
                for (let x = 0; x < 320; x++) {
                    let i = y * 320 + x;
                    if (view_input[2 * i] != view_last[2 * i] || view_input[2 * i + 1] != view_last[2 * i + 1]) {
                        line_changed = true;
                        updatedPixelCount++;
                        break;
                    }
                }
                if (current_rect !== null && line_changed) {
                    current_rect.height++;
                } else if (current_rect !== null) {
                    updated_zones.push(current_rect);
                    current_rect = null;
                } else if (current_rect === null && line_changed) {
                    current_rect = {
                        x: 0,
                        y,
                        width: 320,
                        height: 1,
                    };
                }
            }
            if (current_rect !== null) {
                updated_zones.push(current_rect);
            }
        }
        new Uint8Array(root.lastFrame).set(new Uint8Array(input));

        if (!updatedPixelCount) {
            return new ArrayBuffer(0);
        } else if (root.renderDebug) {
            console.log(`Pixel updated: ${updatedPixelCount}, ${updated_zones.length} areas`);
        }

        // No redraw needed, stop right there

        let totalPixelToDraw = 0;
        for (const area of updated_zones) {
            area.x -= Math.min(2, area.x);
            area.y -= Math.min(2, area.y);
            area.width += Math.min(4, 320 - area.x - area.width);
            area.height += Math.min(4, 240 - area.y - area.height);
            totalPixelToDraw += area.width*area.height;
        }

        if (totalPixelToDraw != 320*240 && (totalPixelToDraw > 320 * 180 || updated_zones.length > 20)) {
            if (root.renderDebug) {
                console.log(`Full redraw instead of ${totalPixelToDraw} pixels/${updated_zones.length} areas`)
            }
            totalPixelToDraw = 320*240
            updated_zones = [{
                    x: 0,
                    y: 0,
                    width: 320,
                    height: 240,
            }]
        } else if (root.renderDebug) {
            console.log(`Redrawing ${totalPixelToDraw} pixels`)
        }

        const screenIdx = screenId === "leftdeck" ? 0 : 1;

        const outputData = new ArrayBuffer(totalPixelToDraw*2 + 20*updated_zones.length); // Number of pixel + 20 (header/footer size) x the number of region
        let offset = 0;

        for (const area of updated_zones) {
            const header = new Uint8Array(outputData, offset, 16);
            const payload = new Uint8Array(outputData, offset + 16, area.width*area.height*2);
            const footer = new Uint8Array(outputData, offset + area.width*area.height*2 + 16, 4);

            header.fill(0)
            footer.fill(0)
            header[0] = 0x84;
            header[2] = screenIdx;
            header[3] = 0x21;

            header[8] = area.x >> 8;
            header[9] = area.x & 0xff;
            header[10] = area.y >> 8;
            header[11] = area.y & 0xff;

            header[12] = area.width >> 8;
            header[13] = area.width & 0xff;
            header[14] = area.height >> 8;
            header[15] = area.height & 0xff;

            if (area.x === 0 && area.width === 320) {
                payload.set(new Uint8Array(input, area.y * 320 * 2, area.width*area.height*2));
            } else {
                for (let y = 0; y < area.height; y++) {
                    payload.set(
                        new Uint8Array(input, ((area.y + y) * 320 + area.x) * 2, area.width * 2),
                        y * area.width * 2);
                }
            }
            footer[0] = 0x40;
            footer[2] = screenIdx;
            offset += area.width*area.height*2 + 20
        }
        if (root.renderDebug) {
            console.log(`Generated ${offset} bytes to be sent`)
        }
        // return new ArrayBuffer(0);
        return outputData;
    }

    Component {
        id: splashOff
        S4MK3.SplashOff {
            anchors.fill: parent
        }
    }
    Component {
        id: stockLive
        S4MK3.StockScreen {
            group: root.group
            screenId: root.screenId
            anchors.fill: parent
        }
    }
    Component {
        id: advancedLive
        S4MK3.AdvancedScreen {
            isLeftScreen: root.screenId == "leftdeck"
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: splashOff
    }

    states: [
        State {
            name: "Live"
            PropertyChanges {
                target: loader
                sourceComponent: isStockTheme ? stockLive : advancedLive
            }
        },
        State {
            name: "Stop"
            PropertyChanges {
                target: loader
                sourceComponent: splashOff
            }
        }
    ]
}
