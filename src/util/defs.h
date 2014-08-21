#ifndef DEFS_H
#define DEFS_H

// Used for returning errors from functions.
enum Result {
    OK = 0,
    ERR = -1
};

// Maximum buffer length to each EngineObject::process call.
const unsigned int MAX_BUFFER_LEN = 160000;

#endif /* DEFS_H */
