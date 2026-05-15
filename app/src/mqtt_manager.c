
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/posix/arpa/inet.h> /* For inet_pton */
#include <zephyr/posix/unistd.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/posix/sys/eventfd.h>

#define MQTT_BROKER_ADDR    "44.232.241.40" //"broker.emqx.io"
#define MQTT_BROKER_PORT    1883
#define MQTT_CLIENT_ID      "goodEgg_client"
//#define MQTT_PUBLISH_TOPIC  "goodEgg/data"

/* Buffers */
static uint8_t rx_buffer[256];
static uint8_t tx_buffer[256];

/* MQTT client context */
static struct mqtt_client client;

static struct sockaddr_storage broker;


#define FDS_MQTT    0
#define FDS_EVENT   1
#define FDS_COUNT   2

static bool mqtt_connected;

/* Event FD to wake up poll */
static int publish_event_fd;

struct publish_msg {
    char topic[64];
    uint8_t payload[128];
    size_t payload_len;
    uint8_t qos;
    uint16_t msg_id;
};

/* Message queue for publishing */
K_MSGQ_DEFINE(publish_queue, sizeof(struct publish_msg), 10, 4);

/* MQTT Event Handler */
static void mqtt_evt_handler(struct mqtt_client *client, const struct mqtt_evt *evt)
{
    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->result == 0) {
            printk("MQTT: Connected!\n");
            mqtt_connected = true;
        } else {
            printk("MQTT: Connection failed: %d\n", evt->result);
        }
        break;

    case MQTT_EVT_DISCONNECT:
        printk("MQTT: Disconnected: %d\n", evt->result);
        mqtt_connected = false;
        break;

    case MQTT_EVT_PUBACK:
        if (evt->result == 0) {
            printk("MQTT: PUBACK packet id: %u\n", evt->param.puback.message_id);
        }
        break;

    case MQTT_EVT_PUBLISH:
        /* Handle incoming messages */
        {
            const struct mqtt_publish_param *pub = &evt->param.publish;
            printk("MQTT: Received on topic %s\n", pub->message.topic.topic.utf8);
            
            /* Read payload */
            uint8_t payload[128];
            size_t len = MIN(pub->message.payload.len, sizeof(payload));
            mqtt_read_publish_payload(client, payload, len);
        }
        break;

    case MQTT_EVT_SUBACK:
        printk("MQTT: Subscribed, packet id: %u\n", evt->param.suback.message_id);
        break;

    default:
        printk("MQTT: Unhandled event: %d\n", evt->type);
        break;
    }
}
/* Broker address setup */
static int broker_init(void)
{
    struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;
    // struct zsock_addrinfo *result;
    // struct zsock_addrinfo hints = {
    //     .ai_family = AF_INET,
    //     .ai_socktype = SOCK_STREAM,
    // };
    //
    // int err = zsock_getaddrinfo(MQTT_BROKER_ADDR, NULL, &hints, &result);
    // if (err) {
    //     printk("getaddrinfo failed: %d\n", err);
    //     return -ENOENT;
    // }

    zsock_inet_pton(AF_INET, MQTT_BROKER_ADDR, &broker4->sin_addr);
    broker4->sin_family = AF_INET;
    broker4->sin_port = htons(MQTT_BROKER_PORT);
    //broker4->sin_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;

    //zsock_freeaddrinfo(result);
    return 0;
}

/* Initialize MQTT client */
static int mqtt_init(void){

    int err;

    err = broker_init();
    if (err) {
        return err;
    }

    mqtt_client_init(&client);

    /* MQTT client configuration */
    client.broker = &broker;
    client.evt_cb = mqtt_evt_handler;
    
    client.client_id.utf8 = (uint8_t *)MQTT_CLIENT_ID;
    client.client_id.size = strlen(MQTT_CLIENT_ID);
    
    client.password = NULL;
    client.user_name = NULL;
    
    client.protocol_version = MQTT_VERSION_3_1_1;
    client.transport.type = MQTT_TRANSPORT_NON_SECURE;

    /* Buffers */
    client.rx_buf = rx_buffer;
    client.rx_buf_size = sizeof(rx_buffer);
    client.tx_buf = tx_buffer;
    client.tx_buf_size = sizeof(tx_buffer);

    return 0;
}


/* Initialize event fd */
static int init_event_fd(void){

    publish_event_fd = eventfd(0, EFD_NONBLOCK);
    if (publish_event_fd < 0) {
        printk("Failed to create eventfd: %d\n", errno);
        return -errno;
    }
    return 0;
}

/* Signal the MQTT thread that new data is available */
static void signal_publish_ready(void){

    eventfd_t value = 1;
    eventfd_write(publish_event_fd, value);
}

/* Clear the event after processing */
static void clear_publish_event(void){

    eventfd_t value;
    eventfd_read(publish_event_fd, &value);
}

bool mqtt_is_connected() {

  return mqtt_connected;
}

/* Public API: Queue a message for publishing (called from any thread) */
int mqtt_publish_async(const char *topic, const uint8_t *data, size_t len, uint8_t qos){

    struct publish_msg msg = {0};
    int err;

    if (len > sizeof(msg.payload)) {
        return -ENOMEM;
    }

    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    memcpy(msg.payload, data, len);
    msg.payload_len = len;
    msg.qos = qos;
    msg.msg_id = 23;//sys_rand32_get() & 0xFFFF;

    err = k_msgq_put(&publish_queue, &msg, K_NO_WAIT);
    if (err) {
        printk("Publish queue full!\n");
        return err;
    }

    /* Wake up the MQTT thread immediately */
    signal_publish_ready();
    
    return 0;
}

/* Process all pending publish messages */
static void process_publish_queue(void){

    struct publish_msg msg;
    int err;

    while (k_msgq_get(&publish_queue, &msg, K_NO_WAIT) == 0) {
        struct mqtt_publish_param param = {
            .message.topic.qos = msg.qos,
            .message.topic.topic.utf8 = (uint8_t *)msg.topic,
            .message.topic.topic.size = strlen(msg.topic),
            .message.payload.data = msg.payload,
            .message.payload.len = msg.payload_len,
            .message_id = msg.msg_id,
            .dup_flag = 0,
            .retain_flag = 0,
        };

        err = mqtt_publish(&client, &param);
        if (err) {
            printk("Failed to publish: %d\n", err);
            /* Optionally re-queue the message */
        } else {
            printk("Published to %s (msg_id: %u)\n", msg.topic, msg.msg_id);
        }
    }
}

/* MQTT processing thread */
static void mqtt_thread(void *p1, void *p2, void *p3){

    int err;
    struct zsock_pollfd fds[FDS_COUNT];

    /* Initialize event fd once */
    err = init_event_fd();
    if (err) {
        printk("Failed to init eventfd\n");
        return;
    }

    k_sleep(K_SECONDS(10));
    while (1) {
        printk("MQTT: Setting up connection...\n");

        err = mqtt_init();
        if (err) {
            printk("MQTT setup failed: %d\n", err);
            k_sleep(K_SECONDS(5));
            continue;
        }

        err = mqtt_connect(&client);
        if (err) {
            printk("MQTT connect failed: %d\n", err);
            k_sleep(K_SECONDS(5));
            continue;
        }

        /* Setup poll for both MQTT socket and event fd */
        fds[FDS_MQTT].fd = client.transport.tcp.sock;
        fds[FDS_MQTT].events = ZSOCK_POLLIN;

        fds[FDS_EVENT].fd = publish_event_fd;
        fds[FDS_EVENT].events = ZSOCK_POLLIN;

        printk("MQTT: Waiting for CONNACK...\n");

        /* Wait for CONNACK */
        int timeout_count = 0;
        while (!mqtt_connected && timeout_count < 10) {

            fds[FDS_MQTT].revents = 0;
            err = zsock_poll(&fds[FDS_MQTT], 1, 1000);
            if (err > 0 && (fds[FDS_MQTT].revents & ZSOCK_POLLIN)) {
                mqtt_input(&client);
            } else if (err == 0) {
                timeout_count++;
            } else if (err < 0) {
                break;
            }
        }

        if (!mqtt_connected) {
            printk("MQTT: Connection timeout\n");
            mqtt_disconnect(&client, NULL);
            k_sleep(K_SECONDS(5));
            continue;
        }

        printk("MQTT: Entering main loop\n");

        /* Main processing loop */
        while (mqtt_connected) {

            int timeout_ms = mqtt_keepalive_time_left(&client);

            /* Reset revents */
            fds[FDS_MQTT].revents = 0;
            fds[FDS_EVENT].revents = 0;

            /* Poll both socket and eventfd - blocks until either has data or timeout */
            err = zsock_poll(fds, FDS_COUNT, timeout_ms);

            if (err < 0) {
                printk("Poll error: %d\n", errno);
                break;
            }

            /* Handle MQTT socket - incoming data from broker */
            if (fds[FDS_MQTT].revents & ZSOCK_POLLIN) {
                err = mqtt_input(&client);
                if (err) {
                    printk("mqtt_input error: %d\n", err);
                    break;
                }
            }

            /* Handle socket errors */
            if (fds[FDS_MQTT].revents & (ZSOCK_POLLERR | ZSOCK_POLLHUP | ZSOCK_POLLNVAL)) {

                printk("Socket error: revents=0x%x\n", fds[FDS_MQTT].revents);
                break;
            }

            /* Handle publish event - woken up by mqtt_publish_async() */
            if (fds[FDS_EVENT].revents & ZSOCK_POLLIN) {
                clear_publish_event();
                process_publish_queue();
            }

            /* Send keepalive if needed */
            err = mqtt_live(&client);
            if (err && err != -EAGAIN) {
                printk("mqtt_live error: %d\n", err);
                break;
            }
        }

        printk("MQTT: Connection lost, reconnecting...\n");
        mqtt_connected = false;
        mqtt_disconnect(&client, NULL);
        k_sleep(K_SECONDS(5));
    }
}

K_THREAD_DEFINE(mqtt_tid, 4096, mqtt_thread, NULL, NULL, NULL, 7, 0, 0);
