#ifndef DEFS_BROADCAST_H
#define DEFS_BROADCAST_H

// NOTE(rryan): Do not change this from [Shoutcast] unless you also put upgrade
// logic in src/preferences/upgrade.h.
#define BROADCAST_PREF_KEY "[Shoutcast]"
#define BROADCAST_DEFAULT_PORT 8000

#define BROADCAST_SERVER_SHOUTCAST "Shoutcast"
#define BROADCAST_SERVER_ICECAST1 "Icecast1"
#define BROADCAST_SERVER_ICECAST2 "Icecast2"

#define BROADCAST_BITRATE_320KBPS 320
#define BROADCAST_BITRATE_256KBPS 256
#define BROADCAST_BITRATE_224KBPS 224
#define BROADCAST_BITRATE_192KBPS 192
#define BROADCAST_BITRATE_160KBPS 160
#define BROADCAST_BITRATE_128KBPS 128
#define BROADCAST_BITRATE_112KBPS 112
#define BROADCAST_BITRATE_96KBPS 96
#define BROADCAST_BITRATE_80KBPS 80
#define BROADCAST_BITRATE_64KBPS 64
#define BROADCAST_BITRATE_48KBPS 48
#define BROADCAST_BITRATE_32KBPS 32

#define BROADCAST_FORMAT_MP3 "MP3"
#define BROADCAST_FORMAT_OV "OggVorbis"
#define BROADCAST_FORMAT_OPUS "Opus"

// EngineNetworkStream can't use locking mechanisms to protect its
// internal worker list against concurrency issues, as it is used by
// methods called from the audio engine thread.
// Instead, the internal list has a fixed number of QSharedPointers
// (which are thread-safe) initialized to null pointers. R/W operations to
// the workers are then performed on thread-safe QSharedPointers and not
// onto the thread-unsafe QVector
#define BROADCAST_MAX_CONNECTIONS 16

#endif /* DEFS_BROADCAST_H */
