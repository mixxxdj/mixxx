import Mixxx 1.0 as Mixxx

Mixxx.WaveformDisplay {
    id: root

    player: Mixxx.PlayerManager.getPlayer(root.group)
}
