#include <gtest/gtest.h>
#ifdef __FFMPEGFILE__
extern "C" {
#include <libavformat/avformat.h>
}
#endif

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
#ifdef __FFMPEGFILE__
    av_register_all();
#endif
    return RUN_ALL_TESTS();
}
