#pragma once

#include <QScopedPointer>
#include <QSharedPointer>

#include "rigtorp/SPSCQueue.h"

#include "util/class.h"
#include "util/reference.h"

// MessagePipe represents one side of a TwoWayMessagePipe. The direction of the
// pipe is with respect to the owner so sender and receiver are
// perspective-dependent.
template <class SenderMessageType, class ReceiverMessageType>
class MessagePipe {
  public:
    MessagePipe(rigtorp::SPSCQueue<SenderMessageType>& receiver_messages,
                rigtorp::SPSCQueue<ReceiverMessageType>& sender_messages,
                BaseReferenceHolder* pTwoWayMessagePipeReference)
            : m_receiver_messages(receiver_messages),
              m_sender_messages(sender_messages),
              m_pTwoWayMessagePipeReference(pTwoWayMessagePipeReference) {
    }

    // Returns the number of ReceiverMessageType messages waiting to be read by
    // the receiver. Non-blocking.
    int messageCount() const {
        return m_sender_messages.size();
    }

    // Try to read read a ReceiverMessageType written by the receiver
    // addressed to the sender. Non-blocking.
    bool readMessage(ReceiverMessageType* message) {
        auto front = m_sender_messages.front();
        if (!front) {
            return false;
        }
        *message = std::move(*front);
        m_sender_messages.pop();
        return true;
    }

    // Writes a message to the receiver and returns true on success.
    bool writeMessage(const SenderMessageType& message) {
        return m_receiver_messages.try_push(message);
    }

  private:
    rigtorp::SPSCQueue<SenderMessageType>& m_receiver_messages;
    rigtorp::SPSCQueue<ReceiverMessageType>& m_sender_messages;
    QScopedPointer<BaseReferenceHolder> m_pTwoWayMessagePipeReference;

    DISALLOW_COPY_AND_ASSIGN(MessagePipe);
};

// TwoWayMessagePipe is a bare-bones wrapper around the above rigtorp::SPSCQueue class that
// facilitates non-blocking two-way communication. To keep terminology clear,
// there are two sides to the message pipe, the sender side and the receiver
// side. The non-blocking aspect of the underlying rigtorp::SPSCQueue class requires that the
// sender methods and target methods each only be called from a single thread
// The most common use-case of this class is sending and receiving messages
// with the callback thread without the callback thread blocking.
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
    static std::pair<
            std::unique_ptr<MessagePipe<SenderMessageType, ReceiverMessageType>>,
            std::unique_ptr<MessagePipe<ReceiverMessageType, SenderMessageType>>>
    makeTwoWayMessagePipe(
            int sender_fifo_size,
            int receiver_fifo_size) {
        QSharedPointer<TwoWayMessagePipe<SenderMessageType, ReceiverMessageType>> pipe(
                new TwoWayMessagePipe<SenderMessageType, ReceiverMessageType>(
                        sender_fifo_size, receiver_fifo_size));

        return {
                std::make_unique<MessagePipe<SenderMessageType, ReceiverMessageType>>(
                        pipe->m_receiver_messages,
                        pipe->m_sender_messages,
                        new ReferenceHolder<TwoWayMessagePipe<SenderMessageType,
                                ReceiverMessageType>>(pipe)),
                std::make_unique<MessagePipe<ReceiverMessageType, SenderMessageType>>(
                        pipe->m_sender_messages,
                        pipe->m_receiver_messages,
                        new ReferenceHolder<TwoWayMessagePipe<SenderMessageType,
                                ReceiverMessageType>>(pipe))};
    }

  private:
    TwoWayMessagePipe(int sender_fifo_size, int receiver_fifo_size)
            : m_receiver_messages(receiver_fifo_size),
              m_sender_messages(sender_fifo_size) {
    }

    // Messages waiting to be delivered to the receiver.
    rigtorp::SPSCQueue<SenderMessageType> m_receiver_messages;
    // Messages waiting to be delivered to the sender.
    rigtorp::SPSCQueue<ReceiverMessageType> m_sender_messages;

    DISALLOW_COPY_AND_ASSIGN(TwoWayMessagePipe);
};
