import QtQml

QtObject {
    id: controller

    property string controllerId: ""
    property bool debugMode: false

    function init(controllerId, debugMode) {
        controller.controllerId = controllerId;
        controller.debugMode = debugMode;
        console.log(controllerId, debugMode);
    }
    function shutdown() {
        console.log(`Shutting down ${controller.controllerId} with debug mode ${controller.controllerId}`);
    }
}
