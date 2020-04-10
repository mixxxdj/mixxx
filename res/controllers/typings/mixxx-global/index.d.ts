declare namespace engine {
    function getParameter(group: string, key: string): number
    function setParameter(group: string, key: string, newValue: number): void
    function getValue(group: string, key: string): number
    function setValue(group: string, key: string, newValue: number): void


    interface connectionBinding {
        disconnect(): boolean
        trigger(): void
        isConnected: boolean
    }

    type callbackHandler = (
        value: number,
        group: string,
        control: string
    ) => void

    function makeConnection(
        group: string,
        key: string,
        handler: callbackHandler
    ): connectionBinding


    function softTakeover(group: string, key: string, enable: bool): void
    function softTakeoverIgnoreNextValue(group: string, key: string): void

    function scratchEnable(
        deck: number,
        intervalsPerRev: number,
        rpm: number,
        alpha: number,
        beta: number,
        ramp: bool
    ): void
    function scratchTick(deck: number, interval: number): void
    function scratchDisable(deck: number, ramp: boolean): void
    function isScratching(deck: number): boolean

    type timerID = number
    function beginTimer(milliseconds: number, callback: function, oneShot: boolean): timerID
    function stopTimer(id: timerID): void

    function brake(deck: number, activate: boolean, factor?: number): void
    function spinback(deck: number, activate: boolean, factor?: number, rate?: number): void
    function softStart(deck: number, activate: boolean, factor?: number): void

    // DEPRECATED
    function connectControl(group: string, key: string, handler: callbackHandler | string, disconnect?: boolean): void
    function trigger(group: string, key: string): void
}



declare namespace midi {
    function sendSysexMsg(byteArray: Array<number>, length?: number): void
    function sendShortMsg(byte1: number, byte2: number, byte3: number): void
}

// used by hid API
declare namespace controller {
    function send(byteArray: Array<number>, length: null, reportID: number): void
}

declare type RGBColorCode = number

declare interface RGBObject {
    red: number
    green: number
    blue: number
}

declare namespace color {
    function colorCodeToObject(color: RGBColorCode): RGBObject
    function colorCodeFromObject(color: RGBObject): RGBColorCode
}

declare class ColorMapper {
    constructor(availableColors: Map<string, number>)
    getValueForNearestColor(ColorCode: RGBColorCode): number
    getNearestColor(ColorCode: RGBColorCode): RGBObject
}

declare interface IBaseController {
    init(id: string, debugging: boolean): void
    shutdown(): void
}

declare interface IHidController extends IBaseController {
    incomingData(data: Array<number>, length: number): void
}

declare function print(text: string): void
