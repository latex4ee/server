#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

#include "server.h"

int server_init(uint16_t port)
{
	const int MAX_CLIENTS = 30;
	int sock_master, sock_new, sock_client[MAX_CLIENTS] = {};
	struct sockaddr_in address;
	int opt = 1;
	int addrlen, activity, i, read_val, sd, max_sd;
	fd_set readfds;
	char buff[1024] = {};
	char *msg = "HTTP/1.1 200 OK\r\nHost:latex4eng.tk\r\n\r\n<html><style>.inprogress {	color: black;}.complete {	color: blue;}.full-source {	color: green;}</style>	<title>LaTeX for Engineers</title>	<body>		<div>		<ul>			<li>				Circuit Theory				<ul>					<li><a href='#'>Circuit Theory report template</a></li>					<li><a href='#'>Circuit Theory Homework One</a></li>					<li><a href='#'>Circuit Theory Homework Two</a></li>		</div>		<div>			<ul>				<li class='inprogress'>In Progress</li>				<li class='complete'>Complete</li>				<li class='full-source'>Full source online</li>			</ul>		</div>	</body></html>";

	// Create IP TCP socket
	if ((sock_master = socket(AF_INET, SOCK_STREAM, 0))==0)
	{
		syslog(LOG_ERR, "%s:%d|socket failed", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	// Attach socket to 8080
	if (setsockopt(sock_master, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))<0)
	{
		syslog(LOG_ERR, "%s:%d|setsockopt failed", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock_master, (struct sockaddr *)&address, sizeof(address))<0)
	{
		syslog(LOG_ERR, "%s:%d|bind failed", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	if (listen(sock_master, MAX_PENDING_CONNECTIONS) <0)
	{
		syslog(LOG_ERR, "%s:%d|listen failed", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	addrlen=sizeof(address);
	while(1)
	{
		// clear socket set
		FD_ZERO(&readfds);
		// add master to set
		FD_SET(sock_master, &readfds);
		max_sd = sock_master;
		// add child sockets to set
		for (i=0; i<MAX_CLIENTS;i++)
		{
			sd = sock_client[i];
			if(sd > 0)
				FD_SET(sd, &readfds);
			if(sd>max_sd)
				max_sd = sd;
		}
		// wait indefinitely for activity on a socket
		activity = select( max_sd +1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno!=EINTR))
		{
			syslog(LOG_ERR, "%s:%d|select error", __FILE__, __LINE__);
		}
		// if activity on master socket, incoming connection
		if (FD_ISSET(sock_master, &readfds))
		{
			if ((sock_new = accept(sock_master, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				syslog(LOG_ERR, "%s:%d|accept error", __FILE__, __LINE__);
				exit(EXIT_FAILURE);
			}
			syslog(LOG_NOTICE, "New connection from [%d %s:%d]\n",
					sock_new, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			if (send(sock_new, msg, strlen(msg), 0) != strlen(msg))
			{
				syslog(LOG_WARN, "%s:%d|send error", __FILE__, __LINE__);
			}

			printf("Response sent\n");
			for (i=0;i<MAX_CLIENTS;i++)
			{
				if (sock_client[i] == 0 )
				{
					sock_client[i] = sock_new;
					printf("Adding to list of sockets as number %d\n", i);
					break;
				}
			}
		}
		// otherwise it's some IO on another socket
		for (i=0;i<MAX_CLIENTS;i++)
		{
			sd = sock_client[i];

			if(FD_ISSET(sd,&readfds))
			{
				if((read_val = read(sd,buff,1024 - 1))==0)
				{
					// Someone disconnected
					getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					syslog(LOG_NOTICE, "Host %s:%d disconnected.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					// Close socket and mark for reuse
					close(sd);
					sock_client[i] = 0;
				}
				else
				{
					// Echo msg that arrived
					send(sd, msg, strlen(msg), 0);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
