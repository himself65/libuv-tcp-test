#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    } else if (nread > 0) {
        uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
        uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
        uv_write(req, client, &wrbuf, 1, echo_write);
    }

    if (buf->base)
        free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void on_error(uv_stream_t *server, int status) {
    if (status) {
        printf("An error occurred, yay!\n");
        printf("Error: %s\n", uv_strerror(status));
        exit(0);
    }
}

void on_close(uv_handle_t *handle) {
    free(handle);
}

void try_new_port() {
    printf("Creating new server...\n");
    
    uv_tcp_t *server = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, server);
    
    // Bind to port 0 to get a random port
    uv_ip4_addr("0.0.0.0", 0, &addr);
    if (uv_tcp_bind(server, (const struct sockaddr*)&addr, 0) != 0) {
        printf("Bind failed\n");
        on_error((uv_stream_t*)server, -1);
        return;
    }
    
    int r = uv_listen((uv_stream_t*)server, DEFAULT_BACKLOG, NULL);
    if (r) {
        printf("Listen failed\n");
        on_error((uv_stream_t*)server, r);
        return;
    }
    
    // Get the port number
    struct sockaddr_in name;
    int namelen = sizeof(name);
    int err = uv_tcp_getsockname(server, (struct sockaddr*)&name, &namelen);
    if (err == 0) {
        int port = ntohs(name.sin_port);
        printf("port %d\n", port);
    } else {
        printf("Failed to get socket name\n");
        printf("Error: %s\n", uv_err_name(err));
    }
    
    // Close the current server and try again
    uv_close((uv_handle_t*)server, on_close);
    
    // Schedule the next attempt
    uv_timer_t *timer = (uv_timer_t*)malloc(sizeof(uv_timer_t));
    uv_timer_init(loop, timer);
    uv_timer_start(timer, (uv_timer_cb)try_new_port, 0, 0);
}

int main() {
    loop = uv_default_loop();
    try_new_port();
    return uv_run(loop, UV_RUN_DEFAULT);
}
