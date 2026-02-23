#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/dhcpv4.h>

#include <errno.h>
#include <string.h>

LOG_MODULE_REGISTER(echo_client, LOG_LEVEL_DBG);

/* WiFi Configuration - MODIFY THESE */
#define WIFI_SSID       "YourSSID"
#define WIFI_PSK        "YourPassword"

/* Server Configuration - MODIFY THESE */
#define SERVER_IP       "192.168.1.100"  /* Your Linux server IP */
#define SERVER_PORT     12345
#define USE_TCP         1                 /* Set to 0 for UDP */

/* Buffer sizes */
#define RECV_BUF_SIZE   256
#define SEND_BUF_SIZE   256

/* Semaphores for synchronization */
static K_SEM_DEFINE(wifi_connected_sem, 0, 1);
static K_SEM_DEFINE(ipv4_obtained_sem, 0, 1);

/* Network management callback structures */
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

/*============================================================================
 * WiFi Management Event Handlers
 *===========================================================================*/

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint64_t mgmt_event,
                                    struct net_if *iface)
{
    switch (mgmt_event) {
    case NET_EVENT_WIFI_CONNECT_RESULT: {
        const struct wifi_status *status = (const struct wifi_status *)cb->info;
        
        if (status->status == 0) {
            LOG_INF("WiFi connected successfully!");
            k_sem_give(&wifi_connected_sem);
        } else {
            LOG_ERR("WiFi connection failed: %d", status->status);
        }
        break;
    }
    case NET_EVENT_WIFI_DISCONNECT_RESULT: {
        const struct wifi_status *status = (const struct wifi_status *)cb->info;
        LOG_INF("WiFi disconnected: %d", status->status);
        break;
    }
    default:
        break;
    }
}

static void ipv4_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint64_t mgmt_event,
                                    struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {

        char addr_str[NET_IPV4_ADDR_LEN];
        for (int i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {

            struct net_if_addr *if_addr = &iface->config.ip.ipv4->unicast[i];
            if(if_addr->addr_type != NET_ADDR_DHCP) {
                continue;
            }
            LOG_INF("IPv4 address obtained: %s",
                    net_addr_ntop(AF_INET, &if_addr->address.in_addr,
                                  addr_str, sizeof(addr_str)));
            k_sem_give(&ipv4_obtained_sem);
            break;
        }
    }
}

/*============================================================================
 * WiFi Connection Function
 *===========================================================================*/

static int wifi_connect(void){

    struct net_if *iface = net_if_get_default();
    if(!iface) {

        LOG_ERR("No default network interface found");
        return -ENODEV;
    }

    /* Register WiFi management event callbacks */
    net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT |
                                 NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_cb);

    /* Register IPv4 event callbacks */
    net_mgmt_init_event_callback(&ipv4_cb, ipv4_mgmt_event_handler,
                                 NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_cb);

    /* Configure WiFi connection parameters */
    struct wifi_connect_req_params wifi_params = {
        .ssid = (uint8_t *)WIFI_SSID,
        .ssid_length = strlen(WIFI_SSID),
        .psk = (uint8_t *)WIFI_PSK,
        .psk_length = strlen(WIFI_PSK),
        .channel = WIFI_CHANNEL_ANY,
        .security = WIFI_SECURITY_TYPE_PSK,
        .mfp = WIFI_MFP_OPTIONAL,
        .timeout = SYS_FOREVER_MS,
        .band = WIFI_FREQ_BAND_UNKNOWN,
    };

    LOG_INF("Connecting to WiFi SSID: %s", WIFI_SSID);

    int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(wifi_params));
    if(ret) {

        LOG_ERR("WiFi connect request failed: %d", ret);
        return ret;
    }

    /* Wait for WiFi connection */
    LOG_INF("Waiting for WiFi connection...");
    if(k_sem_take(&wifi_connected_sem, K_SECONDS(30)) != 0) {

        LOG_ERR("WiFi connection timeout");
        return -ETIMEDOUT;
    }

    /* Wait for IPv4 address via DHCP */
    LOG_INF("Waiting for IPv4 address...");
    if(k_sem_take(&ipv4_obtained_sem, K_SECONDS(30)) != 0) {

        LOG_ERR("DHCP timeout");
        return -ETIMEDOUT;
    }
    return 0;
}

static int tcp_echo_client(void){

    int sock;
    int ret;
    struct sockaddr_in server_addr;
    char send_buf[SEND_BUF_SIZE];
    char recv_buf[RECV_BUF_SIZE];
    int msg_count = 0;

    /* Create TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        LOG_ERR("Failed to create TCP socket: %d", errno);
        return -errno;
    }

    LOG_INF("TCP socket created: %d", sock);

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    ret = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    if (ret != 1) {
        LOG_ERR("Invalid server IP address");
        close(sock);
        return -EINVAL;
    }

    /* Connect to server */
    LOG_INF("Connecting to server %s:%d...", SERVER_IP, SERVER_PORT);
    
    ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        LOG_ERR("Failed to connect: %d", errno);
        close(sock);
        return -errno;
    }

    LOG_INF("Connected to server!");

    while (1) {

        snprintf(send_buf, sizeof(send_buf), "Hello from GoodEgg! Message #%d", msg_count++);
        ret = send(sock, send_buf, strlen(send_buf), 0);
        if(ret < 0) {

            LOG_ERR("Send failed: %d", errno);
            break;
        }
        LOG_INF("Sent %d bytes", ret);

        /* Receive echo response */
        memset(recv_buf, 0, sizeof(recv_buf));
        ret = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
        if(ret < 0) {

            LOG_ERR("Receive failed: %d", errno);
            break;
        } else if (ret == 0) {

            LOG_INF("Server closed connection");
            break;
        }
        recv_buf[ret] = '\0';
        LOG_INF("Received (%d bytes): %s", ret, recv_buf);
        k_sleep(K_SECONDS(2));
    }
    close(sock);
    return 0;
}

int main(void){

    int ret;
    LOG_INF("Zephyr WiFi Echo Client Starting...");

    /* Wait a bit for system to initialize */
    k_sleep(K_SECONDS(1));
    ret = wifi_connect();
    if(ret != 0) {

        LOG_ERR("Failed to connect to WiFi: %d", ret);
        return ret;
    }
    LOG_INF("WiFi connection established!");
    /* Give network stack time to fully initialize */
    k_sleep(K_SECONDS(2));

    ret = tcp_echo_client();
    LOG_INF("Echo client finished with status: %d", ret);
    return 0;
}
