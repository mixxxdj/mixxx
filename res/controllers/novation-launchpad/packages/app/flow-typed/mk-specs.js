declare module '@mixxx-launchpad/mk-specs/buttons' {
  declare type LaunchpadMidiButtonDef = {
    status: number,
    midino: number,
    name: string
  }
  declare var exports: { [id: string]: LaunchpadMidiButtonDef }
}

declare module '@mixxx-launchpad/mk-specs/colors' {
  declare type LaunchpadMidiButtonColor = number
  declare var exports: { [key: string]: LaunchpadMidiButtonColor }
}
