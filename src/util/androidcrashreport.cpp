#include "util/androidcrashreport.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>

#include <QDir>
#include <QFile>
#include <cstdint>
#include <cstring>

#include "util/versionstore.h"

namespace {

constexpr int kSignals[] = {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT, SIGTRAP};
constexpr int kSignalCount = sizeof(kSignals) / sizeof(kSignals[0]);
constexpr int kMaxFrames = 64;

struct sigaction s_previousActions[kSignalCount];
// Prepared up front: only async-signal-safe calls are made while crashing
char s_directory[512];
char s_build[128];

void restorePreviousActions() {
    for (int i = 0; i < kSignalCount; ++i) {
        sigaction(kSignals[i], &s_previousActions[i], nullptr);
    }
}

void writeString(int fd, const char* pText) {
    [[maybe_unused]] const auto written = write(fd, pText, strlen(pText));
}

void writeNumber(int fd, uint64_t value, int base) {
    char buffer[24];
    int pos = sizeof(buffer);
    buffer[--pos] = '\0';
    do {
        buffer[--pos] = "0123456789abcdef"[value % base];
        value /= base;
    } while (value != 0 && pos > 0);
    writeString(fd, buffer + pos);
}

/// Removes the pointer authentication code from a return address
uintptr_t stripPointerAuthentication(uintptr_t address) {
#if defined(__aarch64__)
    // XPACLRI, a no-op on CPUs without pointer authentication
    register uintptr_t x30 asm("x30") = address;
    asm("hint #7" : "+r"(x30));
    return x30;
#else
    return address;
#endif
}

void writeAddress(int fd, int index, uintptr_t address) {
    address = stripPointerAuthentication(address);
    writeString(fd, "#");
    writeNumber(fd, index, 10);
    writeString(fd, " pc 0x");
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(address), &info) != 0 && info.dli_fname) {
        writeNumber(fd, address - reinterpret_cast<uintptr_t>(info.dli_fbase), 16);
        writeString(fd, " ");
        const char* pName = strrchr(info.dli_fname, '/');
        writeString(fd, pName ? pName + 1 : info.dli_fname);
        if (info.dli_sname) {
            writeString(fd, " (");
            writeString(fd, info.dli_sname);
            writeString(fd, ")");
        }
    } else {
        writeNumber(fd, address, 16);
    }
    writeString(fd, "\n");
}

void onCrash(int signalNumber, siginfo_t* pInfo, void* /*pContext*/) {
    // A fault while writing the report goes to the system's handler
    restorePreviousActions();

    char path[sizeof(s_directory) + 48];
    strcpy(path, s_directory);
    char* pEnd = path + strlen(path);
    strcpy(pEnd, "/crash-");
    pEnd += strlen(pEnd);
    // The time as a number, without the non-reentrant time formatting
    uint64_t seconds = static_cast<uint64_t>(time(nullptr));
    char digits[24];
    int count = 0;
    do {
        digits[count++] = static_cast<char>('0' + seconds % 10);
        seconds /= 10;
    } while (seconds != 0);
    while (count > 0) {
        *pEnd++ = digits[--count];
    }
    strcpy(pEnd, ".txt");

    const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        raise(signalNumber);
        return;
    }

    writeString(fd, "Mixxx crash report\n");
    writeString(fd, "Build: ");
    writeString(fd, s_build);
    writeString(fd, "\n");
    writeString(fd, "Signal: ");
    writeNumber(fd, signalNumber, 10);
    writeString(fd, "\n");
    writeString(fd, "Fault address: 0x");
    writeNumber(fd, reinterpret_cast<uintptr_t>(pInfo->si_addr), 16);
    writeString(fd, "\n\n");

    // Walk the stack using frame pointers (works on ARM64 and x86_64 with -fno-omit-frame-pointer)
    uintptr_t fp;
#if defined(__aarch64__)
    asm("mov %0, x29" : "=r"(fp));
#elif defined(__x86_64__)
    asm("mov %0, rbp" : "=r"(fp));
#else
    fp = 0;
#endif

    if (fp != 0) {
        for (int i = 0; i < kMaxFrames; ++i) {
            // On ARM64, the frame record is [fp, lr]; on x86_64 it's [rbp, rip]
            const uintptr_t* frame = reinterpret_cast<const uintptr_t*>(fp);
            if (!frame) {
                break;
            }
            const uintptr_t lr = frame[1];
            if (lr == 0) {
                break;
            }
            writeAddress(fd, i, lr);
            fp = frame[0];
            if (fp < 0x1000 || fp == reinterpret_cast<uintptr_t>(frame)) {
                break;
            }
        }
    }
    writeString(fd, "\n");
    close(fd);

    raise(signalNumber);
}

} // namespace

namespace mixxx {
namespace android {

QString crashReportDirectory() {
    const QString path = QDir::toNativeSeparators(
            QDir::homePath() + "/Mixxx/logs");
    QDir().mkpath(path);
    return path;
}

void installCrashReporter() {
    // Prepare the directory string once (not async-signal-safe)
    const QString dir = crashReportDirectory();
    if (static_cast<size_t>(dir.length()) >= sizeof(s_directory)) {
        return;
    }
    strcpy(s_directory, dir.toUtf8().constData());

    // Prepare the build string
    const QString build = VersionStore::gitVersion();
    if (static_cast<size_t>(build.length()) >= sizeof(s_build)) {
        strncpy(s_build, build.toUtf8().constData(), sizeof(s_build) - 1);
        s_build[sizeof(s_build) - 1] = '\0';
    } else {
        strcpy(s_build, build.toUtf8().constData());
    }

    struct sigaction action = {};
    action.sa_sigaction = onCrash;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&action.sa_mask);

    for (int i = 0; i < kSignalCount; ++i) {
        sigaction(kSignals[i], &action, &s_previousActions[i]);
    }
}

void exportPreviousLog(const QString& logDirPath) {
    const QString previousLog = logDirPath + "/mixxx.log.1";
    const QString targetLog = crashReportDirectory() + "/mixxx-previous.log";
    QFile::copy(previousLog, targetLog);
}

} // namespace android
} // namespace mixxx
