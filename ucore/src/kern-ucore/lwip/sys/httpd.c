#include <assert.h>
#include <unistd.h>
#include <stat.h>
#include <lwip/sockets.h>

#define PORT 80
#define VERSION "0.1"
#define HTTP_VERSION "1.0"

#define E_BAD_REQ	1000

#define BUFFSIZE 512
#define MAXPENDING 5	// Max connection requests

struct http_request {
	int sock;
	char *url;
	char *version;
};

struct responce_header {
	int code;
	char *header;
};

struct responce_header headers[] = {
	{ 200, 	"HTTP/" HTTP_VERSION " 200 OK\r\n"
		"Server: jhttpd/" VERSION "\r\n"},
	{0, 0},
};

struct error_messages {
	int code;
	char *msg;
};

struct error_messages errors[] = {
	{400, "Bad Request"},
	{404, "Not Found"},
};

static void
die(char *m)
{
    panic(m);
}

static void
req_free(struct http_request *req)
{
	kfree(req->url);
	kfree(req->version);
}

static int
send_header(struct http_request *req, int code)
{
	struct responce_header *h = headers;
	while (h->code != 0 && h->header!= 0) {
		if (h->code == code)
			break;
		h++;
	}

	if (h->code == 0)
		return -1;

	int len = strlen(h->header);
	if (lwip_send(req->sock, h->header, len, 0) != len) {
		die("Failed to send bytes to client");
	}

	return 0;
}

int kernel_read(int fd, void *base, size_t len)
{
	int ret = 0;
	if (len == 0) {
		return 0;
	}
	if (!file_testfd(fd, 1, 0)) {
		return -1;
	}
	void *buffer = base;

	size_t alen;
	while (len != 0) {
		if ((alen = 4096) > len) {
			alen = len;
		}
		ret = file_read(fd, buffer, alen, &alen);
		if (alen != 0) {
            assert(len >= alen);
            base += alen, len -= alen;
		}
		if (ret != 0 || alen == 0) {
            return -1;
		}
	}

    return base - buffer;
}

static int
send_data(struct http_request *req, int fd, int size)
{
	// LAB 6: Your code here.
	// File size is limited by BUFFSIZE
	char buf[BUFFSIZE];
	int n;
    while(size > 0) {
        if (file_read(fd, &buf, BUFFSIZE, &n) != 0)
            panic("send_data: Failed to read file");

        /* Debugging */

        if (lwip_send(req->sock, &buf, n, 0) != n) {
            die("Failed to send bytes to client");
        }
        size -= n;
    }

	return 0;
}

static int
send_size(struct http_request *req, long size)
{
	char buf[64];
	int r;

	r = snprintf(buf, 64, "Content-Length: %ld\r\n", (long)size);
	if (r > 63)
		panic("buffer too small!");

	if (lwip_send(req->sock, buf, r, 0) != r)
		return -1;

	return 0;
}

static const char*
mime_type(const char *file)
{
	//TODO: for now only a single mime type
	return "text/html";
}

static int
send_content_type(struct http_request *req)
{
	char buf[128];
	int r;
	const char *type;

	type = mime_type(req->url);
	if (!type)
		return -1;

	r = snprintf(buf, 128, "Content-Type: %s\r\n", type);
	if (r > 127)
		panic("buffer too small!");

	if (lwip_send(req->sock, buf, r, 0) != r)
		return -1;

	return 0;
}

static int
send_header_fin(struct http_request *req)
{
	const char *fin = "\r\n";
	int fin_len = strlen(fin);

	if (lwip_send(req->sock, fin, fin_len, 0) != fin_len)
		return -1;

	return 0;
}

// given a request, this function creates a struct http_request
static int
http_request_parse(struct http_request *req, char *request)
{
	const char *url;
	const char *version;
	int url_len, version_len;

	if (!req)
		return -1;

	if (strncmp(request, "GET ", 4) != 0)
		return -E_BAD_REQ;

	// skip GET
	request += 4;

	// get the url
	url = request;
	while (*request && *request != ' ')
		request++;
	url_len = request - url;

	// if url is / , append "index.html" to end
	if (*url == '/')
		{
			req->url = kmalloc(11);
			char * index = "index.html";
			memmove(req->url, index, 11);
		}
	else
		{
			req->url = kmalloc(url_len + 1);
			memmove(req->url, url, url_len);
			req->url[url_len] = '\0';
		}

	// skip space
	request++;

	version = request;
	while (*request && *request != '\n')
		request++;
	version_len = request - version;

	req->version = kmalloc(version_len + 1);
	memmove(req->version, version, version_len);
	req->version[version_len] = '\0';

	// no entity parsing

	return 0;
}

static int
send_error(struct http_request *req, int code)
{
	char buf[512];
	int r;

	struct error_messages *e = errors;
	while (e->code != 0 && e->msg != 0) {
		if (e->code == code)
			break;
		e++;
	}

	if (e->code == 0)
		return -1;

	r = snprintf(buf, 512, "HTTP/" HTTP_VERSION" %d %s\r\n"
			       "Server: jhttpd/" VERSION "\r\n"
			       "Connection: close"
			       "Content-type: text/html\r\n"
			       "\r\n"
			       "<html><body><p>%d - %s</p></body></html>\r\n",
			       e->code, e->msg, e->code, e->msg);

	if (lwip_send(req->sock, buf, r, 0) != r)
		return -1;

	return 0;
}

static int
send_file(struct http_request *req)
{
	int r;
	long file_size = -1;
	int fd;

	// open the requested url for reading
	// if the file does not exist, send a 404 error using send_error
	// if the file is a directory, send a 404 error using send_error
	// set file_size to the size of the file

	// LAB 6: Your code here.
	/* Debugging */
	// cprintf("send_file: url = %s\n", req->url);

	// Open the requested url for reading
	fd = -1;
    kprintf("Try open file %s\n", req->url);
	if ((r = file_open(req->url, O_RDONLY)) < 0) {
		send_error(req, 404);
		goto end;
	} else {
		fd = r;
	}

	// Get file status
	struct stat stat;
	file_fstat(fd, &stat);

	/* Debugging */
	// cprintf("send_file: file status: %s, %d, %d\n",
	//          stat.st_name, stat.st_size, stat.st_isdir);

	// Check if it is a directory
/*	if (stat.st_isdir) {
		send_error(req, 404);
		goto end;
	}*/

	// Set file_size
	file_size = stat.st_size;

	// Send the file
	if ((r = send_header(req, 200)) < 0)
		goto end;

	if ((r = send_size(req, file_size)) < 0)
		goto end;

	if ((r = send_content_type(req)) < 0)
		goto end;

	if ((r = send_header_fin(req)) < 0)
		goto end;

	r = send_data(req, fd, file_size);

end:
	file_close(fd);
	return r;
}

static void
handle_client(int sock)
{
	struct http_request con_d;
	int r;
	char buffer[BUFFSIZE];
	int received = -1;
	struct http_request *req = &con_d;

	while (1)
	{
		// Receive message
		if ((received = lwip_recv(sock, buffer, BUFFSIZE, 0)) < 0)
			panic("failed to read");

		memset(req, 0, sizeof(req));

		req->sock = sock;

		r = http_request_parse(req, buffer);
		if (r == -E_BAD_REQ)
			send_error(req, 400);
		else if (r < 0)
			panic("parse failed");
		else
			send_file(req);

		req_free(req);

		// no keep alive
		break;
	}

	lwip_close(sock);
}

void
httpd_main(int argc, char **argv)
{
	int serversock, clientsock;
	struct sockaddr_in server, client;

	// Create the TCP socket
	if ((serversock = lwip_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die("Failed to create socket");
    kprintf("serversock %d\n", serversock);

	// Construct the server sockaddr_in structure
	memset(&server, 0, sizeof(server));		// Clear struct
	server.sin_family = AF_INET;			// Internet/IP
	server.sin_addr.s_addr = htonl(INADDR_ANY);	// IP address
	server.sin_port = htons(PORT);			// server port

	// Bind the server socket
	if (lwip_bind(serversock, (struct sockaddr *) &server,
		 sizeof(server)) < 0)
	{
		die("Failed to bind the server socket");
	}

	// Listen on the server socket
	if (lwip_listen(serversock, MAXPENDING) < 0)
		die("Failed to listen on server socket");

	cprintf("Waiting for http connections...\n");

	while (1) {
		unsigned int clientlen = sizeof(client);
		// Wait for client connection
        kprintf("waiting for client\n");
		if ((clientsock = lwip_accept(serversock,
					 (struct sockaddr *) &client,
					 &clientlen)) < 0)
		{
			die("Failed to accept client connection");
		}
		handle_client(clientsock);
	}

	lwip_close(serversock);
}
