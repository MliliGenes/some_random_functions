#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>


// -- global state tracking
int count = 0, max_fd = 0; //client ID counter, highest active FD
int ids[65536]; // array mapping fd -> unique client ID
char *msgs[65536]; // array mapping fd -> partial message sttr buffer. accumulates fragmented TCP stream.

// -- select() BITMASKS & BUFFERS
fd_set rfds; // temp bitmask passed to select() to -detect incoming data/connections.
fd_set wfds; // temp bitmask passed to select() to -detect non-blocking writable sockets.
fd_set afds; // All set (master set) : persistent bitmask retaining all active FDs.
char buf_r[1001]; // read buffer: stores up to 1000 raw bytes from recv() + 1 for ('\0')
char buf_w[100]; // write buffer: scratchpad for sprintf() to format prefixes and annouce messages

// -- error handler
void fatal_error() {
    write(2, "Fatal error\n", 12);
    exit(1); // <- subject required !!
}

// -- broadcast message to other clients
void notify_other(int author, char *str) {
    for (int fd = 0; fd <= max_fd; fd++) {
        // is fd marked writable in wfds && is fd not the sender
        if (FD_ISSET(fd, &wfds) && fd != author)
            send(fd, str, strlen(str), 0); // sends the raw bute string to socket fd
    }
}

// -- register new connected client
void register_client(int fd) {
    max_fd = fd > max_fd ? fd : max_fd; // update if new fd is bbigger than max_fd
    ids[fd] = count++; // assign current count as client ID, then increment count
    msgs[fd] = NULL; // ensure partial message accumulation pointer starts empty
    FD_SET(fd, &afds); // add new socket fd to master set so select() monitors it
    sprintf(buf_w, "server: client %d just arrived\n", ids[fd]); // format arrival announcement
    notify_other(fd, buf_w); // broadcast to all existing clients excluding author
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int	len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int main(int ac, char** av) {
    // 1- validate argument's count
    if (ac != 2) {
        write(2, "Wrong number of arguments\n", 26);
        return 1;
    }

    // 2- create the master listening tcp socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fatal_error();

    // 3- configure IPv4 socket address structure
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr)); // Zero out the entire struct to clear garbage
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1 in 32-bit big-endian int
    servaddr.sin_port = htons(atoi(av[1])); // convert port string to int, then to 16-but network byte order

    // 4- bind socket to address and port
    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
        fatal_error();

    // 5- place socket in passive listening mode
    if (listen(sockfd, 128) != 0)
        fatal_error();

    // 6- init master fd_set tracking
    FD_ZERO(&afds); // clear all bits in afds
    FD_SET(sockfd, &afds); // set the bit for sockfd in master set
    max_fd = sockfd;

    // -- core event multiplexing loop
    while (1) {
        rfds = wfds = afds; // clone master set into working sets before each select() call
        // select(nfds, readfds, writefds, exceptfds, timeout)
        // blocks until at least one monitored socket becomes ready to read or write
        if (select(max_fd + 1, &rfds, &wfds, NULL, NULL) < 0)
            fatal_error();

        // iterate through all possible file descriptores up to max_fd
        for (int fd = 0; fd <= max_fd; fd++) {
            // check if this specific fd has read activity
            if (!FD_ISSET(fd, &rfds))
                continue; // not ready for reading, skit to next fd

            // CASE A: activity in the listening socket -> A NEW CLIENT IS CONNECTING
            if (fd == sockfd) {
                int connfd = accept(sockfd, NULL, NULL); // accept the pending connection, get new client fd
                if (connfd < 0)
                    fatal_error();
                register_client(connfd); // assign ID, add to afds, announce arrival
            }
            // CASE B: activity on a client socket -> INCOMING DATA OR DISCONNECTION
            else {
                int n = recv(fd, buf_r, 1000, 0); // read up to 1000 bytes into buf_r
                if (n <= 0) {
                    // SUBCASE B1: disconnection (n == 0: clean EOF, n < 0: read error)
                    sprintf(buf_w, "server: client %d just left\n", ids[fd]);
                    notify_other(fd, buf_w); // 1. announce deparature to others
                    FD_CLR(fd, &afds); // 2. remove fd from master set
                    close(fd); // 3. close the socket (to prevent fd leak)
                    free(msgs[fd]); // 4. free partial message buffer (prevent mem leak)
                    msgs[fd] = NULL; // 5. clear pointer (to prevent dangling ref)
                }
                else {
                    // SUBCASE B2: Message data receoved (n > 0 bytes)
                    buf_r[n] = 0; // null-terminate the reveived chunk
                    msgs[fd] = str_join(msgs[fd], buf_r); // append chunk to client accumulated stream
                    char *msg;
                    //extract and broadcast every complete \n-terminated line
                    if (extract_message(&msgs[fd], &msg) == 1) {
                        sprintf(buf_w, "client %d: ", ids[fd]); // format prefix for this line
                        notify_other(fd, buf_w); // send prefix "client %d: "
                        notify_other(fd, msg); // send line contents (already contains \n)
                        free(msg); // free line allocated by extract_message()
                    }
                }
            }
        }
    }
    return 0;
}