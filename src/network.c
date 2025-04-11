
#include "network.h"
#include "ext/mongoose.h"
#include <pthread.h>

struct mg_mgr mgr; 
struct mg_connection *c;
bool tic80_network_done = false;    
pthread_t pthread_poll =NULL;
void *websocket_poll(void *arg) {
    while(true) {
        mg_mgr_poll(&mgr, 1000);
    }
	pthread_exit(EXIT_SUCCESS);
}
// Print websocket response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_OPEN) {
        printf("MG_EV_OPEN\n");
      c->is_hexdumping = 1;
    } /*else if (ev == MG_EV_CONNECT && mg_url_is_ssl(s_url)) {
      struct mg_str ca = mg_file_read(&mg_fs_posix, s_ca_path);
      struct mg_tls_opts opts = {.ca = ca, .name = mg_url_host(s_url)};
      mg_tls_init(c, &opts);
    }*/ else if (ev == MG_EV_ERROR) {
      // On error, log error message
      MG_ERROR(("%p %s", c->fd, (char *) ev_data));
      printf("MG_EV_ERROR\n");
    } else if (ev == MG_EV_WS_OPEN) {
        printf("MG_EV_WS_OPEN\n");
      // When websocket handshake is successful, send message
      // mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
    } else if (ev == MG_EV_WS_MSG) {
      // When we get echo response, print it
      printf("MESG");
      struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
      printf("GOT ECHO REPLY: [%.*s]\n", (int) wm->data.len, wm->data.buf);
    }
  
    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_MSG) {
        printf("CLOSE\n");
      *(bool *) c->fn_data = true;  // Signal that we're done
    }
  }

void initWebsocket() {
    mg_mgr_init(&mgr); 
    const char *s_url="ws://drone.alkama.com:9000/test/tic";
    c = mg_ws_connect(&mgr, s_url, fn, &tic80_network_done, NULL);
    pthread_create(&pthread_poll, NULL, websocket_poll, NULL);
}