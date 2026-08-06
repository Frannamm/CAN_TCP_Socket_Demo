/**
 * @file examples/linux_server/main.c
 * @brief Basic Linux UDS server example using socketcan ISO-TP
 */
#include "iso14229.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "dids.h"

static UDSServer_t srv;
static UDSTpIsoTpSock_t tp;
static bool done = false;
static int sleep_ms(uint32_t tms);

void sigint_handler(int signum) {
    printf("SIGINT received\n");
    done = true;
}

static uint8_t stored_value[3] = {0};

static UDSErr_t fn(UDSServer_t *srv, UDSEvent_t ev, void *arg) {
    switch (ev) {

    case UDS_EVT_DiagSessCtrl: {
        UDSDiagSessCtrlArgs_t *d = (UDSDiagSessCtrlArgs_t *)arg;
        if (d->type == UDS_LEV_DS_EXTDS) {
            return UDS_PositiveResponse;
        } else {
            return UDS_NRC_SubFunctionNotSupported;
        }
    }

    case UDS_EVT_SessionTimeout: {
        printf("Session timed out, reverted to default session.\n");
        return UDS_PositiveResponse;
    }

    case UDS_EVT_ReadDataByIdent: {
        UDSRDBIArgs_t *r = (UDSRDBIArgs_t *)arg;

        if (r->dataId == DID_VIN) {
            static const uint8_t value[] = {0xAA, 0xBB, 0xCC};
            r->copy(srv, value, sizeof(value));
            return UDS_PositiveResponse;
        }

        if (r->dataId == DID_TEST_SCRATCH) {
            r->copy(srv, stored_value, sizeof(stored_value));
            return UDS_PositiveResponse;
        }

        if (r->dataId == DID_ECU_SERIAL) {
            static const uint8_t value[] = {'S', 'N', '0', '0', '1'};
            r->copy(srv, value, sizeof(value));
            return UDS_PositiveResponse;
        }

        if (r->dataId == DID_ECU_HW_VER) {
            static const uint8_t value[] = {'H', 'W', '1', '.', '0'};
            r->copy(srv, value, sizeof(value));
            return UDS_PositiveResponse;
        }

        if (r->dataId == DID_ECU_SW_VER) {
            static const uint8_t value[] = {'S', 'W', '1', '.', '0'};
            r->copy(srv, value, sizeof(value));
            return UDS_PositiveResponse;
        }

        if (r->dataId == DID_SLOW_SCRATCH) {
            static int pending_count = 0;
            pending_count++;

            if (pending_count < 3) {
                printf("DID 0x%04X: still processing(%d), sending 0x78 pending...\n", r->dataId, pending_count);
                return UDS_NRC_RequestCorrectlyReceived_ResponsePending;
            } else {
                pending_count = 0;
                static const uint8_t value[] = {0x12, 0x34, 0x56};
                r->copy(srv, value, sizeof(value));
                return UDS_PositiveResponse;
            }
        }

        if (r->dataId == DID_SLOW_SCRATCH2) {
            static int pending_count = 0;
            pending_count++;

            if (pending_count < 5) {
                printf("DID 0x%04X: still processing(%d), sending 0x78 pending...\n", r->dataId, pending_count);
                return UDS_NRC_RequestCorrectlyReceived_ResponsePending;
            } else {
                pending_count = 0;
                static const uint8_t value[] = {0xAB, 0xCD, 0xEF};
                r->copy(srv, value, sizeof(value));
                return UDS_PositiveResponse;
            }
        }


        return UDS_NRC_RequestOutOfRange;
    }

    case UDS_EVT_WriteDataByIdent: {
        UDSWDBIArgs_t *w = (UDSWDBIArgs_t *)arg;

        if (w->dataId == DID_TEST_SCRATCH) {
            if (w->len != sizeof(stored_value)) {
                return UDS_NRC_IncorrectMessageLengthOrInvalidFormat;
            }
            memcpy(stored_value, w->data, w->len);
            return UDS_PositiveResponse;
        }

        return UDS_NRC_RequestOutOfRange;
    }

    default:
        printf("Unhandled event: %d, TA_Type: %d\n", ev, srv->r.info.A_TA_Type);
        return UDS_NRC_ServiceNotSupported;
    }
}

int main(int ac, char **av) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    // 1. Initialize a transport
    if (UDSTpIsoTpSockInitServer(&tp, "vcan0", 0x7E0, 0x7E8, 0x7DF)) {
        fprintf(stderr, "UDSTpIsoTpSockInitServer failed\n");
        exit(-1);
    }

    if (UDSServerInit(&srv)) {
        fprintf(stderr, "UDSServerInit failed\n");
        exit(-1);
    }

    srv.tp = (UDSTp_t *)&tp;
    srv.fn = fn;

    printf("server up, polling . . .\n");
    while (!done) {
        UDSServerPoll(&srv);
        sleep_ms(1);
    }
    printf("server exiting\n");
    return 0;
}

static int sleep_ms(uint32_t tms) {
    struct timespec ts;
    int ret;
    ts.tv_sec = tms / 1000;
    ts.tv_nsec = (tms % 1000) * 1000000;
    do {
        ret = nanosleep(&ts, &ts);
    } while (ret && errno == EINTR);
    return ret;
}