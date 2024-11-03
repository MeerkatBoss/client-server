/**
 * @file SendCommentsMessage.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-02
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __MESSAGE_SEND_COMMENTS_MESSAGE_HPP
#define __MESSAGE_SEND_COMMENTS_MESSAGE_HPP

#include "Message/Message.hpp"
#include <netinet/in.h>
#include <optional>
#include <vector>

namespace message {

class SendCommentsMessage final {
public:
  static std::optional<SendCommentsMessage> fromMessage(Message&& message) {
    if (message.getType() == Message::Type::CommentsRequest) {
      return SendCommentsMessage(std::move(message));
    }
    return std::nullopt;
  }

  // Non-Copyable
  SendCommentsMessage(const SendCommentsMessage&) = delete;
  SendCommentsMessage& operator=(const SendCommentsMessage&) = delete;

  // Movable
  SendCommentsMessage(SendCommentsMessage&&) noexcept = default;
  SendCommentsMessage& operator=(SendCommentsMessage&&) noexcept = default;

  size_t getCount(void) const {
    return m_comments.size();
  }

  size_t getTotal(void) const {
    const auto* payload = 
      reinterpret_cast<const Message::CommentsResponsePayload*> (
          m_message.m_payload->payload
      );
    return ntohl(payload->total_comments);
  }

  std::span<const char> operator[](size_t index) const {
    return m_comments[index];
  }

private:
  explicit SendCommentsMessage(Message&& message);

  Message m_message;
  std::vector<std::span<const char>> m_comments;
};

} // namespace message

#endif /* SendCommentsMessage.hpp */
