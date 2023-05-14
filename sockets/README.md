# Sockets

Sockets are interesting because their lifetime is a little bit complicated.
There are several systemcalls that must proceed in a given order.
Check out the diagrams on [Wikipedia](https://en.wikipedia.org/wiki/Berkeley_sockets).

## Approach

However, when attempting to implement the full API with all its stateful complexity, I realised that it was tricky because of all the quite detailed C structs that are used.
For example, `bind`ing a socket:

```c
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
```
From [Beej's guide to network programming](https://beej.us/guide/bgnet/html/#a-simple-stream-server).

There's an interesting flow in that loop, but wrangling the structs and intrusive linked list is a little beyond my expertise and willingness to wrangle the FFI for now.

So, for this example I decided to create some "nice" C helper functions which are easy to call over the FFI (see [austral_sockets.c](./austral_sockets.c)).
The Socket module provides a restricted interface to cover a few typical socket patterns.
