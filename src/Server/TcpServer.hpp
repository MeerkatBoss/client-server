/**
 * @file TcpServer.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-03
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __SERVER_TCP_SERVER_HPP
#define __SERVER_TCP_SERVER_HPP

#include <cstdint>
namespace server {

void listen_tcp(uint8_t ip_address[4], uint16_t port);

} // namespace server

#endif /* TcpServer.hpp */
