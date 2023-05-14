#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
int aus_socket_bind_inet(int socket_fd, int port) {
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  return bind(socket_fd, (struct sockaddr*)&address, sizeof(address));
}

int aus_socket_bind_addrinfo(int socket_fd, struct addrinfo* res) {
  // TODO: walk res
  return bind(socket_fd, res->ai_addr, res->ai_addrlen);
}

struct addrinfo* aus_socket_getaddrinfo() {
  struct addrinfo hints, *res;
  int sockfd;

  // first, load up address structs with getaddrinfo():

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  getaddrinfo(NULL, "3490", &hints, &res);

  return res;
}

int aus_socket_freeaddrinfo(struct addrinfo *res) {
  freeaddrinfo(res);
  return 0;
}
*/

int austral_sockets_bind(int family, int socktype, const char* addr, const char *port)
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int yes=1;
  int rv;
  const char *node = NULL, *service = NULL;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_flags = AI_PASSIVE; // use my IP if not provided

  if (strlen(addr) > 0) {
    node = addr;
  }
  if (strlen(port) > 0) {
    service = port;
  }

  if ((rv = getaddrinfo(node, service, &hints, &servinfo)) != 0) {
    return -1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      close(sockfd);
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL)  {
    return -1;
  }

  return sockfd;
}

int austral_sockets_accept(int sockfd)
{
  return accept(sockfd, NULL, NULL);
}

int austral_sockets_listen(int sockfd, int backlog)
{
  return listen(sockfd, backlog);
}

int austral_sockets_close(int sockfd)
{
  return close(sockfd);
}

int austral_sockets_send_all(int sockfd, const char *buf)
{
  int len = strlen(buf);
  int total = 0; // how many bytes we've sent
  int bytesleft = len; // how many we have left to send
  int n;

  while(total < len) {
    n = send(sockfd, buf+total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  return n == -1 ? -1 : total; // return -1 on failure, bytes sent on success
}
