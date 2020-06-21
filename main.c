#include "hippie.h"
#include "message.h"

pthread_key_t   g_key;
pthread_mutex_t g_mutex;
pthread_cond_t  g_cond;
pthread_mutex_t s_to_c_mutex;
pthread_cond_t  s_to_c_cond;
pthread_attr_t  g_thread_attr;
pthread_mutex_t server_send_mutex;
pthread_cond_t  server_send_cond;

volatile int  tsd_destructor_counter;
volatile bool client_wait = 1;

/* define functions pointer array */
FUNC_POINTER_R_VP_ARG_I DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[9];
extern void client_send_connect_type_msg(int);
extern void client_send_ping_type_msg(int);
extern void client_send_send_type_msg(int);
extern void client_send_push_type_msg(int);
extern void client_send_sync_type_msg(int);
extern void client_send_delete_type_msg(int);
extern void client_send_req_type_msg(int);
extern void client_send_reconnect_type_msg(int);
extern void client_send_disconnect_type_msg(int);

void pthread_once_f(void)
{
    pthread_attr_init(&g_thread_attr);
    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);
    pthread_mutex_init(&s_to_c_mutex, NULL);
    pthread_cond_init(&s_to_c_cond, NULL);
    pthread_mutex_init(&server_send_mutex, NULL);
    pthread_cond_init(&server_send_cond, NULL);
    pthread_attr_setdetachstate(&g_thread_attr, PTHREAD_CREATE_JOINABLE);
    pthread_key_create(&g_key, tsd_destructor);
}

void * client_f(void * arg)
{
    int clientfd;
    int status;
    FUNC_POINTER_R_VP_ARG_I _funcs[9];

    tsd_destructor_counter = 3;

    /* define functions pointer array */
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[0] = client_send_connect_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[1] = client_send_ping_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[2] = client_send_send_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[3] = client_send_push_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[4] = client_send_sync_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[5] = client_send_delete_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[6] = client_send_req_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[7] = client_send_reconnect_type_msg;
    DEFINE_FUNC_POINTER_R_VP_ARG_I_ARRAY(_funcs)[8] = client_send_disconnect_type_msg;

    struct sockaddr_in *serversock = (struct sockaddr_in *)arg;

    clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    status   = connect(clientfd, (struct sockaddr *)serversock, sizeof(struct sockaddr));
    if (status == -1) {
        perror("[ERROR] client socket connect faild\n");
        return NULL;
    }

    tsd_sock_t * client_tsd_sock = pthread_getspecific(g_key); 
    if (client_tsd_sock == NULL) {
        client_tsd_sock = (tsd_sock_t *) malloc(sizeof(tsd_sock_t));
        pthread_setspecific(g_key, (void *)client_tsd_sock);
    }

    client_tsd_sock->thread_id = pthread_self();
    client_tsd_sock->name = "client_thread";
    client_tsd_sock->sockfd = &clientfd;
    client_tsd_sock->sockaddr = serversock;

    struct timespec _time;
    memset(&_time, 0, sizeof(struct timespec));
    _time.tv_sec = time(NULL) + 4;

    for(;;) 
    {
        pthread_mutex_lock(&s_to_c_mutex);
        // pthread_cond_timedwait(&s_to_c_cond, &s_to_c_mutex, &_time);
        if (client_wait) 
            pthread_cond_wait(&s_to_c_cond, &s_to_c_mutex);
        printf(CLIENT_USER_OPTIONS_PROMOPT);
        pthread_mutex_unlock(&s_to_c_mutex);

        client_send_msg_type_ascii_t option = fgetc(stdin);getchar();
        if (option == CLIENT_SEND_MSG_TYPE_ASCII_QUIT) {
            close(clientfd);
            pthread_mutex_lock(&g_mutex);
            pthread_cond_signal(&g_cond);
            pthread_mutex_unlock(&g_mutex);
            printf("[CLIENT] 客户端断开, 线程退出\n");
            pthread_exit(NULL);
        } else if (option > CLIENT_SEND_MSG_TYPE_ASCII_DISCONNECT || option < CLIENT_SEND_MSG_TYPE_ASCII_CONNECT) {
            client_wait = 0;
            printf("[CLIENT] 输入无效\n");
            continue;
        }
        CLIENT_CALL_SEND_FUNC(option,clientfd,client_wait);
    }

    return NULL;
}

void * dispatch_f(void * arg)
{
    int acceptfd = *((int *)arg);
    unsigned char msg[BUFFER_SIZE];
    ssize_t bytes_read;

    tsd_sock_t * dispatch_sock = (tsd_sock_t *) malloc(sizeof(tsd_sock_t));
    pthread_setspecific(g_key, (void *)dispatch_sock);
    dispatch_sock->thread_id = pthread_self();
    dispatch_sock->name = "accept_thread";
    dispatch_sock->sockfd = &acceptfd;
    dispatch_sock->sockaddr = NULL;

    for(;;) 
    {
        memset(msg, '\0', BUFFER_SIZE);
        bytes_read = recv(acceptfd, msg, BUFFER_SIZE, 0);
        if (bytes_read == -1 && errno == EAGAIN) {
            continue;
        } else if (bytes_read > 0) {
            sleep(1);   
            pthread_mutex_lock(&s_to_c_mutex);
            printf("[RECEIVER] 接收到消息, 解析如下\n");
            
            /* parse message */
            _parse_hermes_bytes_(msg);
            printf("\n按Enter键继续\n");getchar();
            pthread_cond_signal(&s_to_c_cond);
            pthread_mutex_unlock(&s_to_c_mutex);
        }
    }

    return NULL;
}

void tsd_destructor(void * val)
{
    if (val != NULL) {
        tsd_sock_t *tsd_sock = (tsd_sock_t *)val;
        free(tsd_sock);
        pthread_mutex_lock(&g_mutex);
        tsd_destructor_counter--;
        if (tsd_destructor_counter <= 0) {
            pthread_key_delete(g_key);
        }
        pthread_mutex_unlock(&g_mutex);
    }
}

int main(int argc, char **argv)
{
    system("clear");
    printf("Hermes-Protocol 解析程序\n\n\n");

    int status;
    int serverfd;
    int clientfd;
    pthread_once_t _once;

    pthread_once(&_once, pthread_once_f);

    struct sockaddr_in serversock;
    memset(&serversock, 0, sizeof(struct sockaddr_in));

    serversock.sin_family = AF_INET;
    serversock.sin_port = htons(USE_PORT);
    serversock.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef SOCK_NONBLOCK
    serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_IP);
#else
    serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    int old_flag = fcntl(serverfd, F_GETFL);
    fcntl(serverfd, F_SETFL, old_flag | O_NONBLOCK);
#endif

    int reuse_port_on = 1;
    int rt = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &reuse_port_on, sizeof(reuse_port_on));

    if (status == -1) {
        perror("[ERROR] socket create faild\n");
    }

    status = bind(serverfd, (struct sockaddr *) &serversock, sizeof(serversock));
    if (status == -1) {
        perror("[ERROR] socket bind faild\n");
        exit(0);
    }
    
    status =  listen(serverfd, SERVER_BACKLOG);
    if (status == -1) {
        perror("[ERROR] socket listen faild\n");
        exit(0);
    }

    tsd_sock_t * main_tsd_sock = (tsd_sock_t *)malloc(sizeof(struct tsd_sock_tag));
    main_tsd_sock->name = "server_thread";
    main_tsd_sock->sockaddr  = &serversock;
    main_tsd_sock->thread_id = pthread_self();
    main_tsd_sock->sockfd    = &serverfd;
    pthread_setspecific(g_key, (void *)main_tsd_sock);

    /* client_thread */
    pthread_t client_thread;
    pthread_create(&client_thread, &g_thread_attr, client_f, (void *)&serversock);

    /* accept */
    int acceptfd;
    int accept_index = 0;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t clinet_sock_size = sizeof(client_addr);

    sleep(1);
    for(;;) 
    {
        acceptfd = accept(serverfd, (struct sockaddr *)&client_addr, &clinet_sock_size);
        if (acceptfd > 0) {
            pthread_t THREAD_ACCEPT_NAME(accept_index);
            pthread_create(THREAD_ACCEPT_NAME_P(accept_index), NULL, dispatch_f, (void *)&acceptfd);
            // accept_index++;

            pthread_mutex_lock(&s_to_c_mutex);
            printf("[SERVER] 新连接 来自 %s:%hu\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
            pthread_cond_signal(&s_to_c_cond);
            pthread_mutex_unlock(&s_to_c_mutex);

            pthread_mutex_lock(&g_mutex);
            pthread_cond_wait(&g_cond, &g_mutex);
            printf("[SERVER] 检测到客户端断开连接, 服务器结束, 主线程退出.\n");
            close(serverfd);
            pthread_mutex_unlock(&g_mutex);

            pthread_cancel(THREAD_ACCEPT_NAME(accept_index));
            pthread_cond_destroy(&g_cond);
            pthread_cond_destroy(&s_to_c_cond);
            pthread_mutex_destroy(&g_mutex);
            pthread_mutex_destroy(&s_to_c_mutex);

            pthread_exit(NULL);
        } else if (acceptfd == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("[ERROR] accept error.\n");
                exit(0);
            }
        }
    }

    pthread_exit(NULL);
    return 0;
}
