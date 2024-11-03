#include "TcpServer.hpp"
#include "Message/GetCommentsMessage.hpp"
#include "Message/Message.hpp"
#include "Message/NewCommentMessage.hpp"

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

namespace server {

static constexpr size_t BacklogSize = 16;

static int make_listen_socket(uint8_t ip_address[4], uint16_t port);
static void serve_client(int socket, std::vector<std::string>& comments);

static void interrupt_handler(int) { /* Enter handler but do nothing */ }
static void setup_interrupt_handler() {
  int res = 0;
  struct sigaction action;
  action.sa_handler = &interrupt_handler;
  action.sa_flags = 0;
  res = sigemptyset(&action.sa_mask);
  assert(res == 0);

  res = sigaction(SIGINT, &action, NULL);
  assert(res == 0);
}

void listen_tcp(uint8_t ip_address[4], uint16_t port) {
  int listener = make_listen_socket(ip_address, port);

  std::vector<std::string> comments;
  setup_interrupt_handler();

  for (;;) {
    errno = 0;
    int client = accept(listener, NULL, NULL);
    if (client == -1 && errno == EINTR) {
      break;
    }
    assert(client >= 0);

    serve_client(client, comments);
    if (errno == EINTR) {
      break;
    }
  }

  puts("");
  puts("Server stopped");
}

static int make_listen_socket(uint8_t ip_address[4], uint16_t port) {
  constexpr size_t IpAddrMaxLength = 12 + 3;  // 12 digits and 3 dots
  static char addr_buffer[IpAddrMaxLength + 1] = "";
  int res = 0;

  snprintf(addr_buffer, IpAddrMaxLength,
      "%hhd.%hhd.%hhd.%hhd",
      ip_address[0], ip_address[1], ip_address[2], ip_address[3]
  );

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = port;
  res = inet_aton(addr_buffer, &address.sin_addr);
  assert(res == 1);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(fd >= 0);
  res = bind(fd, (const struct sockaddr*) &address, sizeof(address));
  assert(res == 0);
  res = listen(fd, BacklogSize);
  assert(res == 0);

  return fd;
}

static void add_comment(
    int socket,
    message::NewCommentMessage message,
    std::vector<std::string> &comments
);
static void send_comments(
    int socket,
    message::GetCommentsMessage message,
    std::vector<std::string> &comments
);

static void serve_client(int socket, std::vector<std::string>& comments) {
  std::vector<std::byte> buffer(message::Message::MinSize);
  size_t offset = 0;
  size_t read_size = message::Message::MinSize;
  bool conn_closed = false;

  while (!conn_closed) {
    errno = 0;
    int res = recv(socket, buffer.data() + offset, read_size, MSG_WAITALL);
    
    if (res <= 0) {
      break;
    }

    const size_t actual_size = (size_t) res;
    offset += actual_size;
    read_size -= actual_size;

    // Partial read, continue reading
    if (read_size > 0) {
      continue;
    }

    size_t msg_size = 0;
    auto msg = message::Message::fromBytes(
        std::span(buffer.data(), offset),
        msg_size
    );
    assert(msg_size > 0);

    // Message has more bytes, continue reading
    if (msg_size > offset) {
      read_size = msg_size - offset;
      if (buffer.size() < msg_size) {
        buffer.resize(msg_size);
      }
      continue;
    }

    read_size = message::Message::MinSize;
    offset = 0;
    buffer.assign(read_size, std::byte{});

    auto message(std::move(*msg));

    using Type = message::Message::Type;
    
    switch (message.getType()) {
    case Type::NewComment:
      add_comment(
          socket,
          *message::NewCommentMessage::fromMessage(std::move(message)),
          comments
      );
      break;
    case Type::CommentsRequest:
      send_comments(
          socket,
          *message::GetCommentsMessage::fromMessage(std::move(message)),
          comments
      );
      break;
    case Type::Goodbye:
      conn_closed = true;
      break;
    case Type::Hello:
    case Type::CommentOk:
    case Type::CommentsResponse:
    default:
      assert(0 && "Unexpected message");
      break;
    }
  }
  close(socket);
}

static void add_comment(
    int socket,
    message::NewCommentMessage message,
    std::vector<std::string> &comments
) {
  auto raw_comment = message.getComment();
  std::string comment( raw_comment.begin(), raw_comment.end());
  comments.emplace_back(std::move(comment));

  auto response = message::Message::commentOk();
  auto bytes = response.getBytes();

  int sent = send(socket, bytes.data(), bytes.size(), 0);
  assert(sent > 0);
  assert((size_t) sent == bytes.size());
}

static void send_comments(
    int socket,
    message::GetCommentsMessage message,
    std::vector<std::string> &comments
) {
  size_t index = message.getStartIndex();
  size_t total = comments.size();
  size_t count = index < total ? total - index : 0;

  auto response = message::Message::sendComments(comments, index, count);
  auto bytes = response.getBytes();

  int sent = send(socket, bytes.data(), bytes.size(), 0);
  assert(sent > 0);
  assert((size_t) sent == bytes.size());
}

} // namespace server
