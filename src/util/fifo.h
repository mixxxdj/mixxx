#ifndef FIFO_H
#define FIFO_H

#include <QtDebug>

#include "util/pa_ringbuffer.h"
#include "util.h"

template <class DataType>
class FIFO {
  public:
    explicit FIFO(int size) {
        size = roundUpToPowerOf2(size);
        m_data = new DataType[size];
        memset(m_data, 0, sizeof(DataType) * size);
        PaUtil_InitializeRingBuffer(
                &m_ringBuffer, sizeof(DataType), size, m_data);
    }
    virtual ~FIFO() {
        delete [] m_data;
    }
    int readAvailable() const {
        return PaUtil_GetRingBufferReadAvailable(&m_ringBuffer);
    }
    int writeAvailable() const {
        return PaUtil_GetRingBufferWriteAvailable(&m_ringBuffer);
    }
    int read(DataType* pData, int count) {
        return PaUtil_ReadRingBuffer(&m_ringBuffer, pData, count);
    }
    int write(const DataType* pData, int count) {
        return PaUtil_WriteRingBuffer(&m_ringBuffer, pData, count);
    }
    void writeBlocking(const DataType* pData, int count) {
        int written = 0;
        while (written != count) {
            int i = write(pData, count);
            pData += i;
            written += i;
        }
    }
    int aquireWriteRegions(int count,
            DataType** dataPtr1, ring_buffer_size_t* sizePtr1,
            DataType** dataPtr2, ring_buffer_size_t* sizePtr2) {
        return PaUtil_GetRingBufferWriteRegions(&m_ringBuffer, count,
                (void**)dataPtr1, sizePtr1, (void**)dataPtr2, sizePtr2);
    }
    int releaseWriteRegions(int count) {
        return PaUtil_AdvanceRingBufferWriteIndex(&m_ringBuffer, count);
    }
    int aquireReadRegions(int count,
            DataType** dataPtr1, ring_buffer_size_t* sizePtr1,
            DataType** dataPtr2, ring_buffer_size_t* sizePtr2) {
        return PaUtil_GetRingBufferReadRegions(&m_ringBuffer, count,
                (void**)dataPtr1, sizePtr1, (void**)dataPtr2, sizePtr2);
    }
    int releaseReadRegions(int count) {
        return PaUtil_AdvanceRingBufferReadIndex(&m_ringBuffer, count);
    }
  private:
    int roundUpToPowerOf2(int v) {
        // From http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    DataType* m_data;
    PaUtilRingBuffer m_ringBuffer;
    DISALLOW_COPY_AND_ASSIGN(FIFO<DataType>);
};

// TwoWayMessagePipe is a bare-bones wrapper around the above FIFO class that
// facilitates non-blocking two-way communication. To keep terminology clear,
// there are two sides to the message pipe, the sender side and the target side.
// The non-blocking aspect of the underlying FIFO class requires that the sender
// methods and target methods each only be called from a single thread, or
// alternatively guarded with a mutex. The most common use-case of this class is
// sending and receiving messages with the callback thread without the callback
// thread blocking.
template <class SenderMessageType, class TargetMessageType>
class TwoWayMessagePipe {
  public:
    TwoWayMessagePipe(int sender_fifo_size, int target_fifo_size)
            : m_target_messages(target_fifo_size),
              m_sender_messages(sender_fifo_size) {
    }

    ////////////////////////////////////////////////////////////////////////////
    // Target methods. These should only be called from the target thread. Wrap
    // with a mutex to make these methods callable from any thread.
    ////////////////////////////////////////////////////////////////////////////

    // Returns the number of SenderMessageType messages waiting to be read by
    // the target.
    inline int targetMessageCount() const {
        return m_target_messages.readAvailable();
    }

    // Read SenderMessageType messages from the sender up to a maximum of
    // 'count'. Returns the number of messages written to 'messages'.
    inline int targetReadMessages(SenderMessageType* messages, int count) {
        return m_target_messages.read(messages, count);
    }

    // Writes up to 'count' messages from the 'message' array to the sender and
    // returns the number of successfully written messages.
    inline int targetWriteMessage(const TargetMessageType* messages, int count) {
        return m_sender_messages.write(messages, count);
    }


    ////////////////////////////////////////////////////////////////////////////
    // Sender methods. These should only be called by the sender thread. Wrap
    // with a mutex to make these methods callable from any thread.
    ////////////////////////////////////////////////////////////////////////////

    // Returns the number of TargetMessageType messages waiting to be read by
    // the sender.
    inline int senderMessageCount() const {
        return m_sender_messages.readAvailable();
    }

    // Read a TargetMessageType written by the target addressed to the
    // sender.
    inline int senderReadMessages(TargetMessageType* messages, int count) {
        return m_sender_messages.read(messages, count);
    }

    // Writes up to 'count' messages from the 'message' array to the target and
    // returns the number of successfully written messages.
    inline int senderWriteMessage(const SenderMessageType* messages, int count) {
        return m_target_messages.write(messages, count);
    }

  private:
    // Messages waiting to be delivered to the target.
    FIFO<SenderMessageType> m_target_messages;
    // Messages waiting to be delivered to the sender.
    FIFO<TargetMessageType> m_sender_messages;

    // This #define is because the macro gets confused by the template
    // parameters.
#define COMMA ,
    DISALLOW_COPY_AND_ASSIGN(TwoWayMessagePipe<SenderMessageType COMMA TargetMessageType>);
#undef COMMA
};

#endif /* FIFO_H */
