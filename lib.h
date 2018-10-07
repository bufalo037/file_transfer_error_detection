#ifndef LIB
#define LIB

#define SEQ_MAX 64

typedef struct {
    int len;
    char payload[1400];
} msg;

typedef struct 
{
	unsigned char SOH;
	unsigned char LEN;
	unsigned char SEQ;
	unsigned char TYPE;
	
} __attribute__ (( __packed__ )) paket_header;

typedef struct 
{
	unsigned short CHECK;
	unsigned char MARK;

} __attribute__ (( __packed__ )) paket_footer;

typedef struct 
{
	unsigned char MAXL;
	unsigned char TIME;
	unsigned char NPAD;
	unsigned char PADC;
	unsigned char EOL;
	unsigned char QCTL;
	unsigned char QBIN;
	unsigned char CHKT;
	unsigned char REPT;
	unsigned char CAPA;
	unsigned char R;

} __attribute__ (( __packed__ )) S_data;

void initialize_header (paket_header *header,
 unsigned char SEQ, unsigned char TYPE, size_t size_data);
void init_msg(msg* t, const paket_header *header, const void *data,
    size_t len_data, unsigned char mark);
S_data *construct_data_S(unsigned char MAXL, unsigned char TIME,
unsigned char NPAD, unsigned char PADC, unsigned char EOL);

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);

#endif

