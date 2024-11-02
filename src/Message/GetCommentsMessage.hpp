/**
 * @file GetCommentsMessage.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-02
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __MESSAGE_GET_COMMENTS_MESSAGE_HPP
#define __MESSAGE_GET_COMMENTS_MESSAGE_HPP

#include <optional>

#include "Message/Message.hpp"

namespace message {

class GetCommentsMessage final {
public:
  static std::optional<GetCommentsMessage> fromMessage(Message&& message);

  // Non-Copyable
  GetCommentsMessage(const GetCommentsMessage&) = delete;
  GetCommentsMessage& operator=(const GetCommentsMessage&) = delete;

  // Movable
  GetCommentsMessage(GetCommentsMessage&&) noexcept = default;
  GetCommentsMessage& operator=(GetCommentsMessage&&) noexcept = default;
  
  size_t getStartIndex(void) const;

private:
  explicit GetCommentsMessage(Message&& message);
  Message m_message;
};

} // namespace message

#endif /* GetCommentsMessage.hpp */
