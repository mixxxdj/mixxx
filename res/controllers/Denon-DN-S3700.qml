import QtQuick

Item {
    id: controller

    property string controllerId: ""
    property bool debugMode: false

    function init(controllerId, debugMode) {
        controller.controllerId = controllerId;
        controller.debugMode = debugMode;
        console.error(controllerId, debugMode);
        console.error(controller.controllerId, controller.debugMode);
    }
    function shutdown() {
        console.error(`Shutting down ${controller.controllerId} with debug mode ${controller.controllerId}`);
    }
}
