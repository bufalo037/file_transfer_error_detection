#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/timeb.h> //?
#include "lib.h"
#include <time.h>

#define HOST "127.0.0.1"
#define PORT 10000

static FILE *get_file_r(const char *name)
{
    return fopen(name,"rb");
}


static char *read_data(FILE * file, int maxl, size_t* len_data)
{
    char *real_data;
    char *data = malloc(sizeof(char) * maxl);
    *len_data = fread(data, sizeof(char), maxl, file);
    
    real_data = realloc(data, sizeof(char) * *len_data);

    return real_data;
}


static int deliver_message(const msg *t, unsigned int time)
{
    //return 0 ACK, return 1 NAK, return -1 timeout x3
    int i;
    msg *msg2;
    for(i = 0; i < 3;)
    {
        send_message(t);
            //free(msg2);         
        printf("SENDER ASTEAPTA RASPUNS\n");
        msg2 = receive_message_timeout(1000 * time); //-diff
        
        if(msg2 != NULL) 
        {

            printf("SENDER RECEVED PAKKEGE SEQ %d type %c\n",
                 ((paket_header *)msg2->payload)->SEQ, ((paket_header *)msg2->payload)->TYPE);

            if( ((paket_header *)msg2->payload)->SEQ ==
                ((paket_header *)t->payload)->SEQ)
            {
                
                if(((paket_header *)msg2->payload)->TYPE == 'Y')
                {
                    // din cauza ca nu ma folosesc nicaieri de informatia
                    // din packetul S nu are rost chiar daca packetul de ACK
                    // trimite preferintele receptorului inapoi ca raspuns sa le 
                    // salvez sau citesc pentru ca nu ma voi folosi niciodata
                    // de ele.
                    free(msg2);
                    return 0;

                }
                if(((paket_header *)msg2->payload)->TYPE == 'N')
                {
                    free(msg2);
                    return 1;
                }
            }
            else
            {
                printf("SEQ PRIMIT != SEQ CURENT\n");  
            }
        }
        else
        {
            i++;
        }

    }
    return -1;
}

static void transmitere_mesaj(msg *t, int *SEQ, unsigned char time)
{
    int err_code;
    err_code = deliver_message(t, time);
    if(err_code == -1)
    {
        fprintf(stderr,"Timeout ksender");
        exit(EXIT_FAILURE);
    }

    if(err_code == 0)
    {
        (*SEQ)++;
        (*SEQ) = (*SEQ) % SEQ_MAX;
        return;
    }

    (*SEQ)++;
    (*SEQ) = (*SEQ) % SEQ_MAX;
    ((paket_header *)(t->payload))->SEQ = *SEQ;
    *((unsigned short *)(t->payload + t->len - 3)) = crc16_ccitt(t->payload, t->len - 3);
    transmitere_mesaj(t, SEQ ,time); //poate fi optimizat cu recursivitate pe coada
}

int main(int argc, char** argv) {


    int i, SEQ = 0;
    FILE *crt_file;
    size_t len_data;
    char *data;

    msg *t = malloc(sizeof(msg));
    paket_header *header = malloc(sizeof(paket_header));
    S_data *s_data;
    
    init(HOST, PORT);

    //initialize transmision
    printf("Incepe transmisia\n");
    s_data = construct_data_S(250, 5, 0, 0, 0x0D);
    initialize_header(header, SEQ, 'S', sizeof(S_data));
    init_msg(t, header, s_data, sizeof(S_data), s_data->EOL);
    printf("Inainte de transmie\n");
    transmitere_mesaj(t, &SEQ, s_data->TIME);
    printf("COMPELETE S\n");


    for(i = 1;i < argc; i++)
    {
        int n;
        crt_file = get_file_r(argv[i]);
        n = strlen(argv[i]);

        data = malloc(sizeof(char) * (n + 1));
        strncpy(data, argv[i], n);
        data[n] = '\0';
        len_data = n + 1;

        initialize_header(header, SEQ, 'F', len_data);
        init_msg(t, header, data, len_data, s_data->EOL);
        transmitere_mesaj(t, &SEQ, s_data->TIME);
        free(data);
        printf("COMPELETE F\n");
        do
        {
            data = read_data(crt_file, s_data->MAXL, &len_data);
            initialize_header(header, SEQ, 'D', len_data);
            init_msg(t, header, data, len_data, s_data->EOL);
            transmitere_mesaj(t, &SEQ, s_data->TIME);
            free(data);
        }
        while(s_data->MAXL == len_data);
        printf("COMPELETE D\n");
        initialize_header(header, SEQ, 'Z', 0);
        init_msg(t, header, NULL, 0, s_data->EOL);
        transmitere_mesaj(t, &SEQ, s_data->TIME);
        fclose(crt_file);
        printf("COMPELETE Z\n");
    }

    printf("A AJUNS LA B\n");
    initialize_header(header, SEQ, 'B', 0);
    init_msg(t, header, NULL, 0, s_data->EOL);
    transmitere_mesaj(t, &SEQ, s_data->TIME);
    printf("COMPELETE B\n");

    free(t);
    free(header);
    free(s_data);
    

    return 0;
}
