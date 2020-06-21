/*
 *  Hermes-Protocol parse program
 *  include status define and many types of data-processing
 */

#ifndef _HERMES_H_
#define _HERMES_H_

#include "network.h"

#define USE_PORT 6543
#define SERVER_BACKLOG 100
#define BUFFER_SIZE 1024

#define THREAD_ACCEPT_NAME(x)                          thread_accept_##x
#define THREAD_ACCEPT_NAME_P(x)                        &THREAD_ACCEPT_NAME(x)
#define GET_SESSION_TYPE(x,y)                          switch(x){case 0x01:y="系统消息";break;case 0x02:y="私聊";break;case 0x03:y="群聊";break;} 
#define PARSE_4BYTES_TO_INT(length,b,l,u,e)            *length = (b << ((sizeof(int) - 1) * CHAR_BIT)) + (l << ((sizeof(int) - 2) * CHAR_BIT)) + (u << ((sizeof(int) - 3) * CHAR_BIT)) + e 
#define PARSE_8BYTES_TO_LONG(length,b,l,u,e,n,i,c,d) \
*length = (b << ((sizeof(long) - 1) * CHAR_BIT)) + \
(l << ((sizeof(long) - 2) * CHAR_BIT)) + \
(u << ((sizeof(long) - 3) * CHAR_BIT)) + \
(e << ((sizeof(long) - 4) * CHAR_BIT)) + \
(n << ((sizeof(long) - 5) * CHAR_BIT)) + \
(i << ((sizeof(long) - 6) * CHAR_BIT)) + \
(c << ((sizeof(long) - 7) * CHAR_BIT)) + \
d \

#define DEC_1_ASCII  49

#define PLUSONE(x)  x+1
#define MINUSONE(y) y-1

#define CLIENT_USER_OPTIONS_PROMOPT \
"[CLIENT] 选择要发送的消息类型\n \
1] CONNECT (客户端连接至服务器)\n \
2] PING (服务端发送心跳包)\n \
3] SEND (客户端发送消息)\n \
4] PUSH (服务端推送消息)\n \
5] SYNC (客户端同步消息)\n \
6] DELETE (客户端要求删除会话)\n \
7] REQ (可扩展Request/Response获取信息)\n \
8] RECONNECT (服务端断开连接, 客户端请求重连)\n \
9] DISCONNECT (客户端/服务端主动断开)\n \
q] EXIT\n> "

#define MESSAGE_PACK_TYPE_STRING          "msgpack:{type:1,from:16843009,contents:hello,world,profile:{name:hermes,avatar:no}}"
#define REQ_TYPE_MESSAGE_PACK_STRING      "msgpack{session_type:2,session_id:16843009}"
#define REQ_TYPE_MESSAGE_PACK_ACK_STRING  "msgpack{session_type:2,session_id:16843009,name:hermes,avatar:no_avatar,note:no_note}"

#define PARSE_ANY_MESSAGE_FLAG(fixed_byte) \
    bool is_need_ack   = (fixed_byte & MASK_IS_NEED_ACK) == MASK_IS_NEED_ACK; \
    bool is_ack_type   = (fixed_byte & MASK_IS_ACK_TYPE) == MASK_IS_ACK_TYPE; \
    bool is_sync_type  = (fixed_byte & MASK_IS_SYNC_TYPE) == MASK_IS_SYNC_TYPE; \
    bool has_payload   = (fixed_byte & MASK_HAS_PAYLOAD) == MASK_HAS_PAYLOAD; \

#define PARSE_ANY_MESSAGE_FLAG_PRINT \
    printf("是否需要ack: %d\n",   is_need_ack); \
    printf("是ack类消息: %d\n",   is_ack_type); \
    printf("是sync类消息: %d\n",  is_sync_type); \
    printf("是否有消息体: %d\n",   has_payload); \

#define PUSH_SEND_MESSGAG_FLAG(msg_flag) \
    bool is_need_receipts   = (msg_flag & MASK_MSG_FLAG_NEED_RECEIPTS) == MASK_MSG_FLAG_NEED_RECEIPTS; \
    bool is_receipts_type   = (msg_flag & MASK_MSG_FLAG_IS_RECEIPTS) == MASK_MSG_FLAG_IS_RECEIPTS; \
    bool is_read            = (msg_flag & MASK_MSG_FLAG_IS_READ) == MASK_MSG_FLAG_IS_READ; \
    bool is_user_msg        = (msg_flag & MASK_MSG_FLAG_IS_USERMSG) == MASK_MSG_FLAG_IS_USERMSG; \
    bool is_delete          = (msg_flag & MASK_MSG_FLAG_IS_DELETE) == MASK_MSG_FLAG_IS_DELETE; \

#define PUSH_SEND_MESSAGE_FLAG_PRINT \
    printf("是否需要回执: %d\n",     is_need_receipts); \
    printf("是否回执类消息: %d\n",   is_receipts_type); \
    printf("是否已读: %d\n",        is_read); \
    printf("是否是用户消息: %d\n",   is_user_msg); \
    printf("消息是否已删除: %d\n\n", is_delete); \

#define PUSH_SEND_MESSAGE_SESSION_TYPE_PRINT(bytes_num) \
    unsigned char session_type = msg[bytes_num]; \
    char *session_type_string; \
    GET_SESSION_TYPE(session_type, session_type_string); \
    printf("会话类型: %s\n", session_type_string); \

#define PUSH_SEND_MESSAGE_SESSION_ID_PRINT(bytes_num) \
    int session_id; \
    _parse_4bytes_2_int(msg + bytes_num, &session_id); \
    printf("会话ID: %u\n", session_id); \

#define HERMES_VERSION                          0x03
#define MASK_IS_NEED_ACK                        0x01
#define MASK_IS_ACK_TYPE                        0x02
#define MASK_IS_SYNC_TYPE                       0x04
#define MASK_HAS_PAYLOAD                        0x08 

#define MASK_MSG_FLAG_NEED_RECEIPTS             0x01
#define MASK_MSG_FLAG_IS_RECEIPTS               0x02
#define MASK_MSG_FLAG_IS_READ                   0x04
#define MASK_MSG_FLAG_IS_USERMSG                0x08   
#define MASK_MSG_FLAG_IS_DELETE                 0x10
#define MSG_TYPE_PRIVATE_CHAT                   0x02
#define MSG_TYPE_SYSTEM                         0x01
#define MSG_TYPE_GROUP_CHAR                     0x03

#define MASK_CONNECT_TYPE                       0x10
#define MASK_PING_TYPE                          0x20
#define MASK_SEND_TYPE                          0x30
#define MASK_PUSH_TYPE                          0x40
#define MASK_SYNC_TYPE                          0x50
#define MASK_DELETE_TYPE                        0x60
#define MASK_REQ_TYPE                           0x70
#define MASK_RECONNECT_TYPE                     0xd0
#define MASK_DISCONNECT_TYPE                    0xe0

#define MASK_DELETE_TYPE_ALL_SESSION            0x10
#define MASK_DELETE_TYPE_SINGLE_SESSION         0x20
#define MASK_DELETE_TYPE_SINGLE_MESSAGE         0x30
#define MASK_DELETE_TYPE_IS_COMPLETELY          0x01

#define CONNECT_TYPE_UID                        16843009 //00000001000000010000000100000001
#define PING_TYPE_HEART_INTERVAL                6
#define PRIVATE_CHAT_SEND_UID                   16843009
#define PRIVATE_CHAT_LOCAL_ID                   9
#define MESSAGE_LOCAL_ID                        PRIVATE_CHAT_LOCAL_ID
#define MESSAGE_SYNC_LOCAL_ID                   66
#define MESSAGE_TIMESTAMP                       1590378633
#define SEND_TYPE_ACK_ERRNO                     (unsigned char)13
#define SEND_TYPE_ACK_ERROR                     "发送失败: 群聊: 我拉黑了他"
#define OFFLINE_REASON                          "已在其他设备上登录"
#define DELETE_TYPE_ACK_ERRNO                   (unsigned char)255
#define MESSAGE_DISTANCE                        22
#define MESSAGE_ID                              1314
#define SYNC_SINGLE_SESSION_START_MESSAGE_ID    88
#define SYNC_SINGLE_SESSION_END_MESSAGE_ID      99

/* TSD struct */
typedef struct tsd_sock_tag {
        pthread_t           thread_id;
        char                *name;
        struct sockaddr_in  *sockaddr;
        int                 *sockfd;
} tsd_sock_t;

/*  */
struct client_thread_refer_tag {
        int                 *sockfd;
        struct sockaddr_in  *addr;
};

typedef enum client_send_msg_type_ascii_t {
    /* CONNECT type*/
    CLIENT_SEND_MSG_TYPE_ASCII_CONNECT = DEC_1_ASCII,
     /* PING type*/
    CLIENT_SEND_MSG_TYPE_ASCII_PING = 50,
     /* SEND type*/
    CLIENT_SEND_MSG_TYPE_ASCII_SEND = 51,
     /* PUSH type*/
    CLIENT_SEND_MSG_TYPE_ASCII_PUSH = 52,
     /* SYNC type*/
    CLIENT_SEND_MSG_TYPE_ASCII_SYNC = 53,
     /* DELETE type*/
    CLIENT_SEND_MSG_TYPE_ASCII_DELETE,
    /* REQ type*/
    CLIENT_SEND_MSG_TYPE_ASCII_REQ,
     /* RECONNECT type*/
    CLIENT_SEND_MSG_TYPE_ASCII_RECONNECT = 56,
     /* DISCONNECT type*/
    CLIENT_SEND_MSG_TYPE_ASCII_DISCONNECT = 57,
     
     /* Special option, user quit program*/
    CLIENT_SEND_MSG_TYPE_ASCII_QUIT = 113
} client_send_msg_type_ascii_t;

typedef enum client_send_msg_type_int_t {
    /* CONNECT type*/
    CLIENT_SEND_MSG_TYPE_INT_CONNECT = 0x1,
    /* PING type*/
    CLIENT_SEND_MSG_TYPE_INT_PING,
    /* SEND type*/
    CLIENT_SEND_MSG_TYPE_INT_SEND,
    /* PUSH type*/
    CLIENT_SEND_MSG_TYPE_INT_PUSH,
    /* SYNC type*/
    CLIENT_SEND_MSG_TYPE_INT_SYNC,
    /* DELETE type*/
    CLIENT_SEND_MSG_TYPE_INT_DELETE,
    /* REQ type*/
    CLIENT_SEND_MSG_TYPE_INT_REQ,
    /* RECONNECT type*/
    CLIENT_SEND_MSG_TYPE_INT_RECONNECT = 0xd,
    /* DISCONNECT type*/
    CLIENT_SEND_MSG_TYPE_INT_DISCONNECT,
} client_send_msg_type_int_t;

typedef enum server_send_msg_type_t {
    /* server send send-ack to client */
    SERVER_SEND_MSG_TYPE_SEND_ACK = 0x1,
} server_send_msg_type_t;

typedef enum delete_ack_errno_t {
    /* unsupport protocol*/
    DELETE_ACK_ERRNO_UNSUPPORT_PROTOCOL = 1,

    /* illegal protocol */
    DELETE_ACK_ERRNO_ILLEGAL_PROTOCOL,

    /* not allowed */
    DELETE_ACK_ERRNO_NOT_ALLOWED,

    /* delete faild */
    DELETE_ACK_ERRNO_DELETE_FAILD = 255
} delete_ack_errno_t;

typedef enum sync_ack_errno_t {
    /* success */
    SYNC_ACK_ERRNO_SUCCESS = 0,

    /* unsupport protocol*/
    SYNC_ACK_ERRNO_UNSUPPORT_PROTOCOL = 1,

    /* illegal protocol */
    SYNC_ACK_ERRNO_ILLEGAL_PROTOCOL,

    /* exceed limit */
    SYNC_ACK_ERRNO_EXCEED_LIMIT,

    /* sync faild */
    SYNC_ACK_ERRNO_SYNC_FAILD = 255
} sync_ack_errno_t;

void _parse_hermes_bytes_(unsigned char *);
void _int_padded_to_charbytes(unsigned char *, long, int);

unsigned char * get_connect_msg(void);

unsigned char * get_send_privchat_msg_user(void);
unsigned char * get_send_privchat_msg_recvack(void);
unsigned char * get_send_privchat_msg_readack(void);
unsigned char * get_send_user_msg_ack_with_errno(void);

unsigned char * get_push_privchat_msg_user(void);
unsigned char * get_push_privchat_msg_readack(void);

unsigned char * get_delete_all_sessions_type_msg(void);
unsigned char * get_delete_single_session_type_msg(void);
unsigned char * get_delete_single_message_type_msg(void);
unsigned char * get_delete_ack_type_msg(void);

unsigned char * get_req_sessions_info_msg(void);
unsigned char * get_req_session_info_ack_msg(void);

unsigned char * get_sync_msg(void);
unsigned char * get_push_msg(void);

unsigned char * get_ping_msg(void);
unsigned char * get_disconnect_msg(void);

unsigned char * get_sync_new_message_msg(void);
unsigned char * get_sync_single_session_msg(void);
unsigned char * get_sync_last_message_msg(void);
unsigned char * get_sync_ack_msg(void);
char * _get_sync_ack_error(unsigned char);
void _get_sync_ack_messages(unsigned char *, int *);
void _parse_sync_ack_messages(unsigned char *);

char * _get_delete_error(unsigned char);
char * _get_send_error(unsigned char);
char * _get_offline_reason(unsigned char);

void get_type_string(char *, unsigned char);
void _parse_connect_msg_payload(unsigned char *, bool);
void _parse_ping_msg_payload(unsigned char *, bool);
void _parse_send_msg_payload(unsigned char *, bool);
void _parse_push_msg_payload(unsigned char *, bool);
void _parse_delete_msg_payload(unsigned char *, bool);
void _parse_req_msg_payload(unsigned char *, bool);
void _parse_disconnect_msg_payload(unsigned char *, bool);
void _parse_sync_msg_payload(unsigned char *, bool);

void _parse_4bytes_2_int(unsigned char *, int *);
void _parse_2bytes_2_short(unsigned char * msg, short int *);
void _parse_8bytes_2_long(unsigned char * msg, long *);
unsigned char * _set_fixed_bytes_(unsigned char, bool, bool, bool, bool);
unsigned char _set_send_push_message_flag(bool need_receipts, bool is_receipts, bool is_read, bool is_usermsg, bool is_delete);
unsigned char _set_message_delete_flag(unsigned char DELETE_MASK, bool is_delete_completely);
char * get_delete_type_string(unsigned char);
char * get_sync_type_string(unsigned char);

#endif