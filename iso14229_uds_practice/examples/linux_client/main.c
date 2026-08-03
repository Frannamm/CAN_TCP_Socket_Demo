#include "iso14229.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PHYS_ADDR   0x7E0
#define CLIENT_PHYS_ADDR   0x7E8
#define FUNCTIONAL_ADDR    0x7DF

#define DID_TEST_SCRATCH   0xDEAD
#define DID_VIN            0xF190

static UDSClient_t client;
static UDSTpIsoTpSock_t tp;

typedef enum {
    STATE_INIT,
    STATE_AWAIT_SESSION,
    STATE_AWAIT_RDBI,
    STATE_DONE,
    STATE_ERROR
} AppState_t;

static AppState_t app_state = STATE_INIT;

static int fn(UDSClient_t *client, UDSEvent_t ev, void *arg) {
    switch (ev) {
        case UDS_EVT_ResponseReceived:
            printf("Response received (%u bytes): ", client->recv_size);
            for (int i = 0; i < client->recv_size; i++) {
                printf("%02X ", client->recv_buf[i]);
            }
            printf("\n");

            if (app_state == STATE_AWAIT_SESSION) {
                printf("Session control confirmed. Sending RDBI request...\n");
                uint16_t did = DID_TEST_SCRATCH;
                UDSSendRDBI(client, &did, 1);
                app_state = STATE_AWAIT_RDBI;
            } else if (app_state == STATE_AWAIT_RDBI) {
                printf("RDBI complete.\n");
                app_state = STATE_DONE;
            }
            break;

        case UDS_EVT_Err: {
            UDSErr_t *err = (UDSErr_t *)arg;
            printf("Error: %s\n", UDSErrToStr(*err));
            app_state = STATE_ERROR;
            break;
        }

        default:
            break;
    }
    return 0;
}

int main(int ac, char **av) {
    if (ac != 2) {
        fprintf(stderr, "usage: %s <can interface>\n", av[0]);
        exit(-1);
    }

    if (UDSTpIsoTpSockInitClient(&tp, av[1], CLIENT_PHYS_ADDR, SERVER_PHYS_ADDR, FUNCTIONAL_ADDR)) {
        fprintf(stderr, "UDSTpIsoTpSockInitClient failed\n");
        exit(-1);
    }

    if (UDSClientInit(&client)) {
        fprintf(stderr, "UDSClientInit failed\n");
        exit(-1);
    }

    client.tp = (UDSTp_t *)&tp;
    client.fn = fn;

    printf("Sending Diagnostic Session Control (extended session)...\n");
    UDSSendDiagSessCtrl(&client, UDS_LEV_DS_EXTDS);
    app_state = STATE_AWAIT_SESSION;

    while (app_state != STATE_DONE && app_state != STATE_ERROR) {
        UDSClientPoll(&client);
    }

    printf("Client exiting\n");
    return 0;
}