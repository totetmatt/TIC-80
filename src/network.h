#pragma once
#include "ext/mongoose.h"

enum NETWORK_MODE {
    GRABBER,
    SENDER
};


void init_websocket();
void free_websocket();
void set_network_config(char* networkurl,int activate, char* mode);
void network_send(char * src,int x,int y);
void network_get(char* code,int *x,int *y);
bool is_tic80_network_activated();
bool is_network_sender_mode();
bool is_network_grabber_mode();