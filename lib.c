#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"

S_data *construct_data_S(unsigned char MAXL, unsigned char TIME,
unsigned char NPAD, unsigned char PADC, unsigned char EOL)
{
    S_data *data = malloc(sizeof(S_data));

    data->MAXL = MAXL;
    data->TIME = TIME;
    data->NPAD = NPAD;
    data->PADC = PADC;
    data->EOL = EOL;

    data->QCTL = 0;
    data->QBIN = 0;
    data->CHKT = 0;
    data->REPT = 0;
    data->CAPA = 0;
    data->R = 0;
    return data;
}


void initialize_header (paket_header *header,
 unsigned char SEQ, unsigned char TYPE, size_t size_data)
{
    header->SOH = 0x01;
    header->LEN = sizeof(paket_header) + sizeof(paket_footer) + size_data - 2;
    header->SEQ = SEQ;
    header->TYPE = TYPE;
}

void init_msg(msg* t, const paket_header *header, const void *data,
    size_t len_data, unsigned char mark)
{
    t->len = header->LEN + 2;
    size_t n;
    paket_footer *footer = malloc(sizeof(paket_footer));
    n = sizeof(paket_header);
    memcpy(t->payload, header, n);

    if(data != NULL)
        memcpy(t->payload + n, data, len_data);

   footer->CHECK = crc16_ccitt(t->payload, t->len - 3);
   //printf("sender ccr %hu\n", footer->CHECK);
   footer->MARK = mark;

   memcpy(t->payload + n + len_data, footer, sizeof(paket_footer));
   free(footer);
}
