#include "SendCommentsMessage.hpp"
#include "Message/Message.hpp"
#include <netinet/in.h>

namespace message {

SendCommentsMessage::SendCommentsMessage(Message&& message)
  : m_message(std::move(message)), m_comments(0) {
  const auto* payload = 
    reinterpret_cast<const Message::CommentsResponsePayload*> (
        m_message.m_payload->payload
    );

  size_t size = ntohl(m_message.m_header.payload_size) 
                - sizeof(Message::CommentsResponsePayload);
  const char* chars = payload->comments;

  size_t offset = 0;
  size_t length = 0;

  while (offset + length < size) {
    if (chars[offset + length] == '\0') {
      m_comments.push_back(std::span(chars+offset, length));
      offset += length + 1;
      length = 0;
    }
    ++length;
  }
}


} // namespace message
