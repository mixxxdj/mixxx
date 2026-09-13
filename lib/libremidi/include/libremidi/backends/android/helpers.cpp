#include <libremidi/backends/android/helpers.hpp>
#include <libremidi/backends/android/midi_in.hpp>
#include <libremidi/backends/android/midi_out.hpp>

NAMESPACE_LIBREMIDI::android
{
// Main source of knowledge is RtMidi implementation
// which was done by Yellow Labrador

JNIEnv* context::get_thread_env()
{
  JNIEnv* env;
  JavaVM* jvm;
  jsize count;
  if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count < 1)
  {
    LOGE("No JVM found");
    return nullptr;
  }

  if (jvm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED)
  {
    if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
    {
      LOGE("Failed to attach thread");
      return nullptr;
    }
  }

  return env;
}

jobject context::get_context(JNIEnv* env)
{
  auto activity_thread = env->FindClass("android/app/ActivityThread");
  auto current_activity_thread = env->GetStaticMethodID(
      activity_thread, "currentActivityThread", "()Landroid/app/ActivityThread;");
  auto at = env->CallStaticObjectMethod(activity_thread, current_activity_thread);

  if (!at)
  {
    LOGE("Failed to get ActivityThread");
    return nullptr;
  }

  auto get_application
      = env->GetMethodID(activity_thread, "getApplication", "()Landroid/app/Application;");
  auto app = env->CallObjectMethod(at, get_application);

  if (!app)
  {
    LOGE("Failed to get Application");
    return nullptr;
  }

  return app;
}

jobject context::get_midi_manager(JNIEnv* env, jobject ctx)
{
  auto context_class = env->FindClass("android/content/Context");
  auto get_system_service = env->GetMethodID(
      context_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
  auto midi_service_str = env->NewStringUTF("midi");
  auto service = env->CallObjectMethod(ctx, get_system_service, midi_service_str);
  env->DeleteLocalRef(midi_service_str);
  return service;
}

void context::refresh_midi_devices(JNIEnv* env, jobject ctx, bool is_output)
{
  cleanup_devices(env);

  auto midi_service = get_midi_manager(env, ctx);
  if (!midi_service)
    return;

  auto midi_mgr_class = env->FindClass("android/media/midi/MidiManager");
  auto get_devices_method
      = env->GetMethodID(midi_mgr_class, "getDevices", "()[Landroid/media/midi/MidiDeviceInfo;");
  auto device_array = (jobjectArray)env->CallObjectMethod(midi_service, get_devices_method);

  auto device_info_class = env->FindClass("android/media/midi/MidiDeviceInfo");
  auto get_input_count = env->GetMethodID(device_info_class, "getInputPortCount", "()I");
  auto get_output_count = env->GetMethodID(device_info_class, "getOutputPortCount", "()I");

  jsize count = env->GetArrayLength(device_array);
  for (jsize i = 0; i < count; ++i)
  {
    auto device_info = env->GetObjectArrayElement(device_array, i);
    int port_count = is_output ? env->CallIntMethod(device_info, get_input_count)
                               : env->CallIntMethod(device_info, get_output_count);

    if (port_count > 0)
    {
      midi_devices.push_back(env->NewGlobalRef(device_info));
    }
    env->DeleteLocalRef(device_info);
  }

  env->DeleteLocalRef(device_array);
  env->DeleteLocalRef(midi_service);
}

std::string context::port_name(JNIEnv* env, unsigned int port_number)
{
  if (port_number >= midi_devices.size())
  {
    LOGE("Invalid port number");
    return "";
  }

  auto device_info_class = env->FindClass("android/media/midi/MidiDeviceInfo");
  auto get_props_method
      = env->GetMethodID(device_info_class, "getProperties", "()Landroid/os/Bundle;");
  auto bundle = env->CallObjectMethod(midi_devices[port_number], get_props_method);

  auto bundle_class = env->FindClass("android/os/Bundle");
  auto get_string_method
      = env->GetMethodID(bundle_class, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
  auto key = env->NewStringUTF("name");
  auto name_str = (jstring)env->CallObjectMethod(bundle, get_string_method, key);

  std::string result;
  if (name_str)
  {
    const char* name_chars = env->GetStringUTFChars(name_str, nullptr);
    result = name_chars;
    env->ReleaseStringUTFChars(name_str, name_chars);
    env->DeleteLocalRef(name_str);
  }

  env->DeleteLocalRef(key);
  env->DeleteLocalRef(bundle);

  return result;
}

void context::cleanup_devices(JNIEnv* env)
{
  for (jobject device : midi_devices)
  {
    env->DeleteGlobalRef(device);
  }
  midi_devices.clear();
}

void context::open_device(jobject device_info, void* target, bool is_output)
{
  auto env = get_thread_env();
  if (!env)
    return;

  auto ctx = get_context(env);
  if (!ctx)
    return;

  auto midi_mgr = get_midi_manager(env, ctx);
  if (!midi_mgr)
    return;

  // Check if we're on the main thread
  auto looper_class = env->FindClass("android/os/Looper");
  auto get_my_looper_method
      = env->GetStaticMethodID(looper_class, "myLooper", "()Landroid/os/Looper;");
  auto my_looper = env->CallStaticObjectMethod(looper_class, get_my_looper_method);

  jobject handler = nullptr;
  if (!my_looper)
  {
    LOGI("Not on a Looper thread, using main looper");
    auto get_main_looper_method
        = env->GetStaticMethodID(looper_class, "getMainLooper", "()Landroid/os/Looper;");
    auto main_looper = env->CallStaticObjectMethod(looper_class, get_main_looper_method);

    auto handler_class = env->FindClass("android/os/Handler");
    auto handler_ctor = env->GetMethodID(handler_class, "<init>", "(Landroid/os/Looper;)V");
    handler = env->NewObject(handler_class, handler_ctor, main_looper);

    env->DeleteLocalRef(main_looper);
  }

  // Create the callback listener
  auto callback_class = env->FindClass("dev/celtera/libremidi/MidiDeviceCallback");
  if (!callback_class)
  {
    LOGE("MidiDeviceCallback class not found - ensure the Java class is loaded");
    if (handler)
      env->DeleteLocalRef(handler);
    return;
  }

  auto callback_ctor = env->GetMethodID(callback_class, "<init>", "(JZ)V");
  auto callback
      = env->NewObject(callback_class, callback_ctor, (jlong)target, (jboolean)is_output);

  // Open the device
  auto midi_mgr_class = env->FindClass("android/media/midi/MidiManager");
  auto open_device_method = env->GetMethodID(
      midi_mgr_class, "openDevice",
      "(Landroid/media/midi/MidiDeviceInfo;Landroid/media/midi/"
      "MidiManager$OnDeviceOpenedListener;Landroid/os/Handler;)V");

  env->CallVoidMethod(midi_mgr, open_device_method, device_info, callback, handler);

  env->DeleteLocalRef(callback);
  if (handler)
    env->DeleteLocalRef(handler);
}

extern "C" JNIEXPORT void JNICALL Java_dev_celtera_libremidi_MidiDeviceCallback_onDeviceOpened(
    JNIEnv* env, jobject /*thiz*/, jobject midi_device, jlong target_ptr, jboolean is_output)
{
  if (!midi_device || target_ptr == 0)
  {
    LOGE("Invalid device or target pointer in callback");
    return;
  }

  AMidiDevice* amidi_device = nullptr;
  AMidiDevice_fromJava(env, midi_device, &amidi_device);

  if (!amidi_device)
  {
    LOGE("Failed to convert Java MIDI device to AMidiDevice");
    return;
  }

  if (is_output)
  {
    midi_out::open_callback(reinterpret_cast<midi_out*>(target_ptr), amidi_device);
  }
  else
  {
    midi_in::open_callback(reinterpret_cast<midi_in*>(target_ptr), amidi_device);
  }
}

}
