#include <cstring>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ur_modern_driver/log.h"
#include "ur_modern_driver/ur/server.h"

URServer::URServer(int port)
  : port_(port)
{
}

URServer::~URServer()
{
  TCPSocket::close();
}

void URServer::setOptions(int socket_fd)
{
  TCPSocket::setOptions(socket_fd);

  int flag = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
}

std::string URServer::getIP()
{
  sockaddr_in name;
  socklen_t len = sizeof(name);
  int res = ::getsockname(getSocketFD(), (sockaddr*)&name, &len);

  if(res < 0)
  {
    LOG_ERROR("Could not get local IP");
    return std::string();
  }

  char buf[128];
  inet_ntop(AF_INET, &name.sin_addr, buf, sizeof(buf));
  return std::string(buf);
}

bool URServer::bind()
{
  std::string empty;
  bool res = TCPSocket::setup(empty, port_);
  
  if(!res)
    return false;

  if(::listen(getSocketFD(), 1) < 0)
    return false;

  return true;
}

bool URServer::accept()
{
  if(TCPSocket::getState() != SocketState::Connected || client_.getSocketFD() > 0)
    return false;

  struct sockaddr addr; 
  socklen_t addr_len;
  int client_fd = ::accept(getSocketFD(), &addr, &addr_len);

  if(client_fd <= 0)
    return false;

  setOptions(client_fd);

  return client_.setSocketFD(client_fd);
}

void URServer::disconnectClient()
{
  if(client_.getState() != SocketState::Connected)
    return;
  
  client_.close();
}

bool URServer::write(const uint8_t* buf, size_t buf_len, size_t &written)
{
  return client_.write(buf, buf_len, written);
}