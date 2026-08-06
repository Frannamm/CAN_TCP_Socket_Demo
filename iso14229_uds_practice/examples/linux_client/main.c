#include "iso14229.h"
#include "dids.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PHYS_ADDR   0x7E0
#define CLIENT_PHYS_ADDR   0x7E8
#define FUNCTIONAL_ADDR    0x7DF

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

typedef enum {
    ACTION_NONE,
    ACTION_SEND_SESSION,
    ACTION_SEND_RDBI
} PendingAction_t;

static AppState_t app_state = STATE_INIT;
static PendingAction_t pending_action = ACTION_NONE;
static int retries_left = MAX_RETRIES;
static uint16_t target_did = DID_TEST_SCRATCH;

static void run_transaction(uint16_t did) {
    target_did = did;
    app_state = STATE_INIT;
    retries_left = MAX_RETRIES;
    pending_action = ACTION_SEND_SESSION;

    while (app_state != STATE_DONE && app_state != STATE_ERROR) {
        UDSClientPoll(&client);

        if (pending_action == ACTION_SEND_SESSION) {
            UDSErr_t ret = UDSSendDiagSessCtrl(&client, UDS_LEV_DS_EXTDS);
            if (ret == UDS_OK) {
                printf("Sending Diagnostic Session Control (extended session)...\n");
                app_state = STATE_AWAIT_SESSION;
                pending_action = ACTION_NONE;
            } else if (ret != UDS_ERR_BUSY) {
                printf("Send failed: %s\n", UDSErrToStr(ret));
                app_state = STATE_ERROR;
                pending_action = ACTION_NONE;
            }
            // if BUSY: do nothing, retry next loop iteration

        } else if (pending_action == ACTION_SEND_RDBI) {
            UDSErr_t ret = UDSSendRDBI(&client, &target_did, 1);
            if (ret == UDS_OK) {
                printf("Sending RDBI request for DID 0x%04X...\n", target_did);
                app_state = STATE_AWAIT_RDBI;
                pending_action = ACTION_NONE;
            } else if (ret != UDS_ERR_BUSY) {
                printf("Send failed: %s\n", UDSErrToStr(ret));
                app_state = STATE_ERROR;
                pending_action = ACTION_NONE;
            }
        }
    }

    printf(app_state == STATE_ERROR ? "Transaction failed.\n\n" : "Transaction complete.\n\n");
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
                retries_left = MAX_RETRIES;
                pending_action = ACTION_SEND_RDBI;
            } else if (app_state == STATE_AWAIT_RDBI) {
                printf("RDBI complete.\n");
                app_state = STATE_DONE;
            }
            break;

        case UDS_EVT_Err: {
            UDSErr_t *err = (UDSErr_t *)arg;
            printf("Error: %s\n", UDSErrToStr(*err));

            switch (*err) {
                case UDS_ERR_TIMEOUT:
                    if (retries_left > 0) {
                        retries_left--;
                        printf("Timeout - retrying (%d retries left)...\n", retries_left);
                        if (app_state == STATE_AWAIT_SESSION) pending_action = ACTION_SEND_SESSION;
                        else if (app_state == STATE_AWAIT_RDBI) pending_action = ACTION_SEND_RDBI;
                    } else {
                        printf("Giving up after max retries.\n");
                        app_state = STATE_ERROR;
                    }
                    break;

                case UDS_NRC_RequestOutOfRange:
                    printf("DID not supported by this server.\n");
                    app_state = STATE_ERROR;
                    break;

                case UDS_NRC_SubFunctionNotSupported:
                    printf("Requested session type not supported.\n");
                    app_state = STATE_ERROR;
                    break;

                case UDS_NRC_IncorrectMessageLengthOrInvalidFormat:
                    printf("Request malformed (wrong length/format).\n");
                    app_state = STATE_ERROR;
                    break;

                default:
                    printf("Unhandled error/NRC, giving up.\n");
                    app_state = STATE_ERROR;
                    break;
            }
            break;
        }

        default:
            break;
    }

    return 0;
}

int main(int ac, char **av) {
    if (ac < 2) {
        fprintf(stderr, "usage: %s <can interface> [did_hex]\n", av[0]);
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

    if (ac >= 3) {
        // one-shot mode: DID given on command line
        run_transaction((uint16_t)strtol(av[2], NULL, 16));
        return app_state == STATE_ERROR ? -1 : 0;
    }

    // interactive mode: prompt for DIDs until empty line or "q"
    char line[32];
    while (1) {
        printf("Enter DID (hex, e.g. F190) or 'q' to quit: ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        if (line[0] == 'q' || line[0] == '\n') break;

        uint16_t did = (uint16_t)strtol(line, NULL, 16);
        run_transaction(did);
    }

    printf("Client exiting\n");
    return 0;
}