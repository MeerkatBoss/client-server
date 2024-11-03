#include "Message.hpp"
#include <cassert>
#include <netinet/in.h>
#include <new>
#include <algorithm>
#include <optional>

namespace message {

static constexpr char Magic[3] = { 'M', 'S', 'G' };

Message::Message(Message&& other) noexcept
  : m_header(other.m_header),
    m_payload(other.m_payload)
{
  other.m_header.type = Type::Hello;
  other.m_header.payload_size = 0;
  other.m_payload = nullptr;
}

Message& Message::operator=(Message&& other) noexcept {
  if (m_payload != nullptr) {
      auto* bytes = reinterpret_cast<std::byte*>(m_payload);
      delete[] bytes;
  }

  m_header = other.m_header;
  m_payload = other.m_payload;

  other.m_header.type = Type::Hello;
  other.m_header.payload_size = 0;
  other.m_payload = nullptr;

  return *this;
}

Message Message::hello(void) {
  MessageHeader header;
  std::copy_n(Magic, sizeof(Magic), header.magic);
  header.type = Type::Hello;
  header.payload_size = 0;

  return Message(header);
}

Message Message::goodbye(void) {
  MessageHeader header;
  std::copy_n(Magic, sizeof(Magic), header.magic);
  header.type = Type::Goodbye;
  header.payload_size = 0;

  return Message(header);
}

Message Message::commentOk(void) {
  MessageHeader header;
  std::copy_n(Magic, sizeof(Magic), header.magic);
  header.type = Type::CommentOk;
  header.payload_size = 0;

  return Message(header);
}

Message::DynamicMessage* Message::allocateDynamic(Type type, size_t alloc_size) {
  assert(alloc_size > sizeof(MessageHeader));
  std::byte* bytes =
    new(std::align_val_t(alignof(DynamicMessage))) std::byte[alloc_size];

  DynamicMessage* message = reinterpret_cast<DynamicMessage*>(bytes);
  MessageHeader& header = message->header;

  std::copy_n(Magic, sizeof(Magic), header.magic);
  header.type = type;
  header.payload_size = htonl(alloc_size - sizeof(MessageHeader));
  
  return message;
}

Message Message::newComment(std::string comment) {
  const size_t alloc_size = sizeof(DynamicMessage) + comment.length();

  DynamicMessage* message = allocateDynamic(Type::NewComment, alloc_size);
  char* chars = reinterpret_cast<char*>(message->payload);
  std::copy_n(comment.begin(), comment.length(), chars);

  return Message(message);
}

Message Message::getComments(uint32_t start_index) {
  const size_t alloc_size =
    sizeof(DynamicMessage) + sizeof(CommentsRequestPayload);

  DynamicMessage* message = allocateDynamic(Type::CommentsRequest, alloc_size);

  CommentsRequestPayload& payload =
    *reinterpret_cast<CommentsRequestPayload*>(message->payload);
  payload.start_index = htonl(start_index);

  return Message(message);
}

Message Message::sendComments(
    const std::vector<std::string> comments,
    size_t start_index,
    size_t send_count
) {
  assert(send_count == 0 || start_index < comments.size());
  assert(send_count == 0 || start_index + send_count <= comments.size());

  size_t total_length = 0;
  for (size_t i = start_index; i < start_index + send_count; ++i) {
    total_length += comments[i].length() + 1;
  }

  const size_t alloc_size =
    sizeof(DynamicMessage) + sizeof(CommentsResponsePayload) + total_length;

  DynamicMessage* message = allocateDynamic(Type::CommentsResponse, alloc_size);
  CommentsResponsePayload& payload =
    *reinterpret_cast<CommentsResponsePayload*>(message->payload);
  payload.total_comments = htonl(comments.size());
  payload.sent_comments = htonl(send_count);

  char* chars = payload.comments;
  for (size_t i = start_index; i < start_index + send_count; ++i) {
    std::copy_n(comments[i].begin(), comments[i].length(), chars);
    chars[comments[i].length()] = '\0';
    chars += comments[i].length() + 1;
  }

  return Message(message);
}

std::span<const std::byte> Message::getBytes(void) const {
  if (m_payload == nullptr) {
    return std::span(
        reinterpret_cast<const std::byte*> (&m_header),
        sizeof(m_header)
    );
  }

  return std::span(
      reinterpret_cast<const std::byte*> (m_payload),
      sizeof(MessageHeader) + m_header.payload_size
  );
}

std::optional<Message> Message::fromBytes(
    std::span<const std::byte> bytes,
    size_t& message_size
) {
  if (bytes.size() < MinSize) {
    return std::nullopt;
  }

  const MessageHeader& header =
    *reinterpret_cast<const MessageHeader*> (bytes.data());

  bool has_magic = std::equal(Magic, Magic + sizeof(Magic), header.magic);
  if (!has_magic) {
    return std::nullopt;
  }

  size_t payload_size = ntohl(header.payload_size);

  if (payload_size == 0) {
    bool has_valid_type =
      header.type == Type::Hello ||
      header.type == Type::Goodbye ||
      header.type == Type::CommentOk;
    bool has_valid_size = (bytes.size() == sizeof(MessageHeader));
    
    if (has_valid_type && has_valid_size) {
      message_size = sizeof(MessageHeader);
      return Message(header);
    } else {
      return std::nullopt;
    }
  }
  
  if (header.type == Type::CommentsRequest) {
    size_t full_size = sizeof(MessageHeader) + sizeof(CommentsRequestPayload);

    if (payload_size != sizeof(CommentsRequestPayload)) {
      return std::nullopt;
    }
    if (bytes.size() > full_size) {
      return std::nullopt;
    }

    message_size = full_size;
    if (bytes.size() < full_size) {
      return std::nullopt;
    }

    DynamicMessage* message = allocateDynamic(Type::CommentsRequest, full_size);
    std::copy(
        bytes.begin(), bytes.end(),
        reinterpret_cast<std::byte*> (message)
    );

    return Message(message);
  }

  if (
    header.type == Type::CommentsResponse ||
    header.type == Type::NewComment
  ) {
    size_t full_size = sizeof(MessageHeader) + payload_size;
    if (bytes.size() > full_size) {
      return std::nullopt;
    }

    message_size = full_size;
    if (bytes.size() < full_size) {
      return std::nullopt;
    }

    DynamicMessage* message = allocateDynamic(Type::CommentsRequest, full_size);
    std::copy(
        bytes.begin(), bytes.end(),
        reinterpret_cast<std::byte*> (message)
    );

    return Message(message);
  }

  return std::nullopt;
}


} // namespace message
