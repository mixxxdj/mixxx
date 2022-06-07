

declare class ControllerScriptInterfaceLegacy {
        public getValue(group : string, name : string) : number;
        public setValue(group : string, name : string, newValue: number) : void;
        public softTakeover(group : string, name : string, set : boolean) : void;
        public getParameter(group : string, name : string) : number;
        public setParameter(group : string, name : string, newValue : number) : void;
        public getParameterForValue(group : string, name : string, value : number) : number;
        public reset(group : string, name : string) : void;
        public getDefaultValue(group : string, name : string) : number;
        public getDefaultParameter(group : string, name : string) : number;

        /**
         * Callback function called from the engine, when the value of a specified control changes
         * @callback controlCallback
         * @param {number} value New value of the connected control
         * @param {string} group Group of the control e.g. "[Channel1]"
         * @param {string} name Name of the control e.g. "play_indicator"
         */

        /** Connects a specified Mixxx Control with a callback function, which is executed if the value of the control changes
         * @details This connection has a FIFO buffer - all value change events are processed in serial order.
         * @param {string} group Group of the control e.g. "[Channel1]"
         * @param {string} name Name of the control e.g. "play_indicator"
         * @param {controlCallback} callback JS function, which will be called everytime, the value of the connected control changes.
         */
        public  makeConnection(group : string, name : string, callback : controlCallback);

        /** Connects a specified Mixxx Control with a callback function, which is executed if the value of the control changes
         * @details This connection is unbuffered - when value change events occur faster, than the mapping can process them,
         *          only the last value set for the control is processed.
         * @param {string} group Group of the control e.g. "[Channel1]"
         * @param {string} name Name of the control e.g. "VuMeter"
         * @param {controlCallback} callback JS function, which will be called everytime, the value of the connected control changes.
         */
        public  makeUnbufferedConnection(group : string, name : string, callback : controlCallback);
        /** @deprecated Use makeConnection instead */
        public  connectControl(group : string, name : string, passedCallback : controlCallback, disconnect:boolean = false);

        /** Called indirectly by the objects returned by connectControl */
        public trigger(group : string, name : string) : void;

        /** @deprecated Use console.log instead */
        public log(message : string) : void;
        /**
         * Callback function called from the engine, when the the interval of a timer is reached
         * @callback timerCallback
         */
        public beginTimer(interval:number, scriptCode : timerCallback, oneShot:boolean = false);
        public stopTimer(timerId : number) : void;
        public scratchEnable(deck : number, intervalsPerRev : number, rpm : number, alpha : number, beta : number, ramp:boolean = true) : void;
        public scratchTick(deck : number, interval : number) : void;
        public scratchDisable(deck : number, ramp:boolean = true) : void;
        public isScratching(deck : number) : boolean;
        public softTakeover(group : string, name : string, set : boolean) : void;
        public softTakeoverIgnoreNextValue(group : string, name : string) : void;
        public brake(deck : number, activate : boolean, factor:number = 1.0, rate:number = 1.0) : void;
        public spinback(deck : number, activate : boolean, factor:number = 1.8, rate:number = -10.0) : void;
        public softStart(deck : number, activate : boolean, factor:number = 1.0) : void;
    }
var engine = new ControllerScriptInterfaceLegacy;
