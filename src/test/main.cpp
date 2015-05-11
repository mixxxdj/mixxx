#include <gtest/gtest.h>

#ifdef __WINDOWS__
#include <windows.h>
#include <io.h> // Debug Console
#endif // __WINDOWS__

int main(int argc, char **argv) {

#ifdef __WINDOWS__
    // Try to attach to the console of the calling process
    // or create a new one.
    if(!AttachConsole(ATTACH_PARENT_PROCESS)) {
        // we are started via double click. create a new console
        AllocConsole() {
            SetConsoleTitleA("mixxx-test");
        }
    }

    int fd;
    FILE *fp;

    fd = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), 0);
    fp = _fdopen(fd, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    fd = _open_osfhandle((long) GetStdHandle(STD_ERROR_HANDLE), 0);
    fp = _fdopen(fd, "w");
    *stderr = *fp;
    setvbuf(stderr, NULL, _IONBF, 0);
#endif // __WINDOWS_

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
