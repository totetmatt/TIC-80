
#include "network.h"

#include <pthread.h>

struct mg_mgr mgr; 
struct mg_connection *c;
char ws_url[1024]="ws://drone.alkama.com:9000/test/tic";
bool tic80_network_done = false;    
pthread_t pthread_poll =NULL;

bool tic80_network_activated = false;
enum NETWORK_MODE tic80_network_mode = SENDER;

pthread_mutex_t network_message_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
char buf[65536];
struct mg_str latest_ws_message; 
// Print websocket response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_OPEN) {
        
      // c->is_hexdumping = 1;
    } /*else if (ev == MG_EV_CONNECT && mg_url_is_ssl(s_url)) {
      struct mg_str ca = mg_file_read(&mg_fs_posix, s_ca_path);
      struct mg_tls_opts opts = {.ca = ca, .name = mg_url_host(s_url)};
      mg_tls_init(c, &opts);
    }*/ else if (ev == MG_EV_ERROR) {
      // On error, log error message
      //MG_ERROR(("%p %s", c->fd, (char *) ev_data));
      printf("MG_EV_ERROR\n");
    } else if (ev == MG_EV_WS_OPEN) {
        printf("Connected to %s\n",ws_url);
      // When websocket handshake is successful, send message
      // mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
    } else if (ev == MG_EV_WS_MSG && tic80_network_mode == GRABBER)  {
      // When we get echo response, print it
      pthread_mutex_lock(&network_message_buffer_mutex);
      latest_ws_message = mg_strdup(((struct mg_ws_message *) ev_data)->data);
      //printf("GOT ECHO REPLY: [%.*s]\n", (int) latest_ws_message.len, latest_ws_message.buf);
      pthread_mutex_unlock(&network_message_buffer_mutex);

    }
    if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE ) { // || ev == MG_EV_WS_MSG
        printf("CLOSE\n");
        //tic80_network_activated = false;
      *(bool *) c->fn_data = true;  // Signal that we're done
    }
  }
  void *websocket_poll(void *arg) {
    while(true && !tic80_network_done) {
       
        mg_mgr_poll(&mgr, 100);
    }
    mg_mgr_free(&mgr);
    //initWebsocket();
	pthread_exit(EXIT_SUCCESS);
}
static void ws_connect() {

  if(c == NULL) {
    printf("Connecting...\n");
    c = mg_ws_connect(&mgr, ws_url, fn, &tic80_network_done, NULL);
  }
}
void init_websocket() {
    mg_mgr_init(&mgr); 
    //mg_log_set(MG_LL_DEBUG);
    
    mg_timer_add(&mgr, 3000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, ws_connect, NULL);
 

    pthread_create(&pthread_poll, NULL, websocket_poll, NULL);
}
void free_websocket(){
    mg_mgr_free(&mgr);  
}
void set_network_config(char* networkurl, int activate, char* mode) {
    // Network activated
    strcpy(ws_url,networkurl);
    tic80_network_activated = activate;
    if(tic80_network_activated) {
      printf("Network is activated\n");
    } else {
      printf("Network is not activated\n");
    }
    // Networkmode
    if(strcmp("GRABBER",mode)==0) {
      tic80_network_mode = GRABBER;
      printf("Networkmode set to GRABBER\n");
    } else if (strcmp("SENDER",mode)==0) {
      tic80_network_mode = SENDER;
      printf("Networkmode set to SENDER\n");
    } else {
      printf("No valid mode, fallback to SENDER\n");
      tic80_network_mode = SENDER;
    }
    
}
void network_send(char * src,int x,int y){
 
   //mg_ws_printf ? https://github.com/cesanta/mongoose/blob/master/tutorials/websocket/json-rpc-over-websocket/main.c#L61-L63
    
    if(tic80_network_mode==SENDER){
      mg_snprintf(buf, 65536, "{%m:%m,%m:{%m:[%d,%d],%m:%m}}", 
        MG_ESC("s"),MG_ESC("tic80"),
        MG_ESC("data"),
          MG_ESC("pos"),x,y,
          MG_ESC("code"),MG_ESC(src)
      );
      mg_ws_send(c, buf, strlen(buf), WEBSOCKET_OP_TEXT);
      return;
    }
}
void network_get(char* code,int *x,int *y) {
    pthread_mutex_lock(&network_message_buffer_mutex);
    if(latest_ws_message.len>0){
      strcpy(code,mg_json_get_str(latest_ws_message,"$.data.code"));
    } 
    //printf("GOT ECHO REPLY: [%.*s]\n", (int) latest_ws_message.len, latest_ws_message.buf);
    *x = (int)(mg_json_get_long(latest_ws_message,"$.data.pos[0]",-1));
    *y = (int)(mg_json_get_long(latest_ws_message,"$.data.pos[1]",-1));
    //printf("%i\n",*x);
    
    pthread_mutex_unlock(&network_message_buffer_mutex);
}
bool is_tic80_network_activated(){
  return tic80_network_activated;
}
bool is_network_sender_mode(){
  return tic80_network_mode == SENDER;
}
bool is_network_grabber_mode(){
  return tic80_network_mode == GRABBER;
}