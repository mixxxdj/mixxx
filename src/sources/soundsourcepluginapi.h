#ifndef MIXXX_SOUNDSOURCEPLUGINAPI_H
#define MIXXX_SOUNDSOURCEPLUGINAPI_H

#define MIXXX_SOUNDSOURCEPLUGINAPI_VERSION 8
/** @note SoundSource API Version history:
 1 - Mixxx 1.8.0 Beta 2
 2 - Mixxx 1.9.0 Pre (added key code)
 3 - Mixxx 1.10.0 Pre (added freeing function for extensions)
 4 - Mixxx 1.11.0 Pre (added composer field to SoundSource)
 5 - Mixxx 1.12.0 Pre (added album artist and grouping fields to SoundSource)
 6 - Mixxx 1.12.0 Pre (added cover art suppport)
 7 - Mixxx 1.13.0 New SoundSource/AudioSource API
 8 - Mixxx 1.13.0 New SoundSource Plugin API
 */

//As per QLibrary docs: http://doc.trolltech.com/4.6/qlibrary.html#resolve
#ifdef Q_OS_WIN
#define MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT __declspec(dllexport)
#else
#define MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
#endif

#include "sources/soundsourceprovider.h"

// Function types and names of the public SoundSource plugin API

namespace Mixxx {

// extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT const char* Mixxx_getVersion()

// extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT int Mixxx_SoundSourcePluginAPI_getVersion()
typedef int (*SoundSourcePluginAPI_getVersionFunc)();
const char * const SoundSourcePluginAPI_getVersionFuncName = "Mixxx_SoundSourcePluginAPI_getVersion";

// extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT QStringList Mixxx_SoundSourcePluginAPI_getSoundSourceProvider()
typedef SoundSourceProviderPointer (*SoundSourcePluginAPI_getSoundSourceProviderFunc)();
const char* const SoundSourcePluginAPI_getSoundSourceProviderFuncName = "Mixxx_SoundSourcePluginAPI_getSoundSourceProvider";

} // Mixxx

#endif // MIXXX_SOUNDSOURCEPLUGINAPI_H
