#include "hippie.h"

void _parse_hermes_bytes_(unsigned char * msg)
{
    unsigned char fixed_byte = *msg;

    unsigned char msg_type = (fixed_byte >> 4) & 0xf;
    char type_string[16];
    get_type_string(type_string, msg_type);
    printf("消息类型: %s\n", type_string);

    PARSE_ANY_MESSAGE_FLAG(fixed_byte)
    PARSE_ANY_MESSAGE_FLAG_PRINT

    unsigned char version_type = msg[1];
    if (version_type == 0x00) {
        return;
    }
    printf("协议版本号: %d\n", version_type);

    if (has_payload) {
        int payload_length;
        _parse_4bytes_2_int(msg + 2, &payload_length);
        printf("消息体长度: %u\n\n", payload_length);

        switch (msg_type) {
            case CLIENT_SEND_MSG_TYPE_INT_CONNECT:_parse_connect_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_PING:_parse_ping_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_SEND:_parse_send_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_PUSH:_parse_push_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_DELETE:_parse_delete_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_REQ:_parse_req_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_DISCONNECT:_parse_disconnect_msg_payload(msg + 6, is_ack_type);break;
            case CLIENT_SEND_MSG_TYPE_INT_SYNC:_parse_sync_msg_payload(msg + 6, is_ack_type);break;
        }
    }
}

void _parse_connect_msg_payload(unsigned char * msg, bool is_ack_type)
{
    // uid + token
    int uid;
    _parse_4bytes_2_int(msg, &uid);
    printf("uid: %u\n", uid);
    printf("token: %s\n", msg + 4);
}

void _parse_ping_msg_payload(unsigned char * msg, bool is_ack_type)
{
    //  interval
    short int heart_interval;
    _parse_2bytes_2_short(msg, &heart_interval);
    printf("心跳间隔: %d\n", heart_interval);
}

void _parse_send_msg_payload(unsigned char * msg, bool is_ack_type)
{
    unsigned char msg_flag = *msg;

    PUSH_SEND_MESSGAG_FLAG(msg_flag)
    PUSH_SEND_MESSAGE_FLAG_PRINT

    // send ack
    if (is_ack_type) {
        int local_id,msg_timestamp;
        unsigned char ack_errno = msg[0];
        
        _parse_4bytes_2_int(msg + 1, &local_id);
        _parse_4bytes_2_int(msg + 5, &msg_timestamp);

        printf("local ID: %d\n", local_id);
        printf("错误码: %d (%s)\n", ack_errno, _get_send_error(ack_errno));
        printf("消息服务端时间戳: %d\n", msg_timestamp);

        if (ack_errno == 0) {
            int message_id;
            _parse_4bytes_2_int(msg + 9, &message_id);
            printf("message ID: %d\n", message_id);
        }
        return;
    }

    /* session */
    PUSH_SEND_MESSAGE_SESSION_TYPE_PRINT(1)
    PUSH_SEND_MESSAGE_SESSION_ID_PRINT(2)

    /* user message */
    if (is_user_msg) {
        int local_id;
        _parse_4bytes_2_int(msg + 6, &local_id);
        printf("local ID: %u\n", local_id);
        printf("消息体: %s\n", msg + 10);
    }

    // 接受回执 or 已读回执
    if (is_ack_type || is_read) {
        long ack_msg_id;
        _parse_8bytes_2_long(msg + 6, &ack_msg_id);
        printf("回执消息ID: %lu\n", ack_msg_id);
    }
}

void _parse_push_msg_payload(unsigned char * msg, bool is_ack_type)
{
    unsigned char msg_flag = *msg;

    PUSH_SEND_MESSGAG_FLAG(msg_flag)
    PUSH_SEND_MESSAGE_FLAG_PRINT

    /* session */
    PUSH_SEND_MESSAGE_SESSION_TYPE_PRINT(1)
    PUSH_SEND_MESSAGE_SESSION_ID_PRINT(2)

    /* message id */
    long msg_id;
    _parse_8bytes_2_long(msg + 6, &msg_id);
    printf("消息ID: %lu\n", msg_id);

    /* 用户消息 */
    if (is_user_msg) {
        /* message timestamp */
        int msg_timestamp;
        _parse_4bytes_2_int(msg + 14, &msg_timestamp);
        printf("消息时间戳: %u\n", msg_timestamp);

        /* message distance */
        int distance;
        _parse_4bytes_2_int(msg + 18, &distance);
        printf("距离: %u\n", distance);

        /* message body */
        printf("消息体: %s\n", msg + 20);
    }
}

void _parse_delete_msg_payload(unsigned char *msg, bool is_ack_type)
{
    unsigned char delete_flag = *msg;
    // local id
    int local_id;
    _parse_4bytes_2_int(msg + 1, &local_id);
    printf("local ID: %u\n", local_id);

    if (is_ack_type) {
        if (delete_flag > 0) {
            printf("错误码: %u (%s)\n", delete_flag, _get_delete_error(delete_flag));
        } else {
            printf("删除成功\n");
        }
        return;
    }

    unsigned char delete_type = (delete_flag >> 4) & 0x0F;
    bool is_delete_completely = ((delete_flag & 0x0F) & MASK_DELETE_TYPE_IS_COMPLETELY) == MASK_DELETE_TYPE_IS_COMPLETELY;
    printf("删除类型: %s\n", get_delete_type_string(delete_type));
    printf("是否彻底删除消息: %u\n", is_delete_completely);

    if (delete_type == 0x2 || delete_type == 0x3) {
        PUSH_SEND_MESSAGE_SESSION_TYPE_PRINT(5)
        PUSH_SEND_MESSAGE_SESSION_ID_PRINT(6)
    }

    if (delete_type == 0x3) {
        long message_id;
        _parse_8bytes_2_long(msg + 10, &message_id);
        printf("message ID: %lu\n", message_id);
    }
}

void _parse_req_msg_payload(unsigned char * msg, bool is_ack_type)
{
    unsigned char flag_byte = *msg;
    flag_byte = MINUSONE(flag_byte);

    if (is_ack_type) {
        if (flag_byte == 0x00) {
            printf("请求成功\n");
        } else {
            printf("错误码: %d\n", flag_byte);
        }
    } else {
        printf("Req type: %u\n", flag_byte);
    }

    int local_id;
    _parse_4bytes_2_int(msg + 1, &local_id);
    printf("local ID: %u\n", local_id);

    printf("msgpack内容: %s\n", msg + 5);
}

void _parse_disconnect_msg_payload(unsigned char *msg, bool is_ack_type)
{
    unsigned char offline_reason = MINUSONE(*msg);
    printf("离线原因: %s\n", _get_offline_reason(offline_reason));
}

void _parse_sync_msg_payload(unsigned char * msg, bool is_ack_type)
{
    unsigned char sync_type = *msg;
    sync_type = MINUSONE(sync_type);

    long start_message_id, end_message_id;
    int local_id, sync_local_id;
    _parse_4bytes_2_int(msg + 1, &local_id);

    if (is_ack_type) {
        _parse_4bytes_2_int(msg + 6, &sync_local_id);
        printf("SYNC状态: %s\n", _get_sync_ack_error(sync_type));
        printf("local ID: %u\n", local_id);
        printf("sync local ID: %u\n\n", sync_local_id);
        printf("消息体: 多个 PUSH 包, 消息如下\n");
        _parse_sync_ack_messages(msg + 10);
        return;
    }

    printf("SYNC类型: %s\n", get_sync_type_string(sync_type));
    printf("local ID: %u\n", local_id);

    if (sync_type == 0x01 || sync_type == 0x02) {
        /* sync local id */
        _parse_4bytes_2_int(msg + 5, &sync_local_id);
        printf("sync local ID: %u\n", sync_local_id);
    } else {
        PUSH_SEND_MESSAGE_SESSION_TYPE_PRINT(5)
        PUSH_SEND_MESSAGE_SESSION_ID_PRINT(6)
        if (sync_type == 0x03) {
            _parse_8bytes_2_long(msg + 10, &start_message_id);
            _parse_8bytes_2_long(msg + 18, &end_message_id);
            printf("同步起始message ID: %lu\n同步结束message ID: %lu\n", start_message_id, end_message_id);
        }
    }
}

void _int_padded_to_charbytes(unsigned char * msg, long num, int bytes_num) 
{   
    int i = 0;
    for(;i < bytes_num - 1;i++) {
        msg++;
    }

    int j = 0;
    unsigned char c;
    for(;j < bytes_num;j++) {
        // + 1？ avoid '\0' byte
        c = ((num >> (j * CHAR_BIT)) & 0xFF) + 1;
        *msg = c;msg--;
    }
}

void _parse_2bytes_2_short(unsigned char * msg, short int * length)
{
    short b,e;
    b = (*msg)-1;msg++;
    e = (*msg)-1;
    // printf("%d %d %d %d\n", b,l,u,e);
    *length = (b << CHAR_BIT) + e;
}

void _parse_4bytes_2_int(unsigned char * msg, int * length)
{
    int b,l,u,e;
    b = (*msg)-1;msg++;
    l = (*msg)-1;msg++;
    u = (*msg)-1;msg++;
    e = (*msg)-1;
    // printf("%d %d %d %d\n", b,l,u,e);
    PARSE_4BYTES_TO_INT(length,b,l,u,e);
}

void _parse_8bytes_2_long(unsigned char * msg, long * length)
{
    long b,l,u,e,d,n,i,c;
    b = (*msg)-1;msg++;
    l = (*msg)-1;msg++;
    u = (*msg)-1;msg++;
    e = (*msg)-1;msg++;
    n = (*msg)-1;msg++;
    i = (*msg)-1;msg++;
    c = (*msg)-1;msg++;
    d = (*msg)-1;msg++;
    // printf("%d %d %d %d %d %d %d %d\n", b,l,u,e,n,i,c,d);
    PARSE_8BYTES_TO_LONG(length,b,l,u,e,n,i,c,d);
}

unsigned char * _set_fixed_bytes_(unsigned char MSG_TYPE_MASK, bool is_need_ack, bool is_ack_type, bool is_sync_type, bool has_payload)
{
    unsigned char *msg = (unsigned char *)malloc(BUFFER_SIZE);
    unsigned char fixed_byte;
    unsigned char proto_version_byte;

    if (is_need_ack)
        fixed_byte |= MASK_IS_NEED_ACK;

    if (is_ack_type)
        fixed_byte |= MASK_IS_ACK_TYPE;

    if (is_sync_type)
        fixed_byte |= MASK_IS_SYNC_TYPE;
    
    if (has_payload)
        fixed_byte |= MASK_HAS_PAYLOAD;

    fixed_byte |= MSG_TYPE_MASK;
    proto_version_byte = HERMES_VERSION;

    msg[0] = fixed_byte;
    msg[1] = proto_version_byte;

    return msg;
}

unsigned char * get_connect_msg(void) 
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_CONNECT_TYPE, 1, 0, 0, 1);

    // heap
    char token[] = "hermes-protocol";

    int payload_length = 4 + strlen(token);
    _int_padded_to_charbytes(msg + 2, payload_length, sizeof(payload_length));

    _int_padded_to_charbytes(msg + 2 + sizeof(payload_length), CONNECT_TYPE_UID, sizeof(int));

    strcpy((char *)(msg + 2 + sizeof(payload_length) + sizeof(int)), token);

    // printf("%d %d %d %d %d %d %d %d %d %d %c %c %c\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[14], msg[15]);
    return msg;
}

unsigned char * get_ping_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_PING_TYPE, 1, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 2, sizeof(int));

    _int_padded_to_charbytes(msg + 2 + sizeof(int), PING_TYPE_HEART_INTERVAL, sizeof(short int));

    return msg;
}

unsigned char * get_send_privchat_msg_user(void) 
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SEND_TYPE, 1, 0, 0, 1);

    int msg_boyd_length = strlen(MESSAGE_PACK_TYPE_STRING) + 10;

    _int_padded_to_charbytes(msg + 2, msg_boyd_length, sizeof(int));

    // 消息标识
    msg[6] = _set_send_push_message_flag(1, 0, 0, 1, 0);

    // 会话
    msg[7] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 8, PRIVATE_CHAT_SEND_UID, sizeof(int));
    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_LOCAL_ID, sizeof(int));
    
    //heap
    strcpy((char *)(msg + 16), MESSAGE_PACK_TYPE_STRING);

    return msg;
}

unsigned char * get_send_privchat_msg_recvack(void) 
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SEND_TYPE, 0, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 14, sizeof(int));

    // 消息标识
    msg[6] = _set_send_push_message_flag(0, 1, 0, 0, 0);

    // 会话
    msg[7] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 8, PRIVATE_CHAT_SEND_UID, sizeof(int));

    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_LOCAL_ID, sizeof(long int));

    return msg;
}

unsigned char * get_send_privchat_msg_readack(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SEND_TYPE, 0, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 14, sizeof(int));

    /* message flag */
    msg[6] = _set_send_push_message_flag(0, 1, 1, 0, 0);

    /* session */
    msg[7] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 8, PRIVATE_CHAT_SEND_UID, sizeof(int));

    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_LOCAL_ID, sizeof(long int));

    return msg;
}

unsigned char * get_send_user_msg_ack_with_errno(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SEND_TYPE, 0, 1, 0, 1);

    _int_padded_to_charbytes(msg + 2, 9, sizeof(int));

    msg[6] = SEND_TYPE_ACK_ERRNO;

    /* local id & timestamp */
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));
    _int_padded_to_charbytes(msg + 11, MESSAGE_TIMESTAMP, sizeof(int));

    return msg;
}

unsigned char * get_push_privchat_msg_user(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_PUSH_TYPE, 0, 0, 0, 1);

    int payload_length = strlen(MESSAGE_PACK_TYPE_STRING) + 22;
    _int_padded_to_charbytes(msg + 2, payload_length, sizeof(int));

    // 消息标识
    msg[6] = _set_send_push_message_flag(0, 0, 0, 1, 0);   

    // 会话
    msg[7] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 8, PRIVATE_CHAT_SEND_UID, sizeof(int));

    // 消息ID
    _int_padded_to_charbytes(msg + 12, MESSAGE_ID, sizeof(long));

    // 时间戳
    _int_padded_to_charbytes(msg + 20, MESSAGE_TIMESTAMP, sizeof(int));

    // 距离
    _int_padded_to_charbytes(msg + 24, MESSAGE_DISTANCE, sizeof(int));

    // 消息体
    strcpy((char *)(msg + 28), MESSAGE_PACK_TYPE_STRING);

    return msg; 
}

unsigned char * get_push_privchat_msg_readack(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_PUSH_TYPE, 0, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 14, sizeof(int));

    // 消息标识
    msg[6] = _set_send_push_message_flag(0, 1, 1, 0, 0);

    // 会话
    msg[7] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 8, PRIVATE_CHAT_SEND_UID, sizeof(int));

    // 消息ID
    _int_padded_to_charbytes(msg + 12, MESSAGE_ID, sizeof(long));

    return msg;
}

unsigned char * get_delete_all_sessions_type_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_DELETE_TYPE, 1, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 5, sizeof(int));

    // delete flag
    msg[6] = _set_message_delete_flag(MASK_DELETE_TYPE_ALL_SESSION, 0);

    // local id
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));

    return msg;
}

unsigned char * get_delete_single_session_type_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_DELETE_TYPE, 1, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 10, sizeof(int));

    /* delete flag */
    msg[6] = _set_message_delete_flag(MASK_DELETE_TYPE_SINGLE_SESSION, 1);

    /* local id */
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));

    /* session */
    msg[11] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_SEND_UID, sizeof(int));

    return msg;
}

unsigned char * get_delete_single_message_type_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_DELETE_TYPE, 1, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 10, sizeof(int));

    /* delete flag */
    msg[6] = _set_message_delete_flag(MASK_DELETE_TYPE_SINGLE_MESSAGE, 0);

    /* local id */
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));

    /* session */
    msg[11] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_SEND_UID, sizeof(int));

    /* message id */
    _int_padded_to_charbytes(msg + 16, MESSAGE_ID, sizeof(long));

    return msg;
}

unsigned char * get_delete_ack_type_msg(void) 
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_DELETE_TYPE, 0, 1, 0, 1);

    _int_padded_to_charbytes(msg + 2, 5, sizeof(int));

    /* errno */
    msg[6] = DELETE_TYPE_ACK_ERRNO;

    /* local id */
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));

    return msg;
}

unsigned char * get_req_sessions_info_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_REQ_TYPE, 1, 0, 0, 1);

    int payload_length = 5 + strlen(REQ_TYPE_MESSAGE_PACK_STRING);
    _int_padded_to_charbytes(msg + 2, payload_length, sizeof(int));

    msg[6] = PLUSONE(0x01);

    /* local id */
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));

    strcpy((char *)(msg + 11), REQ_TYPE_MESSAGE_PACK_STRING);

    return msg;
}

unsigned char * get_req_session_info_ack_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_REQ_TYPE, 0, 1, 0, 1);

    int payload_length = 5 + strlen(REQ_TYPE_MESSAGE_PACK_ACK_STRING);
    _int_padded_to_charbytes(msg + 2, payload_length, sizeof(int));

    msg[6] = PLUSONE(0x00);

    /* local id */
    _int_padded_to_charbytes(msg + 7, MESSAGE_LOCAL_ID, sizeof(int));

    strcpy((char *)(msg + 11), REQ_TYPE_MESSAGE_PACK_ACK_STRING);

    return msg;
}

unsigned char * get_disconnect_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_DISCONNECT_TYPE, 0, 0, 0, 1);

    _int_padded_to_charbytes(msg + 2, 1, sizeof(int));

    msg[6] = PLUSONE(0x04);

    return msg;
}

unsigned char * get_sync_new_message_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SYNC_TYPE, 1, 0, 1, 1);

    _int_padded_to_charbytes(msg + 2, 9, sizeof(int));

    msg[6] = PLUSONE(0x02);

    _int_padded_to_charbytes(msg + 7,  MESSAGE_LOCAL_ID, sizeof(int));
    _int_padded_to_charbytes(msg + 11, MESSAGE_SYNC_LOCAL_ID, sizeof(int));

    return msg;
}

unsigned char * get_sync_single_session_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SYNC_TYPE, 1, 0, 1, 1);

    _int_padded_to_charbytes(msg + 2, 26, sizeof(int));

    msg[6] = PLUSONE(0x03);

    _int_padded_to_charbytes(msg + 7,  MESSAGE_LOCAL_ID, sizeof(int));

    msg[11] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_SEND_UID, sizeof(int));

    _int_padded_to_charbytes(msg + 16, SYNC_SINGLE_SESSION_START_MESSAGE_ID, sizeof(long));
    _int_padded_to_charbytes(msg + 24, SYNC_SINGLE_SESSION_END_MESSAGE_ID, sizeof(long));

    return msg;
}

unsigned char * get_sync_last_message_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SYNC_TYPE, 1, 0, 1, 1);

    _int_padded_to_charbytes(msg + 2, 10, sizeof(int));

    msg[6] = PLUSONE(0x04);
    _int_padded_to_charbytes(msg + 7,  MESSAGE_LOCAL_ID, sizeof(int));

    msg[11] = MSG_TYPE_PRIVATE_CHAT;
    _int_padded_to_charbytes(msg + 12, PRIVATE_CHAT_SEND_UID, sizeof(int));

    return msg;
}

unsigned char * get_sync_ack_msg(void)
{
    unsigned char *msg;
    msg = _set_fixed_bytes_(MASK_SYNC_TYPE, 0, 1, 1, 1);

    msg[6] = PLUSONE(0x00);
    _int_padded_to_charbytes(msg + 7,  MESSAGE_LOCAL_ID, sizeof(int));

    msg[11] = PLUSONE(0x01);
    _int_padded_to_charbytes(msg + 12, MESSAGE_SYNC_LOCAL_ID, sizeof(int));

    int sync_message_length = 222;
    _get_sync_ack_messages(msg + 16, &sync_message_length);

    _int_padded_to_charbytes(msg + 2, 10 + sync_message_length, sizeof(int));

    return msg;
}

void _get_sync_ack_messages(unsigned char * msg, int *msg_length)
{
    int index = 0;
    int addr_index = 0;

    for(;index < 8;index++) {
        const unsigned char *push_message = get_push_privchat_msg_user();
        strcpy((char *)(msg + addr_index), (const char *)push_message);
        addr_index += strlen((const char *)push_message);
    }

    *msg_length = addr_index;
}

void _parse_sync_ack_messages(unsigned char *msg)
{
    int push_package_length;
    char push_message[BUFFER_SIZE];
    while (*msg != '\0') {
        _parse_4bytes_2_int(msg + 2, &push_package_length);
        strncpy(push_message, (const char *)msg + 6 + 22, push_package_length - 22);
        printf("%s\n", push_message);
        memset(push_message, 0, BUFFER_SIZE);
        msg = msg + push_package_length + 6;
    }
}

unsigned char _set_send_push_message_flag(bool need_receipts, bool is_receipts, bool is_read, bool is_usermsg, bool is_delete)
{
    unsigned char msg_flag = 0;

    if (need_receipts) {
        msg_flag |= MASK_MSG_FLAG_NEED_RECEIPTS;
    }
    if (is_receipts) {
        msg_flag |= MASK_MSG_FLAG_IS_RECEIPTS;
    }
    if (is_read) {
        msg_flag |= MASK_MSG_FLAG_IS_READ;
    }
    if (is_usermsg) {
        msg_flag |= MASK_MSG_FLAG_IS_USERMSG;
    }
    if (is_delete) {
        msg_flag |= MASK_MSG_FLAG_IS_DELETE;
    }

    return msg_flag;
}

unsigned char _set_message_delete_flag(unsigned char DELETE_MASK, bool is_delete_completely)
{
    unsigned char delete_flag = 0;
    delete_flag |= DELETE_MASK;
    
    if (is_delete_completely) {
        delete_flag |= MASK_DELETE_TYPE_IS_COMPLETELY;
    }

    return delete_flag;
}

// type_string(0x9b) -> 0x2a: strcpy 0x2a C 0x2b O 0x2c N ...
// type_string(0x9b) -> 0x2a:   =    type_string(0x7d, in_func_copy)
void get_type_string(char * type_string, unsigned char msg_type)
{
    switch (msg_type) {
        case CLIENT_SEND_MSG_TYPE_INT_CONNECT:
           strcpy(type_string, "CONNECT");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_PING:
            strcpy(type_string, "PING");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_SEND:
            strcpy(type_string, "SEND");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_PUSH:
            strcpy(type_string, "PUSH");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_SYNC:
            strcpy(type_string, "SYNC");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_DELETE:
            strcpy(type_string, "DELETE");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_REQ:
            strcpy(type_string, "REQ");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_RECONNECT:
            strcpy(type_string, "RECONNECT");
        break;
        case CLIENT_SEND_MSG_TYPE_INT_DISCONNECT:
            strcpy(type_string, "DISCONNECT");
        break;
    }
}

char * get_delete_type_string(unsigned char delete_type)
{
    char *type_string;
    switch(delete_type) {
        case 1:type_string = "删除所有会话";break;
        case 2:type_string = "删除指定会话";break;
        case 3:type_string = "删除单条消息";break;
    }
    return type_string;
}

char * get_sync_type_string(unsigned char sync_type)
{
    char *type_string;
    switch(sync_type) {
        case 1:
        case 2:type_string = "同步新消息";break;
        case 3:type_string = "同步指定会话";break;
        case 4:type_string = "同步最后一条消息";break;
    }
    return type_string;
}

char * _get_send_error(unsigned char ack_errno)
{
    return SEND_TYPE_ACK_ERROR;
}

char * _get_offline_reason(unsigned char offline_no)
{
    return OFFLINE_REASON;
}

char * _get_delete_error(unsigned char ack_errno)
{
    char *ack_error;
    switch(ack_errno)
    {
        case DELETE_ACK_ERRNO_UNSUPPORT_PROTOCOL:
            ack_error = "协议版本不支持";
        break;
        case DELETE_ACK_ERRNO_ILLEGAL_PROTOCOL:
            ack_error = "协议不合法";
        break;
        case DELETE_ACK_ERRNO_NOT_ALLOWED:
            ack_error = "不允许删除会话";
        break;
        case DELETE_ACK_ERRNO_DELETE_FAILD:
            ack_error = "删除失败, 服务器内部错误";
        break;
    }

    return ack_error;
}

char * _get_sync_ack_error(unsigned char ack_errno)
{
    char *ack_error;
    switch(ack_errno)
    {
        case SYNC_ACK_ERRNO_SUCCESS:
            ack_error = "同步成功";
        break;
        case SYNC_ACK_ERRNO_UNSUPPORT_PROTOCOL:
            ack_error = "协议版本不支持";
        break;
        case SYNC_ACK_ERRNO_ILLEGAL_PROTOCOL:
            ack_error = "协议不合法";
        break;
        case SYNC_ACK_ERRNO_EXCEED_LIMIT:
            ack_error = "同步会话超过数量";
        break;
        case SYNC_ACK_ERRNO_SYNC_FAILD:
            ack_error = "同步失败, 服务器内部错误";
        break;
    }

    return ack_error;
}
