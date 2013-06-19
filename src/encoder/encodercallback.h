#ifndef ENCODERCALLBACK_H
#define ENCODERCALLBACK_H

class EncoderCallback {
  public:
    // writes to encoded audio to a stream, e.g., a file stream or shoutcast stream
    virtual void write(unsigned char *header, unsigned char *body,
                       int headerLen, int bodyLen) = 0;
};

#endif /* ENCODERCALLBACK_H */

