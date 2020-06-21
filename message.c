#include "message.h"
#include "hermes.h"

void _send_msg_2_fd(unsigned char * send_msg, int clientfd) 
{
    printf(",共 %zd 字节\n", strlen((const char *)send_msg));
    ssize_t send_bytes = send(clientfd, send_msg, strlen((const char *)send_msg), 0);
    if (send_bytes == strlen((const char *)send_msg)) {
        free(send_msg);
    }
}

void client_send_connect_type_msg(int clientfd) 
{
    MSG_TYPE_PROMOPT("CONNECT");
    unsigned char *send_msg = get_connect_msg();
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_ping_type_msg(int clientfd)
{
    MSG_TYPE_PROMOPT("PING");
    unsigned char *send_msg = get_ping_msg();
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_send_type_msg(int clientfd)
{
    SEND_TYPE_USER_OPTIONS_PROMOPT;
    send_type_msg_species_t option = fgetc(stdin);getchar();

    unsigned char *send_msg;
    switch(option) 
    {
        case SEND_TYPE_MSG_SPECIES_USER_MSG:
        default:
            MSG_TYPE_PROMOPT("用户消息");
            send_msg = get_send_privchat_msg_user();
        break;

        case SEND_TYPE_MSG_SPECIES_IS_RECV:
            MSG_TYPE_PROMOPT("接受回执");
            send_msg = get_send_privchat_msg_recvack();
        break;

        case SEND_TYPE_MSG_SPECIES_IS_READED:
            MSG_TYPE_PROMOPT("已读回执");
            send_msg = get_send_privchat_msg_readack();
        break;

        case SEND_TYPE_MSG_SPECIES_SEND_ACK:
            MSG_TYPE_PROMOPT("SEND-ACK");
            send_msg = get_send_user_msg_ack_with_errno();
        break;
    }
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_push_type_msg(int clientfd)
{
    PUSH_TYPE_USER_OPTIONS_PROMOPT;
    push_type_msg_species_t option = fgetc(stdin);getchar();

    unsigned char *send_msg;
    switch(option) 
    {
        case PUSH_TYPE_MSG_SPECIES_USER_MSG:
            MSG_TYPE_PROMOPT("推送消息");
            send_msg = get_push_privchat_msg_user();
        default:
        break;

        case PUSH_TYPE_MSG_SPECIES_IS_READED:
            MSG_TYPE_PROMOPT("已读回执");
            send_msg = get_push_privchat_msg_readack();
        break;
    }
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_delete_type_msg(int clientfd)
{
    unsigned char *send_msg;

    DELETE_TYPE_USER_OPTIONS_PROMOPT;
    delete_type_msg_species_t option = fgetc(stdin);getchar();
    switch(option)
    {
        case DELETE_TYPE_MSG_SPECIES_DELETE_ALL_SESSIONS:
        default:
            MSG_TYPE_PROMOPT("删除所有会话");
            send_msg = get_delete_all_sessions_type_msg();
        break;

        case DELETE_TYPE_MSG_SPECIES_DELETE_SINGLE_SESSION:
            MSG_TYPE_PROMOPT("删除指定会话");
            send_msg = get_delete_single_session_type_msg();
        break;

        case DELETE_TYPE_MSG_SPECIES_DELETE_SINGLE_MESSAGE:
            MSG_TYPE_PROMOPT("删除单条消息");
            send_msg = get_delete_single_message_type_msg();
        break;

        case DELETE_TYPE_MSG_SPECIES_DELETE_ACK:
            MSG_TYPE_PROMOPT("DELETE-ACK");
            send_msg = get_delete_ack_type_msg();
        break;
    }
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_req_type_msg(int clientfd)
{
    unsigned char *send_msg;
    REQ_TYPE_USER_OPTIONS_PROMOPT;
    delete_type_msg_species_t option = fgetc(stdin);getchar();
    switch(option)
    {
        case REQ_TYPE_MSG_SPECIES_REQ_MSG:
        default:
            MSG_TYPE_PROMOPT("获取消息");
            send_msg = get_req_sessions_info_msg();
        break;

        case REQ_TYPE_MSG_SPECIES_REQ_ACK:
            MSG_TYPE_PROMOPT("REQ-ACK");
            send_msg = get_req_session_info_ack_msg();
        break;
    }
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_reconnect_type_msg(int clientfd)
{
    MSG_TYPE_PROMOPT("RECONNECT");
    unsigned char *send_msg = (unsigned char *)malloc(1);

    *send_msg |= MASK_RECONNECT_TYPE;

    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_disconnect_type_msg(int clientfd)
{
    MSG_TYPE_PROMOPT("DISCONNECT(SERVER-SEND)");
    unsigned char *send_msg = get_disconnect_msg();
    _send_msg_2_fd(send_msg, clientfd);
}

void client_send_sync_type_msg(int clientfd)
{
    unsigned char *send_msg;
    SYNC_TYPE_USER_OPTIONS_PROMOPT
    sync_type_msg_species_t option = fgetc(stdin);getchar();
    switch(option)
    {
        case SYNC_TYPE_MSG_SPECIES_SYNC_NEW_MESSAGE:
        default:
            MSG_TYPE_PROMOPT("同步新消息");
            send_msg = get_sync_new_message_msg();
        break;
        case SYNC_TYPE_MSG_SPECIES_SYNC_SINGLE_SESSION:
            MSG_TYPE_PROMOPT("同步指会话");
            send_msg = get_sync_single_session_msg();
        break;
        case SYNC_TYPE_MSG_SPECIES_SYNC_LAST_MESSAGE:
            MSG_TYPE_PROMOPT("同步指定会话最后一条消息");
            send_msg = get_sync_last_message_msg();
        break;
        case SYNC_TYPE_MSG_SPECIES_SYNC_ACK:
            MSG_TYPE_PROMOPT("SYNC-ACK");
            send_msg = get_sync_ack_msg();
        break;
    }
    _send_msg_2_fd(send_msg, clientfd);
}