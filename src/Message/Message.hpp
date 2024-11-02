/**
 * @file Message.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-02
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __MESSAGE_MESSAGE_HPP
#define __MESSAGE_MESSAGE_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace message {

class Message final {
public:
  enum class Type : uint8_t {
    Hello,            // No payload
    Goodbye,          // No payload
    NewComment,       // Dynamic payload (NewCommentPayload)
    CommentsRequest,  // Dynamic payload (CommentsRequestPayload)
    CommentOk,        // No payload
    CommentsResponse  // Dynamic payload (CommentsResponsePayload)
  };

  // Non-Copyable
  Message(const Message&) = delete;
  Message& operator=(const Message&) = delete;

  // Movable
  Message(Message&&) noexcept;
  Message& operator=(Message&&) noexcept;

  static Message hello(void);

  static Message goodbye(void);

  static Message newComment(std::string comment);
  
  static Message getComments(uint32_t start_index);

  static Message commentOk(void);

  static Message sendComments(
      const std::vector<std::string> comments,
      size_t start_index,
      size_t send_count
  );

  static std::optional<Message> fromBytes(
      std::span<const std::byte> bytes,
      size_t& need_bytes
  );

  std::span<const std::byte> getBytes(void) const;


  Message() = delete;

  Type getType(void) const noexcept;

  ~Message() {
    if (m_payload != nullptr) {
      auto* bytes = reinterpret_cast<std::byte*>(m_payload);
      delete[] bytes;
    }
  }

private:
  struct MessageHeader {
    char magic[3];
    Type type;
    uint32_t payload_size;
  } __attribute__((packed));

  struct NewCommentPayload {
    char comment[];
  };

  struct CommentsRequestPayload {
    uint32_t start_index;
  };

  struct CommentsResponsePayload {
    uint32_t total_comments;
    uint32_t sent_comments;

    char comments[];  // NUL-separated strings
  };

  struct DynamicMessage {
    MessageHeader header;
    std::byte payload[];
  };

  explicit Message(const MessageHeader& header)
    : m_header(header), m_payload(nullptr) {
  }

  explicit Message(DynamicMessage* message)
    : Message(message->header) {
    m_payload = message;
  }

  static std::optional<MessageHeader> parseHeader(
      std::span<std::byte, sizeof(MessageHeader)> bytes
  );

  static DynamicMessage* parseCommentsRequest(
      const MessageHeader& header,
      std::span<std::byte> bytes
  );

  static DynamicMessage* parseCommentsResponse(
      const MessageHeader& header,
      std::span<std::byte> bytes
  );

  static DynamicMessage* parseNewComment(
      const MessageHeader& header,
      std::span<std::byte> bytes
  );

  MessageHeader m_header;
  DynamicMessage* m_payload = NULL;
};

} // namespace message

#endif /* Message.hpp */
