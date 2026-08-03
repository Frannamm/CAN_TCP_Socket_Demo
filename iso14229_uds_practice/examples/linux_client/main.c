#include "iso14229.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PHYS_ADDR   0x7E0
#define CLIENT_PHYS_ADDR   0x7E8
#define FUNCTIONAL_ADDR    0x7DF

#define DID_TEST_SCRATCH   0xDEAD
#define DID_VIN            0xF190

#define MAX_RETRIES 3

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
static int retries_left = MAX_RETRIES;

static void send_session_ctrl(UDSClient_t *client) {
    printf("Sending Diagnostic Session Control (extended session)...\n");
    UDSSendDiagSessCtrl(client, UDS_LEV_DS_EXTDS);
    app_state = STATE_AWAIT_SESSION;
    retries_left = MAX_RETRIES;
}

static void send_rdbi(UDSClient_t *client) {
    printf("Sending RDBI request...\n");
    uint16_t did = DID_TEST_SCRATCH;
    UDSSendRDBI(client, &did, 1);
    app_state = STATE_AWAIT_RDBI;
    retries_left = MAX_RETRIES;
}

static int fn(UDSClient_t *client, UDSEvent_t ev, void *arg) {
    switch (ev) {
        case UDS_EVT_ResponseReceived:
            printf("Response received (%u bytes): ", client->recv_size);
            for (int i = 0; i < client->recv_size; i++) {
                printf("%02X ", client->recv_buf[i]);
            }
            printf("\n");

            if (app_state == STATE_AWAIT_SESSION) {
                printf("Session control confirmed.\n");
                send_rdbi(client);
            } else if (app_state == STATE_AWAIT_RDBI) {
                printf("RDBI complete.\n");
                app_state = STATE_DONE;
            }
            break;

        case UDS_EVT_Err: {
            UDSErr_t *err = (UDSErr_t *)arg;
            printf("Error: %s\n", UDSErrToStr(*err));

            if (*err == UDS_ERR_TIMEOUT && retries_left > 0) {
                retries_left--;
                printf("Timeout - retrying (%d retries left)...\n", retries_left);
                if (app_state == STATE_AWAIT_SESSION) {
                    UDSSendDiagSessCtrl(client, UDS_LEV_DS_EXTDS);
                } else if (app_state == STATE_AWAIT_RDBI) {
                    uint16_t did = DID_TEST_SCRATCH;
                    UDSSendRDBI(client, &did, 1);
                }
            } else {
                printf("Giving up.\n");
                app_state = STATE_ERROR;
            }
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

    send_session_ctrl(&client);

    while (app_state != STATE_DONE && app_state != STATE_ERROR) {
        UDSClientPoll(&client);
    }

    printf("Client exiting\n");
    return app_state == STATE_ERROR ? -1 : 0;
}