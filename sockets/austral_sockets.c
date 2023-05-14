#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Most of this file was copied and adapted from Beej's guide to network programming:
// https://beej.us/guide/bgnet/html/

// TODO: use errno to provide more helpful error messages back to Austral code.

/**
 * Create a socket bound to a given address and port. If addr or port are empty strings,
 * they'll be treated as "NULL" when used with getaddrinfo.
 *
 * By doing the create and bind in a single function, we encapsulate the linked list result
 * from getaddrinfo. This reduces flexibility, unfortunately, because the caller does not
 * have access to setsocketopt() between the calls to socket() and bind(). Someday
 * I might improve this.
 *
 * Maybe an API based on using and returning opaque (struct addrinfo*) pointers would allow
 * Austral equivalents of socket(), setsocketopt(), bind() etc.
 */
int austral_sockets_easybind(int family, int socktype, const char* addr, const char *port)
{
  int sockfd;
  struct addrinfo hints, *servinfo, *curr;
  int yes=1;
  int rv;
  const char *node = NULL, *service = NULL;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_flags = AI_PASSIVE; // use my IP if not provided

  // Austral doesn't pass null pointers for strings; but we'll use empty strings
  // as sentinels. If a string is empty, it gets implicitly treated as NULL.
  // TODO: are empty strings meaningful values for these APIs?
  if (strlen(addr) > 0) {
    node = addr;
  }
  if (strlen(port) > 0) {
    service = port;
  }

  if ((rv = getaddrinfo(node, service, &hints, &servinfo)) != 0) {
    return -1;
  }

  // Loop through all the results and bind to the first we can
  for(curr = servinfo; curr != NULL; curr = curr->ai_next) {
    if ((sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) == -1) {
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      close(sockfd);
      continue;
    }

    if (bind(sockfd, curr->ai_addr, curr->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }

    break;
  }

  // All done with this structure now.
  freeaddrinfo(servinfo);

  if (curr == NULL)  {
    return -1;
  }

  return sockfd;
}

int austral_sockets_accept(int sockfd)
{
  // TODO: actually capture the client information.
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

/**
 * Keep trying to send a string until we get an error.
 */
int austral_sockets_send_all(int sockfd, const char *buf)
{
  int len = strlen(buf);
  int sent = 0; // how many bytes we've sent
  int remaining = len; // how many we have left to send
  int n;

  while(sent < len) {
    n = send(sockfd, buf+sent, remaining, 0);
    if (n == -1) {
      break;
    }
    sent += n;
    remaining -= n;
  }

  // Return -1 on failure, bytes sent on success
  return n == -1 ? -1 : sent;
  // TODO: it would be nice to also return bytes sent on failure. Beej's code does that
  // using an output parameter, but I'm not keen to try that with the FFI.
}
