/** @file defs_audiofiles.h
 *
 *  This file contains constants that are related to the types of audio files we support.
 *
 */

#ifndef __DEFS_AUDIOFILES_H__
#define __DEFS_AUDIOFILES_H__

#ifdef __M4A__
/** The types of audio files we support */
#define MIXXX_SUPPORTED_AUDIO_FILETYPES "*.wav *.mp3 *.m4a *.ogg *.aiff *.aif *.flac"
/** A regex for the types of audio files we support */
#define MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX "\\.(mp3|m4a|ogg|aiff|aif|wav|flac)"
#else //If the __M4A__ symbol isn't defined, then we can't use .m4a as a supported audio filetype :)
/** The types of audio files we support */
#define MIXXX_SUPPORTED_AUDIO_FILETYPES "*.wav *.mp3 *.ogg *.aiff *.aif *.flac"
/** A regex for the types of audio files we support */
#define MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX "\\.(mp3|ogg|aiff|aif|wav|flac)"
#endif //__M4A__


#endif
