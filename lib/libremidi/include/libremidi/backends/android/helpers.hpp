#pragma once
#include <libremidi/error.hpp>

#include <amidi/AMidi.h>
#include <android/log.h>

#include <jni.h>
#include <pthread.h>

#include <string>
#include <vector>

#define LOG_TAG "libremidi"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

NAMESPACE_LIBREMIDI
{
namespace android
{
struct context
{
  static inline std::string client_name;
  static inline std::vector<jobject> midi_devices;

  static JNIEnv* get_thread_env();
  static jobject get_context(JNIEnv* env);
  static jobject get_midi_manager(JNIEnv* env, jobject context);
  static void refresh_midi_devices(JNIEnv* env, jobject context, bool is_output);
  static void open_device(jobject device_info, void* target, bool is_output);
  static std::string port_name(JNIEnv* env, unsigned int port_number);
  static void cleanup_devices(JNIEnv* env);
};

extern "C" JNIEXPORT void JNICALL Java_dev_celtera_libremidi_MidiDeviceCallback_onDeviceOpened(
    JNIEnv* env, jobject /*thiz*/, jobject midi_device, jlong target_ptr, jboolean is_output);
}
}
