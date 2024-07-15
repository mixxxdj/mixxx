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
}
