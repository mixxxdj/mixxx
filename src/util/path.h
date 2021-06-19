#pragma once

#ifndef PATH_MAX
#ifndef MAX_PATH
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247%28v=vs.85%29.aspx
const int MAX_PATH = 260;
#endif
// Use POSIX name for MAX_PATH
enum {
    PATH_MAX = MAX_PATH
};
#endif
