/*
 * bytes data send to server
 */

#ifndef _CLIENT_SELECT_FUNCS_H_
#define _CLIENT_SELECT_FUNCS_H_

#include "network.h"

#define FUNC_POINTER_R_VP_ARG_I  r_vp_arg_i
#define DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(vari_name_suffix) FUNC_POINTER_R_VP_ARG_I##vari_name_suffix

#define SEND_TYPE_USER_OPTIONS_PROMOPT          printf("[SENDER] 选择 SEND 消息类型(默认1)\n 1] 用户消息\n 2] 接受回执\n 3] 已读回执\n 4] SEND-ACK\n> ")
#define PUSH_TYPE_USER_OPTIONS_PROMOPT          printf("[SENDER] 选择 PUSH 消息类型(默认1)\n 1] 推送消息\n 2] 已读回执\n> ")
#define DELETE_TYPE_USER_OPTIONS_PROMOPT        printf("[SENDER] 选择 DELETE 消息类型(默认1)\n 1] 删除所有会话\n 2] 删除指定会话\n 3] 删除单条消息\n 4] DELETE-ACK \n> ")
#define REQ_TYPE_USER_OPTIONS_PROMOPT           printf("[SENDER] 选择 REQ 消息类型\n 1] 获取消息\n 2] REQ-ACK \n> ")

#ifndef DEC_1_ASCII
#define DEC_1_ASCII 49
#endif

#define ASCII_TO_DEC(option) option-DEC_1_ASCII
#define OPTION_TOFUNC_INDEX(option) ASCII_TO_DEC(option)
#define CLIENT_CALL_SEND_FUNC(option,fd,iswait) \
iswait=1;(DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[OPTION_TOFUNC_INDEX(option)])(clientfd)

#define SYNC_TYPE_USER_OPTIONS_PROMOPT \
printf("[SENDER] 选择 SYNC 消息类型(默认1)\n 1] 同步新消息\n 2] 同步指定会话\n 3] 同步指定会话最后一条消息 \n 4] SYNC-ACK\n> ");

#define MSG_TYPE_PROMOPT(MSG_TYPE)              printf("[SENDER] 发送了 " MSG_TYPE " 类型消息");

/* define void_p-type return and int-type arg function pointer */
typedef void (*FUNC_POINTER_R_VP_ARG_I)(int);

typedef enum send_type_msg_species_t {
    /*  user message, represent '1'*/
    SEND_TYPE_MSG_SPECIES_USER_MSG = DEC_1_ASCII,

    /* recieve ack */
    SEND_TYPE_MSG_SPECIES_IS_RECV,

    /* readed ack */
    SEND_TYPE_MSG_SPECIES_IS_READED,

    /* ack */
    SEND_TYPE_MSG_SPECIES_SEND_ACK
} send_type_msg_species_t;

typedef enum push_type_msg_species_t {
    /*  user message, represent '1'*/
    PUSH_TYPE_MSG_SPECIES_USER_MSG = DEC_1_ASCII,

    /* readed ack */
    PUSH_TYPE_MSG_SPECIES_IS_READED = 50
} push_type_msg_species_t;

typedef enum delete_type_msg_species_t {
    /*  delete all sessions*/
    DELETE_TYPE_MSG_SPECIES_DELETE_ALL_SESSIONS = DEC_1_ASCII,

    /* delete single session */
    DELETE_TYPE_MSG_SPECIES_DELETE_SINGLE_SESSION = 50,

    /* delete single message */
    DELETE_TYPE_MSG_SPECIES_DELETE_SINGLE_MESSAGE = 51,

    /* ack */
    DELETE_TYPE_MSG_SPECIES_DELETE_ACK
} delete_type_msg_species_t;

typedef enum req_type_msg_species_t {
    /*  user message, represent '1'*/
    REQ_TYPE_MSG_SPECIES_REQ_MSG = 49,

    /* readed ack */
    REQ_TYPE_MSG_SPECIES_REQ_ACK
} req_type_msg_species_t;

typedef enum sync_type_msg_species_t {
    /* sync all/new session */
    SYNC_TYPE_MSG_SPECIES_SYNC_NEW_MESSAGE = 49,

    /* sync single message */
    SYNC_TYPE_MSG_SPECIES_SYNC_SINGLE_SESSION = 50,

    /* sync last message */
    SYNC_TYPE_MSG_SPECIES_SYNC_LAST_MESSAGE = 51,

    /* ack */
    SYNC_TYPE_MSG_SPECIES_SYNC_ACK
} sync_type_msg_species_t;

void _stoupper(char *);
void _send_msg_2_fd(unsigned char *, int);
void client_send_connect_type_msg(int);
void client_send_ping_type_msg(int);
void client_send_send_type_msg(int);
void client_send_disconnect_type_msg(int);
void client_send_reconnect_type_msg(int);
void client_send_req_type_msg(int);
void client_send_delete_type_msg(int);
void client_send_sync_type_msg(int);

#endif