#pragma once
#include <Qt>

// Maximum buffer length to each EngineObject::process call.
// Note: MAX_BUFFER_LEN shouldn't be use for audio processing,
// because the buffers should be sized according to the processing
// block in frames and then multiplied by the channel count.
// Then audio buffers used in I/O and non realtime operation
// should be sized according to their task with a balance in mind
// between CPU efficiency and memory usage.
//
// Max frames at 44.1kHz:     4096
// Max frames at  192kHz: 4 * 4096
//
// TODO: Replace this with mixxx::AudioParameters::bufferSize()
// Mixxx engine uses audio buffers up to ~80 ms. This is 8192 Frames @ 96 kHz
// see class SoundManagerConfig::AudioBufferSizeIndex
// 96 kHz * 80 ms = 7680 -> 8192 (2^13)
constexpr unsigned int kMaxEngineFrames = 8192;
constexpr unsigned int kMaxEngineChannels = 2;
constexpr unsigned int kMaxEngineSamples = kMaxEngineChannels * kMaxEngineFrames;
constexpr unsigned int MAX_BUFFER_LEN = 160000;

constexpr int kMaxNumberOfDecks = 4;
constexpr int kMaxNumberOfHotcues = 36;
// Hotcue index is 0-based (0..35), +1 is the main cue
// Used for a common tracking of the main cue in CueControl and WCueButton
constexpr int kMainCueIndex = kMaxNumberOfHotcues;

// Keyboard shortcut components for showing the Track Properties dialog and
// for displaying the shortcut in the track context menu
constexpr Qt::Modifier kPropertiesShortcutModifier = Qt::CTRL;
constexpr Qt::Key kPropertiesShortcutKey = Qt::Key_Return;

// Keyboard shortcut for hiding track and removing from Crate/Playlist/AutoDJ queue.
// This is also used to display the shortcut in the track context menu.
// Also used for the 'Remove' actions in the library sidebar.
#ifdef Q_OS_MAC
// Note: On macOS, CTRL corresponds to the Command key.
constexpr Qt::Modifier kHideRemoveShortcutModifier = Qt::CTRL;
constexpr Qt::Key kHideRemoveShortcutKey = Qt::Key_Backspace;
#else
constexpr Qt::Modifier kHideRemoveShortcutModifier = static_cast<Qt::Modifier>(0);
constexpr Qt::Key kHideRemoveShortcutKey = Qt::Key_Delete;
#endif

constexpr Qt::Key kRenameSidebarItemShortcutKey = Qt::Key_F2;
