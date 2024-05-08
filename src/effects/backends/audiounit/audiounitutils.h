#pragma once

#define AUDIO_UNIT_GET(                                          \
        INSTANCE, PROPERTY, SCOPE, ELEMENT, OUT_VALUE, OUT_SIZE) \
    AudioUnitGetProperty(INSTANCE,                               \
            kAudioUnitProperty_##PROPERTY,                       \
            kAudioUnitScope_##SCOPE,                             \
            ELEMENT,                                             \
            OUT_VALUE,                                           \
            OUT_SIZE)

#define AUDIO_UNIT_INFO(INSTANCE, PROPERTY, SCOPE, ELEMENT, OUT_VALUE) \
    AudioUnitGetPropertyInfo(INSTANCE,                                 \
            kAudioUnitProperty_##PROPERTY,                             \
            kAudioUnitScope_##SCOPE,                                   \
            ELEMENT,                                                   \
            OUT_VALUE,                                                 \
            nullptr)
