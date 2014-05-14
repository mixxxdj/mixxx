#ifndef SLEEP_H
#define SLEEP_H

#ifdef __WINDOWS__
#include <windows.h>
//sleep on linux assumes seconds where as Sleep on Windows assumes milliseconds
#define sleep(x) Sleep(x*1000)
#else
#include <unistd.h>
#endif

#endif /* SLEEP_H */
