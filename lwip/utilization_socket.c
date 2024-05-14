/*
 * Copyright 2020, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <string.h>
#include <microkit.h>

#include <lwip/ip.h>
#include <lwip/pbuf.h>
#include <lwip/tcp.h>

#include <echo.h>

#define START_PMU 3
#define STOP_PMU 5

/* This file implements a TCP based utilization measurment process that starts
 * and stops utilization measurements based on a client's requests.
 * The protocol used to communicate is as follows:
 * - Client connects
 * - Server sends: 100 IPBENCH V1.0\n
 * - Client sends: HELLO\n
 * - Server sends: 200 OK (Ready to go)\n
 * - Client sends: LOAD cpu_target_lukem\n
 * - Server sends: 200 OK\n
 * - Client sends: SETUP args::""\n
 * - Server sends: 200 OK\n
 * - Client sends: START\n
 * - Client sends: STOP\n
 * - Server sends: 220 VALID DATA (Data to follow)\n
 *                                Content-length: %d\n
 *                                ${content}\n
 * - Server closes socket.
 *
 * It is also possible for client to send QUIT\n during operation.
 *
 * The server starts recording utilization stats when it receives START and
 * finishes recording when it receives STOP.
 *
 * Only one client can be connected.
 */

static struct tcp_pcb *utiliz_socket;
uintptr_t data_packet;
uintptr_t cyclecounters_vaddr;

#define WHOAMI "100 IPBENCH V1.0\n"
#define HELLO "HELLO\n"
#define OK_READY "200 OK (Ready to go)\n"
#define LOAD "LOAD cpu_target_lukem\n"
#define OK "200 OK\n"
#define SETUP "SETUP args::\"\"\n"
#define START "START\n"
#define STOP "STOP\n"
#define QUIT "QUIT\n"
#define KBD "KBD\n"
#define RESPONSE "220 VALID DATA (Data to follow)\n"    \
    "Content-length: %d\n"                              \
    "%s\n"
#define IDLE_FORMAT ",%ld,%ld"
#define ERROR "400 ERROR\n"

#define msg_match(msg, match) (strncmp(msg, match, strlen(match))==0)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define RES(x, y, z) "220 VALID DATA (Data to follow)\n"    \
    "Content-length: "STR(x)"\n"\
    ","STR(y)","STR(z)


#define ULONG_MAX 0xffffffffffffffff

struct bench *bench = (void *)(uintptr_t)0x5010000;
struct tcb_pcb *curr_pcb;
#define BUFLEN 7
char kbd_buf[BUFLEN];
int buf_index = 0;

uint64_t start;
uint64_t idle_ccount_start;
uint64_t idle_overflow_start;


static inline void my_reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

static inline void my_itoa(uint64_t n, char s[])
{
    int i;
    uint64_t sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    my_reverse(s);
}

static err_t utilization_sent_callback(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    return ERR_OK;
}

static err_t utilization_recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if (p == NULL) {
        tcp_close(pcb);
        return ERR_OK;
    }

    pbuf_copy_partial(p, data_packet, p->tot_len, 0);
    err_t error;

    if (msg_match(data_packet, HELLO)) {
        error = tcp_write(pcb, OK_READY, strlen(OK_READY), TCP_WRITE_FLAG_COPY);
        if (error) {
            microkit_dbg_puts("Failed to send OK_READY message through utilization peer");
        }
    } else if (msg_match(data_packet, LOAD)) {
        error = tcp_write(pcb, OK, strlen(OK), TCP_WRITE_FLAG_COPY);
        if (error) {
            microkit_dbg_puts("Failed to send OK message through utilization peer");
        }
    } else if (msg_match(data_packet, SETUP)) {
        error = tcp_write(pcb, OK, strlen(OK), TCP_WRITE_FLAG_COPY);
        if (error) {
            microkit_dbg_puts("Failed to send OK message through utilization peer");
        }
    } else if (msg_match(data_packet, START)) {
        printf("measurement starting... \n");

        // start = bench->ts;
        // idle_ccount_start = bench->ccount;
        // idle_overflow_start = bench->overflows;

        microkit_notify(START_PMU);

    } else if (msg_match(data_packet, STOP)) {        
        printf("measurement finished \n");;
        
        uint64_t total, idle;

        // total = bench->ts - start;
        // total += ULONG_MAX * (bench->overflows - idle_overflow_start);
        // idle = bench->ccount - idle_ccount_start;

        char tbuf[16];
        my_itoa(total, tbuf);

        char ibuf[16];
        my_itoa(idle, ibuf);

        char buffer[100];

        int len = strlen(tbuf) + strlen(ibuf) + 2;
        char lbuf[16];
        my_itoa(len, lbuf);

        strcat(strcpy(buffer, "220 VALID DATA (Data to follow)\nContent-length: "), lbuf);
        strcat(buffer, "\n,");
        strcat(buffer, ibuf);
        strcat(buffer, ",");
        strcat(buffer, tbuf);

        // microkit_dbg_puts(buffer);
        error = tcp_write(pcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);

        tcp_shutdown(pcb, 0, 1);
        
        microkit_notify(STOP_PMU);
    } else if (msg_match(data_packet, QUIT)) {
        /* Do nothing for now */
    } else if (msg_match(data_packet, KBD)) {
        if (curr_pcb) {
            printf("\nsent '%s'\n", kbd_buf);
            char* str = "RECEIVED: '";
            error = tcp_write(curr_pcb, str, strlen(str), TCP_WRITE_FLAG_COPY);
            error = tcp_write(curr_pcb, kbd_buf, strlen(kbd_buf), TCP_WRITE_FLAG_COPY);
            str = "'\n";
            error = tcp_write(curr_pcb, str, strlen(str), TCP_WRITE_FLAG_COPY);
            for (int i = 0; i < 20; i++) {
                kbd_buf[i] = '\0';
            }
            buf_index = 0;
            if (error) {
                microkit_dbg_puts("Failed to send OK message through utilization peer");
            }
        } else {
            curr_pcb = pcb;
            printf("Keyboard primed\n");
            char* return_string = "Keyboard ready for input\n";
            error = tcp_write(pcb, return_string, strlen(return_string), TCP_WRITE_FLAG_COPY);
            if (error) {
                microkit_dbg_puts("Failed to send OK message through utilization peer");
            }
        }
    } else {
        microkit_dbg_puts("Received a message that we can't handle ");
        microkit_dbg_puts(data_packet);
        microkit_dbg_puts("\n");
        // printf("input :%s:, match = :%s:\n", data_packet, KBD);
        error = tcp_write(pcb, ERROR, strlen(ERROR), TCP_WRITE_FLAG_COPY);
        if (error) {
            microkit_dbg_puts("Failed to send OK message through utilization peer");
        }
    }

    return ERR_OK;
}

static err_t utilization_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    microkit_dbg_puts("Utilization connection established!\n");
    curr_pcb = newpcb;
    err_t error = tcp_write(newpcb, WHOAMI, strlen(WHOAMI), TCP_WRITE_FLAG_COPY);
    if (error) {
        microkit_dbg_puts("Failed to send WHOAMI message through utilization peer");
    }
    tcp_sent(newpcb, utilization_sent_callback);
    tcp_recv(newpcb, utilization_recv_callback);
    
    return ERR_OK;
}

int setup_utilization_socket(void)
{
    utiliz_socket = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (utiliz_socket == NULL) {
        microkit_dbg_puts("Failed to open a socket for listening!");
        return -1;
    }

    err_t error = tcp_bind(utiliz_socket, IP_ANY_TYPE, UTILIZATION_PORT);
    if (error) {
        microkit_dbg_puts("Failed to bind the TCP socket");
        return -1;
    } else {
        microkit_dbg_puts("Utilisation port bound to port 1236\n");
    }

    utiliz_socket = tcp_listen_with_backlog_and_err(utiliz_socket, 1, &error);
    if (error != ERR_OK) {
        microkit_dbg_puts("Failed to listen on the utilization socket");
        return -1;
    }
    tcp_accept(utiliz_socket, utilization_accept_callback);

    return 0;
}

int send_keypress(char c) {
    if (curr_pcb) {
        if (buf_index < BUFLEN) {
            kbd_buf[buf_index] = c;
            buf_index++;
            /* kbd_buf[buf_index++] = '\0'; */
        } else {
            printf("\nbuffer full (max 7 chars)!\n");
        }
        // int error = tcp_write(curr_pcb, str, strlen(str), TCP_WRITE_FLAG_COPY);
    } else {
        printf("keyboard not ready, type KBD in terminal\n");
    }
    return 0;
}