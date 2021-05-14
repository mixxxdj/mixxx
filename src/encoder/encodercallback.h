#pragma once

class EncoderCallback {
  public:
    // writes to encoded audio to a stream, e.g., a file stream or broadcast stream
    virtual void write(const unsigned char *header, const unsigned char *body,
                       int headerLen, int bodyLen) = 0;
    // gets stream position
    virtual int tell() = 0;
    // sets stream position
    virtual void seek(int pos) = 0;
    // gets stream length
    virtual int filelen() = 0;
};
