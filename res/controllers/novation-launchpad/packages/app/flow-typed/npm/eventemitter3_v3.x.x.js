declare module 'eventemitter3' {
  declare type ListenerFn = (...args: any[]) => void
  declare class EventEmitter {
    static constructor(): EventEmitter,
    static prefixed: string | boolean,
    eventNames(): (string | Symbol)[],
    listeners(event: string | Symbol): ListenerFn[],
    listenerCount(event: string | Symbol): number,
    on(event: string | Symbol, listener: ListenerFn, context?: any): this,
    addListener(event: string | Symbol, listener: ListenerFn, context?: any): this,
    once(event: string | Symbol, listener: ListenerFn, context?: any): this,
    removeAllListeners(event?: string | Symbol): this,
    removeListener(event: string | Symbol, listener?: ListenerFn, context?: any, once?: boolean): this,
    off(event: string | Symbol, listener?: ListenerFn, context?: any, once?: boolean): this,
    emit(event: string, ...params?: any[]): this
  }
  declare module.exports: Class<EventEmitter>
}

// Filename aliases
declare module 'eventemitter3/index' {
  declare module.exports: $Exports<'eventemitter3'>
}
declare module 'eventemitter3/index.js' {
  declare module.exports: $Exports<'eventemitter3'>
}
