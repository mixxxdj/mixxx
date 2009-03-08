

cd mp4v2-2.0-20090110
cat file_win32.cpp.patch | patch

./configure --disable-static --disable-debug
make LDFLAGS="-no-undefined"