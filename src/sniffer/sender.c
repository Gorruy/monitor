/*
* Sniffer is prrogramm that can collect data about incoming udp packages
* Copyright (C) 2024  Vladimir Mimikin
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <unistd.h>

#include "sender.h"
#include "helpers.h"

#define SEND_Q_NAME "/DataQueue" // Queue that will send data to representer
#define RECV_Q_NAME "/NoteQueue" // Queue that will recieve notification from representer


static struct ubus_context *ctx;
struct blob_buf b;

enum {
	HELLO_ID,
	HELLO_MSG,
	HELLO_LAST
};

static const struct blobmsg_policy hello_policy[] = {
	[HELLO_ID] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
	[HELLO_MSG] = { .name = "msg", .type = BLOBMSG_TYPE_STRING },
};

static int test_hello(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[HELLO_LAST];
	char *msgstr = "(unknown)";
	char *strbuf;

	blobmsg_parse(hello_policy, ARRAY_SIZE(hello_policy), tb, blob_data(msg), blob_len(msg));

	if (tb[HELLO_MSG])
		msgstr = blobmsg_data(tb[HELLO_MSG]);

	blob_buf_init(&b, 0);
	strbuf = blobmsg_alloc_string_buffer(&b, "message", 64 + strlen(obj->name) + strlen(msgstr));
	sprintf(strbuf, "%s: Hello, world: %s", obj->name, msgstr);
	blobmsg_add_string_buffer(&b);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

static const struct ubus_method test_methods[] = {
	UBUS_METHOD("hello", test_hello, hello_policy),
};

static struct ubus_object_type test_object_type =
	UBUS_OBJECT_TYPE("test", test_methods);

static struct ubus_object test_object = {
	.name = "test",
	.type = &test_object_type,
	.methods = test_methods,
	.n_methods = ARRAY_SIZE(test_methods),
};

static struct ubus_object test_object2 = {
	.name = "test2",
	.type = &test_object_type,
	.methods = test_methods,
	.n_methods = ARRAY_SIZE(test_methods),
};

void* send_data_to_representer(void* args_struct_ptr)
{
    /* Creates a posix message queue and starts listening for
       signal from representer proccess, after which it sends 
       collected statistics to representer  */

    pid_t ubusd_pid = fork();
    if (!ubusd_pid) {
        !execl( "/usr/bin/sudo", "/usr/bin/sudo", "/usr/local/sbin/ubusd", (char*)NULL );
    }
    else {
       
        const char *ubus_socket = NULL;
        int ret = 0;
        int ch;

        ctx = ubus_connect(ubus_socket);
        if (!ctx) {
            fprintf(stderr, "Failed to connect to ubus\n");
            return -1;
        }

        ret = ubus_add_object(ctx, &test_object);
        if (ret)
            fprintf(stderr, "Failed to add_object object: %s\n", ubus_strerror(ret));

        ret = ubus_add_object(ctx, &test_object2);
        if (ret)
            fprintf(stderr, "Failed to add_object object: %s\n", ubus_strerror(ret));

        uloop_init();
        ubus_add_uloop(ctx);
        uloop_run();
        uloop_done();

        // sender_args_t* args = (sender_args_t*)args_struct_ptr;

        // struct mq_attr notif_attr = {
        //     .mq_maxmsg = 1,
        //     .mq_msgsize = sizeof(size_t)
        // };

        // struct mq_attr data_attr = {
        //     .mq_maxmsg = 1,
        //     .mq_msgsize = sizeof(size_t)*2
        // };

        // mqd_t notif_q = mq_open(RECV_Q_NAME, O_RDONLY | O_CREAT | O_NONBLOCK, 0666, &notif_attr);
        // if ( notif_q == (mqd_t) -1 ) {
        //     THREAD_ERROR_RETURN("Error in queue creation!");
        // }

        // mqd_t data_q = mq_open(SEND_Q_NAME, O_WRONLY | O_CREAT, 0666, &data_attr);
        // if ( data_q == (mqd_t) -1 ) {
        //     THREAD_ERROR_RETURN("Error in queue creation!");
        // }

        // size_t note[2];
        // size_t stats_to_send[2] = {0};
        // int rcv_status;
        // int send_status;

        // while (!break_signal) {
        //     rcv_status = mq_receive(notif_q, (char*)note, sizeof(size_t)*2, NULL);

        //     if ( rcv_status == -1 && errno != EAGAIN) {
        //         THREAD_ERROR_RETURN("Error when receiving message from queue");
        //     }
        //     else if ( rcv_status == -1 ) {
        //         continue;
        //     }        

        //     pthread_mutex_lock(args->pkt_mtx);
        //     stats_to_send[0] = *(args->pkt_num_ptr);
        //     stats_to_send[1] = *(args->pkt_len_ptr);

        //     send_status = mq_send(data_q, (char*)&stats_to_send, sizeof(size_t)*2, 0);

        //     *(args->pkt_num_ptr) = 0;
        //     *(args->pkt_len_ptr) = 0;
        //     pthread_mutex_unlock(args->pkt_mtx);
            
        //     if ( send_status == -1 ){
        //         THREAD_ERROR_RETURN("Error when sending message to queue");
        //     }
        // }

        // mq_unlink(RECV_Q_NAME);
        // mq_unlink(SEND_Q_NAME);


    }
    kill( SIGINT, ubusd_pid );
    ubus_free(ctx);
    uloop_done();

    return (void*) 1;
}

