* minGW is not officially supported by mp4v2
* libmp4v2-0.dll is from the snapshot relese of mp4v2-2.0-20090110 (from http://code.google.com/p/mp4v2/)
* you may need to apply the file_win32.cpp.patch depending on weither your mingw
* finally building as a DLL with no debug info worked best, as well as telling ld not to allow undefines.
./configure --disable-static --disable-debug
make LDFLAGS="-no-undefined"