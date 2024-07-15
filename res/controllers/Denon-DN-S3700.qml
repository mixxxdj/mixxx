import QtQml

import "Mixxx"

MixxxController {
    id: controller

    function init() {
        console.error(controller.controllerId, controller.debugMode);
    }
    function shutdown() {
        console.error(`Shutting down ${controller.controllerId} with debug mode ${controller.debugMode}`);
    }

    MixxxScreen {
        screenId: "screen 7"
        splashOff: 5000
        Component.onCompleted: console.error(`MixxxScreen.identifier=${screenId} ${splashOff}`)
    }
}
