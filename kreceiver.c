#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"
#include <time.h>

#define HOST "127.0.0.1"
#define PORT 10001

static void write_file(FILE *file, void *message, size_t size)
{
    fwrite(message, sizeof(char), size, file);
}


static char set_type_msg(const msg *r)
{
    printf("receiver ccr %hu ccr %hu\n", crc16_ccitt(r->payload, r->len - 3),*(unsigned short *)(r->payload + r->len - 3));
    if(crc16_ccitt(r->payload, r->len - 3) == *(unsigned short *)(r->payload + r->len - 3))
        return 'Y';
    return 'N';
}


static char *get_file_name(msg *r)
{
    size_t n = sizeof(char) * (((paket_header *)r->payload)->LEN);
    //LEN e pana la final -3 bytes -2 . +5 bytes pt "recv_"
    char *nume = malloc(n + 1);
    sprintf(nume,"recv_");
    memcpy(nume + 5,((char *)r->payload + sizeof(paket_header)), n - 5);
    nume[n] = '\0';
    return nume;
}


static int receive_messages(msg *t, unsigned int time)
{
    //return 0 ok return -1 timeout
    int i;
    static S_data *s_data = NULL;
    static FILE *crt_file = NULL;
    static paket_header* header = NULL;
   
    msg *r;
    char msg_type;

    if (header == NULL)
        header = malloc(sizeof(paket_header));

    for(i = 0;i < 3;)
    {
        printf("RECEIVR ASCULTA\n");
        r = receive_message_timeout(1000 * time);
        if(r != NULL) //TODO: de verificat
        {
            if(t == NULL || ((paket_header *)r->payload)->SEQ ==
                (((paket_header *)t->payload)->SEQ +1) % SEQ_MAX)
            {
                switch(((paket_header *)r->payload)->TYPE)
                {
                    case 'S':
                    {   if(s_data == NULL)
                        {
                            s_data =  malloc(sizeof(S_data));   
                            s_data = construct_data_S(250, 5, 0, 0, 0x0D);
                        }

                        if(t == NULL)
                        {
                            t = malloc(sizeof(msg));
                        }
                        msg_type = set_type_msg(r);
                        initialize_header(header,((paket_header *)r->payload)->SEQ,
                         msg_type, sizeof(S_data));
                        init_msg(t, header, s_data, sizeof(S_data), s_data->EOL);
                        printf("S received %c\n", msg_type);
                        send_message(t);
                        return receive_messages(t, s_data->TIME);
                    }
                    case 'F':
                    {
                        msg_type = set_type_msg(r);
                        printf("F received %c\n", msg_type);
                        initialize_header(header,((paket_header *)r->payload)->SEQ, msg_type, 0);
                        init_msg(t, header, NULL, 0, s_data->EOL);
                        send_message(t);
                        if(msg_type == 'N')
                            return receive_messages(t, s_data->TIME);
                        char *nume = get_file_name(r);
                        crt_file = fopen(nume, "wb");
                        free(nume);
                        
                        return receive_messages(t, s_data->TIME);


                    }
                    case 'D':
                    {
                        msg_type = set_type_msg(r);
                        printf("D received %c\n", msg_type);
                        initialize_header(header,((paket_header *)r->payload)->SEQ, msg_type, 0);
                        init_msg(t, header, NULL, 0, s_data->EOL);
                        send_message(t);
                        if(msg_type == 'N')
                        {
                            return receive_messages(t, s_data->TIME);
                        }
                        write_file(crt_file, (r->payload + sizeof(paket_header)),
                         r->len - sizeof(paket_header) - sizeof(paket_footer));
                        return receive_messages(t, s_data->TIME);

                    }
                    case 'Z':
                    {
                        msg_type = set_type_msg(r);
                        printf("Z received %c\n", msg_type);
                        initialize_header(header,((paket_header *)r->payload)->SEQ, msg_type, 0);
                        init_msg(t, header, NULL, 0, s_data->EOL);
                        send_message(t);
                        if(msg_type == 'N')
                            return receive_messages(t, s_data->TIME);
                        fclose(crt_file);
                        return receive_messages(t, s_data->TIME);

                    }
                    case 'B':
                    {
                        msg_type = set_type_msg(r);
                        printf("B received %c\n", msg_type);
                        initialize_header(header,((paket_header *)r->payload)->SEQ, msg_type, 0);
                        init_msg(t, header, NULL, 0, s_data->EOL);
                        send_message(t);
                        if(msg_type == 'N')
                            return receive_messages(t, s_data->TIME);
                        free(s_data);
                        free(header);
                        free(t);
                        printf("PAKET B PRIMIT CORECT\n");
                        return 0;
                    }
                }
            }
           
        }
        else
        {
            
            i++;
            if(t != NULL)
            {
                send_message(t);
            }
        }
    }
    return -1;
}


int main(int argc, char** argv) {

    init(HOST, PORT);


    if(receive_messages(NULL, 5) == 0)
        printf("succes\n");
    else
        printf("timeout receiver\n");

    return 0;
}
