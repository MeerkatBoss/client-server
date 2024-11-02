/**
 * @file NewCommentMessage.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-02
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __MESSAGE_NEW_COMMENT_MESSAGE_HPP
#define __MESSAGE_NEW_COMMENT_MESSAGE_HPP

#include <optional>
#include <span>

#include "Message/Message.hpp"

namespace message {

class NewCommentMessage final {
public:
  static std::optional<NewCommentMessage> fromMessage(Message&& message);

  // Non-Copyable
  NewCommentMessage(const NewCommentMessage&) = delete;
  NewCommentMessage& operator=(const NewCommentMessage&) = delete;

  // Movable
  NewCommentMessage(NewCommentMessage&&) noexcept = default;
  NewCommentMessage& operator=(NewCommentMessage&&) noexcept = default;

  std::span<const char> getComment(void) const;
private:
  explicit NewCommentMessage(std::span<std::byte> bytes);

  Message m_message;
};

} // namespace message

#endif /* NewCommentMessage.hpp */
