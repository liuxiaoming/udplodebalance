//============================================================================
// Name        : msg.h
// Author      : xiaoming
// Version     :
// Date        : 2013.07.03
// Copyright   : xiaoming_beta@163.com
// Description : the definition of Message struct
//============================================================================

#ifndef MSG_H
#define MSG_H
typedef struct
{
    /* 消息的发送进程是谁，就填谁的id */
    unsigned src_id;

    /* 消息的接收进程是谁，就填谁的id */
    unsigned dst_id;

    /* 发送“时间请求”消息时填写，回复“时间应答”消息时，其值要与请求消息保持一致。 */
    unsigned usr_id;

    /* 消息类型：0, 时间请求；1, 时间答应；2, 心跳请求；3, 心跳应答 */
    unsigned msg_type;

    /* 服务端回复“时间应答”消息时，在data中填入当前时间的字符串，形式如“2013-06-20 13:56:28”即可  */
    char data[32];
} t_msg;
#endif // MSG_H
