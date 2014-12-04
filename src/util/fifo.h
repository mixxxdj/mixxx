#ifndef FIFO_H
#define FIFO_H

#include <QtDebug>
#include <QMutex>
#include <QScopedPointer>
#include <QSharedPointer>

#include "util/pa_ringbuffer.h"
#include "util/reference.h"
#include "util/math.h"
#include "util.h"

template <class DataType>
class FIFO {
  public:
    explicit FIFO(int size)
            : m_data(NULL) {
        size = roundUpToPowerOf2(size);
        // If we can't represent the next higher power of 2 then bail.
        if (size < 0) {
            return;
        }
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
    DataType* m_data;
    PaUtilRingBuffer m_ringBuffer;
    DISALLOW_COPY_AND_ASSIGN(FIFO<DataType>);
};

// MessagePipe represents one side of a TwoWayMessagePipe. The direction of the
// pipe is with respect to the owner so sender and receiver are
// perspective-dependent. If serializeWrites is true then calls to writeMessages
// will be serialized with a mutex.
template <class SenderMessageType, class ReceiverMessageType>
class MessagePipe {
  public:
    MessagePipe(FIFO<SenderMessageType>& receiver_messages,
                FIFO<ReceiverMessageType>& sender_messages,
                BaseReferenceHolder* pTwoWayMessagePipeReference,
                bool serialize_writes)
            : m_receiver_messages(receiver_messages),
              m_sender_messages(sender_messages),
              m_pTwoWayMessagePipeReference(pTwoWayMessagePipeReference),
              m_bSerializeWrites(serialize_writes) {
    }

    // Returns the number of ReceiverMessageType messages waiting to be read by
    // the receiver. Non-blocking.
    inline int messageCount() const {
        return m_sender_messages.readAvailable();
    }

    // Read a ReceiverMessageType written by the receiver addressed to the
    // sender. Non-blocking.
    inline int readMessages(ReceiverMessageType* messages, int count) {
        return m_sender_messages.read(messages, count);
    }

    // Writes up to 'count' messages from the 'message' array to the receiver
    // and returns the number of successfully written messages. If
    // serializeWrites is active, this method is blocking.
    inline int writeMessages(const SenderMessageType* messages, int count) {
        if (m_bSerializeWrites) {
            m_serializationMutex.lock();
        }
        int result = m_receiver_messages.write(messages, count);
        if (m_bSerializeWrites) {
            m_serializationMutex.unlock();
        }
        return result;
    }

  private:
    QMutex m_serializationMutex;
    FIFO<SenderMessageType>& m_receiver_messages;
    FIFO<ReceiverMessageType>& m_sender_messages;
    QScopedPointer<BaseReferenceHolder> m_pTwoWayMessagePipeReference;
    bool m_bSerializeWrites;

#define COMMA ,
    DISALLOW_COPY_AND_ASSIGN(MessagePipe<SenderMessageType COMMA ReceiverMessageType>);
#undef COMMA
};

// TwoWayMessagePipe is a bare-bones wrapper around the above FIFO class that
// facilitates non-blocking two-way communication. To keep terminology clear,
// there are two sides to the message pipe, the sender side and the receiver
// side. The non-blocking aspect of the underlying FIFO class requires that the
// sender methods and target methods each only be called from a single thread,
// or alternatively guarded with a mutex. The most common use-case of this class
// is sending and receiving messages with the callback thread without the
// callback thread blocking.
//
// This class is an implementation detail and cannot be instantiated
// directly. Use makeTwoWayMessagePipe(...) to create a two-way pipe.
template <class SenderMessageType, class ReceiverMessageType>
class TwoWayMessagePipe {
  public:
    // Creates a TwoWayMessagePipe with SenderMessageType and
    // ReceiverMessageType as the message types. Returns a pair of MessagePipes,
    // the first is the sender's pipe (sends SenderMessageType and receives
    // ReceiverMessageType messages) and the second is the receiver's pipe
    // (sends ReceiverMessageType and receives SenderMessageType messages).
    static QPair<MessagePipe<SenderMessageType, ReceiverMessageType>*,
                 MessagePipe<ReceiverMessageType, SenderMessageType>*> makeTwoWayMessagePipe(
                     int sender_fifo_size,
                     int receiver_fifo_size,
                     bool serialize_sender_writes,
                     bool serialize_receiver_writes) {
        QSharedPointer<TwoWayMessagePipe<SenderMessageType, ReceiverMessageType> > pipe(
            new TwoWayMessagePipe<SenderMessageType, ReceiverMessageType>(
                sender_fifo_size, receiver_fifo_size));

        return QPair<MessagePipe<SenderMessageType, ReceiverMessageType>*,
                     MessagePipe<ReceiverMessageType, SenderMessageType>*>(
                         new MessagePipe<SenderMessageType, ReceiverMessageType>(
                             pipe->m_receiver_messages, pipe->m_sender_messages,
                             new ReferenceHolder<TwoWayMessagePipe<SenderMessageType, ReceiverMessageType> >(pipe),
                             serialize_sender_writes),
                         new MessagePipe<ReceiverMessageType, SenderMessageType>(
                             pipe->m_sender_messages, pipe->m_receiver_messages,
                             new ReferenceHolder<TwoWayMessagePipe<SenderMessageType, ReceiverMessageType> >(pipe),
                             serialize_receiver_writes));
    }

  private:
    TwoWayMessagePipe(int sender_fifo_size, int receiver_fifo_size)
            : m_receiver_messages(receiver_fifo_size),
              m_sender_messages(sender_fifo_size) {
    }

    // Messages waiting to be delivered to the receiver.
    FIFO<SenderMessageType> m_receiver_messages;
    // Messages waiting to be delivered to the sender.
    FIFO<ReceiverMessageType> m_sender_messages;

    // This #define is because the macro gets confused by the template
    // parameters.
#define COMMA ,
    DISALLOW_COPY_AND_ASSIGN(TwoWayMessagePipe<SenderMessageType COMMA ReceiverMessageType>);
#undef COMMA
};

#endif /* FIFO_H */
