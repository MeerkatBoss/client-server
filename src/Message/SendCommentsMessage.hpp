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
#include <optional>
#include <vector>

namespace message {

class SendCommentsMessage final {
public:
  static std::optional<SendCommentsMessage> fromMessage(Message&& message);

  // Non-Copyable
  SendCommentsMessage(const SendCommentsMessage&) = delete;
  SendCommentsMessage& operator=(const SendCommentsMessage&) = delete;

  // Movable
  SendCommentsMessage(SendCommentsMessage&&) noexcept = default;
  SendCommentsMessage& operator=(SendCommentsMessage&&) noexcept = default;

  size_t getCount(void) const;

  std::span<const unsigned char> operator[](size_t index) const;

private:
  explicit SendCommentsMessage(Message&& message);

  Message m_message;
  std::vector<std::span<const unsigned char>> comments;
};

} // namespace message

#endif /* SendCommentsMessage.hpp */
