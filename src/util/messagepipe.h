#pragma once

#include <gsl/pointers>
#include <memory>

#include "rigtorp/SPSCQueue.h"

// MessagePipe represents one side of a TwoWayMessagePipe. The direction of the
// pipe is with respect to the owner so sender and receiver are
// perspective-dependent.
template <class SenderMessageType, class ReceiverMessageType>
class MessagePipe {
    template<typename T>
    using not_null_shared_queue = gsl::not_null<std::shared_ptr<rigtorp::SPSCQueue<T>>>;

  public:
    MessagePipe(not_null_shared_queue<SenderMessageType> receiver_messages,
            not_null_shared_queue<ReceiverMessageType> sender_messages)
            : m_receiver_messages(std::move(receiver_messages)),
              m_sender_messages(std::move(sender_messages)) {
    }

    // Disallow copying (could violate the Single-consumer/Single-producer constraint)
    // allow moving since that does not violate the constraint
    MessagePipe(const MessagePipe&) = delete;
    MessagePipe& operator=(const MessagePipe&) = delete;
    MessagePipe(MessagePipe&&) noexcept = default;
    MessagePipe& operator=(MessagePipe&&) noexcept = default;
    ~MessagePipe() = default;

    // Returns the number of ReceiverMessageType messages waiting to be read by
    // the receiver. Non-blocking.
    int messageCount() const {
        return m_sender_messages->size();
    }

    // Try to read read a ReceiverMessageType written by the receiver
    // addressed to the sender. Non-blocking.
    bool readMessage(ReceiverMessageType* message) {
        auto front = m_sender_messages->front();
        if (!front) {
            return false;
        }
        *message = std::move(*front);
        m_sender_messages->pop();
        return true;
    }

    // Writes a message to the receiver and returns true on success.
    bool writeMessage(const SenderMessageType& message) {
        return m_receiver_messages->try_push(message);
    }

  private:
    not_null_shared_queue<SenderMessageType> m_receiver_messages;
    not_null_shared_queue<ReceiverMessageType> m_sender_messages;
};

/// makeTwoWayMessagePipe is a bare-bones wrapper around the above rigtorp::SPSCQueue class that
/// facilitates non-blocking two-way communication. To keep terminology clear,
/// there are two sides to the message pipe, the sender side and the receiver
/// side. The non-blocking aspect of the underlying rigtorp::SPSCQueue class requires that the
/// sender methods and target methods each only be called from a single thread
/// The most common use-case of this class is sending and receiving messages
/// with the callback thread without the callback thread blocking.
/// Returns a pair of MessagePipes,
/// the first is the sender's pipe (sends SenderMessageType and receives
/// ReceiverMessageType messages) and the second is the receiver's pipe
/// (sends ReceiverMessageType and receives SenderMessageType messages).
template<class SenderMessageType, class ReceiverMessageType>
static std::pair<
        MessagePipe<SenderMessageType, ReceiverMessageType>,
        MessagePipe<ReceiverMessageType, SenderMessageType>>
makeTwoWayMessagePipe(
        int sender_fifo_size,
        int receiver_fifo_size) {
    auto sender_pipe = gsl::make_not_null(
            std::make_shared<rigtorp::SPSCQueue<ReceiverMessageType>>(
                    sender_fifo_size));
    auto receiver_pipe =
            gsl::make_not_null(std::make_shared<rigtorp::SPSCQueue<SenderMessageType>>(
                    receiver_fifo_size));
    MessagePipe<SenderMessageType, ReceiverMessageType> requestPipe(receiver_pipe, sender_pipe);
    MessagePipe<ReceiverMessageType, SenderMessageType> responsePipe(
            std::move(sender_pipe), std::move(receiver_pipe));
    return {std::move(requestPipe), std::move(responsePipe)};
};
