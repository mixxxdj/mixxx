import QtQml

import "Mixxx"

MixxxController {
    id: controller

    onInit: console.error(`Starting controller ${controller.controllerId} with debug mode ${controller.debugMode}`)
    onShutdown: console.error(`Shutting down ${controller.controllerId} with debug mode ${controller.debugMode}`)

    MixxxScreen {
        screenId: "screen 7"
        splashOff: 5000
        onInit: console.error(`MixxxScreen.screenId=${screenId}, MixxxScreen.splashOff=${splashOff}`)
        transformFrame: (frame, timestamp, area) => {
            console.error(frame)
            return new ArrayBuffer(0)
        }
    }
}
