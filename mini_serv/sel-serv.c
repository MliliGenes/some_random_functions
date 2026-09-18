#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

// exam boilerplate 

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
	int		len;

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

// sel logic

size_t max_fds = 0, count = 0;

int listen_fd;
fd_set read_set, write_set, origin_set;

int clients[50000];
char *clients_messages[50000];

char write_buff[4096], read_buff[4096];

void fatal_error(char *msg)
{
    write(2, msg, strlen(msg));
    write(2, "\n", 1);
    exit(1);
}

void init_socket()
{
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        fatal_error("Fatal error");
    return;
}

void bind_socket(int port)
{
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(struct sockaddr_in));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
    servaddr.sin_port = htons(port);
    if (bind(listen_fd, (const struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) != 0)
        fatal_error("Fatal error");
    return;
}

void listen_socket()
{
    if (listen(listen_fd, 120) != 0)
        fatal_error("Fatal error");
}

void __notify(int author, char *str)
{
    for (int fd = 0; fd <= max_fds; fd++)
    {
        if (fd == author)
            continue;
        if (FD_ISSET(fd, &write_set))
            send(fd, str, strlen(str), 0);
    }
}

void add_client(int client_fd)
{
    FD_SET(client_fd, &origin_set);
    clients[client_fd] = count;
    count++;
    if (client_fd + 1 > max_fds)
        max_fds = client_fd + 1;
}

void accept_client()
{
    int client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd == -1)
        return;
    add_client(client_fd);
    sprintf(write_buff, "server: client %d just arrived\n", clients[client_fd]);
    __notify(client_fd, write_buff);
}

void remove_client(int fd) {
    close(fd);
    FD_CLR(fd, &origin_set);
    free(clients_messages[fd]);
    clients_messages[fd] = NULL;
    sprintf(write_buff, "server: client %d just left\n", clients[fd]);
    __notify(fd, write_buff);
}

void read_message(int fd)
{
    int n = recv(fd, read_buff, 4096 - 1, 0);

    if (n <= 0)
    {
        remove_client(fd);
        return;
    }
    read_buff[n] = '\0';
    clients_messages[fd] = str_join(clients_messages[fd], read_buff);

    char *full_message = NULL;
    while (extract_message(&clients_messages[fd], &full_message)) {
        sprintf(write_buff, "client %d: ", clients[fd]);
        __notify(fd, write_buff);
        __notify(fd, full_message);
        free(full_message);
    }
}

void start_server()
{
    FD_ZERO(&origin_set);
    FD_SET(listen_fd, &origin_set);
    max_fds = listen_fd + 1;

    while (1)
    {
        read_set = write_set = origin_set;

        if (select(max_fds, &read_set, &write_set, NULL, NULL) == -1)
            fatal_error("Error");

        for (int fd = 0; fd <= max_fds; fd++)
        {
            if (!FD_ISSET(fd, &read_set))
                continue;

            if (fd == listen_fd)
            {
                accept_client();
                break;
            }
            read_message(fd);
        }
    }
}

int main(int ac, char **av)
{
    if (ac != 2)
        fatal_error("Wrong number of arguments");

    init_socket();
    bind_socket(atoi(av[1]));
    listen_socket();

    start_server();

    return 0;
}