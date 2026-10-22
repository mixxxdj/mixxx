// Stub for the NDK-removed libandroid-support.so.
//
// The youtubedl-android AAR's libpython.so declares DT_NEEDED
// libandroid-support.so (a pre-NDK-r23 compat shim), but references zero
// symbols from it (verified: only undefined symbols are Py_BytesMain,
// __cxa_atexit, __libc_init, __register_atfork — all from libc/libpython).
// A stub with no symbols satisfies the dynamic linker without changing
// behavior. Without it, initPython() fails with dlopen ENOENT and
// YoutubeDL.init() throws "failed to initialize".
void ytdlp_android_support_stub_holder(void) {
}
