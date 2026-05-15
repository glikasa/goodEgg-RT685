
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/socket.h>
#include <zephyr/posix/arpa/inet.h> /* For inet_pton */
#include <zephyr/posix/unistd.h>
#include <zephyr/net/net_ip.h>
#include <errno.h>
#include <string.h>

LOG_MODULE_REGISTER(echo_client, LOG_LEVEL_DBG);

/* Server Configuration - MODIFY THESE */
#define SERVER_IP "102.37.8.6" //"45.79.112.203"
#define SERVER_HOST "tcpbin.com"
#define SERVER_PORT 8080//4242 
#define USE_TCP 1 /* Set to 0 for UDP */

/* Buffer sizes */
#define RECV_BUF_SIZE 256
#define SEND_BUF_SIZE 256

uint8_t send_buf[SEND_BUF_SIZE];
uint8_t recv_buf[SEND_BUF_SIZE];
int msg_count = 0;

int recv_with_poll(int sock, void *buf, size_t len, int timeout_ms)
{
    int ret;
    int64_t end_time = k_uptime_get() + timeout_ms;

    while (k_uptime_get() < end_time) {
        /* Try recv first */
        ret = zsock_recv(sock, buf, len, 0);
        
        if (ret > 0) {
            return ret;  /* Got data! */
        }
        
        if (ret < 0 && errno != EAGAIN) {
            return ret;  /* Real error */
        }

        /* No data yet, wait a bit */
        k_sleep(K_MSEC(200));
    }

    errno = ETIMEDOUT;
    return -1;
}


int tcp_echo_test(void)
{
    int sock, ret;
    //char buf[128];
    struct sockaddr_in addr;
    //struct zsock_pollfd fds[1];

    /* Create socket */
    sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        LOG_ERR("Failed to create socket: %d", errno);
        return -1;
    }

    /* Setup address */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    zsock_inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    /* Connect */
    ret = zsock_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERR("Connect failed: %d", errno);
        zsock_close(sock);
        return -1;
    }
    LOG_INF("Connected!");


  while (1) {

    snprintf(send_buf, sizeof(send_buf), "Hello from GoodEgg! Message #%d",msg_count++);
    ret = zsock_send(sock, send_buf, strlen(send_buf), 0);
    if (ret < 0) {

        LOG_ERR("Send failed: %d", errno);
        break;
    }
    LOG_INF("Sent %d bytes", ret);

    memset(recv_buf, 0, sizeof(recv_buf));
    ret = recv_with_poll(sock, recv_buf, sizeof(recv_buf) - 1, 10000);
    if (ret > 0) {
        recv_buf[ret] = '\0';
        LOG_INF("Received: %s", recv_buf);
    } else {
        LOG_ERR("Receive failed: %d", errno);
    }

    /* Wait for response using poll */
    // fds[0].fd = sock;
    // fds[0].events = ZSOCK_POLLIN;
    //
    // ret = zsock_poll(fds, 1, 10000);  /* 10 second timeout */
    // if (ret > 0 && (fds[0].revents & ZSOCK_POLLIN)) {
    //     /* Data available */
    //
    //     memset(recv_buf, 0, sizeof(recv_buf));
    //     ret = zsock_recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    //     if (ret > 0) {
    //         recv_buf[ret] = '\0';
    //         LOG_INF("Received: %s", recv_buf);
    //     }
    // } else if (ret == 0) {
    //     LOG_WRN("Timeout waiting for response");
    // } else {
    //     LOG_ERR("Poll error: %d", errno);
    //     break;
    // }
    k_sleep(K_SECONDS(2));
  }
  close(sock);
  return 0;
}




    /* Send data */

int main(void)
{
    // int sock;
    // int ret;
    // struct zsock_addrinfo hints = {0};
    // struct zsock_addrinfo *res;

    printk("Testing echo server: %s:%d\n", SERVER_IP, SERVER_PORT);
    k_sleep(K_SECONDS(20));

    /* Check network interface */
    struct net_if *iface = net_if_get_default();
    if (!iface) {
      printk("No network interface found\n");
      return -1;
    }
    /* Wait for network to be up */
    while (!net_if_is_up(iface)) {

      printk("Waiting for network...\n");
      k_sleep(K_SECONDS(2));
    }
    printk("Network is up!\n");
    tcp_echo_test();
    return 0;








    /* Resolve hostname to IP */
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_STREAM;
    //
    // ret = zsock_getaddrinfo(SERVER_HOST, STRINGIFY(SERVER_PORT), &hints, &res);
    // if (ret != 0) {
    //     printk("DNS lookup failed: %d\n", ret);
    //     return -1;
    // }
    // printk("DNS resolved successfully\n");


    /* Create socket */
//     sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if (sock < 0) {
//         printk("Socket creation failed: %d\n", errno);
//         //zsock_freeaddrinfo(res);
//         return -1;
//     }
//
//
//   struct sockaddr_in server_addr;
//
// //   /* Configure server address */
//   memset(&server_addr, 0, sizeof(server_addr));
//   server_addr.sin_family = AF_INET;
//   server_addr.sin_port = htons(SERVER_PORT);
//
//   ret = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
//   if (ret != 1) {
//     LOG_ERR("Invalid server IP address");
//     close(sock);
//     return -EINVAL;
//   }
//
//     /* Connect using resolved address */
//     ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
//     //ret = connect(sock, res->ai_addr, res->ai_addrlen);
//     //zsock_freeaddrinfo(res);
//
//     if (ret < 0) {
//         printk("Connection failed: %d\n", errno);
//         close(sock);
//         return -1;
//     }
//
//     printk("Connected to %s!\n", SERVER_HOST);
//
//     /* Send test data */
//     const char *test_msg = "Hello from GoodEgg!";
//     ret = send(sock, test_msg, strlen(test_msg), 0);
//     if (ret < 0) {
//         printk("Send failed: %d\n", errno);
//         close(sock);
//         return -1;
//     }
//
//     printk("Sent: %s\n", test_msg);
//
//     /* Receive echo */
//     char buffer[128];
//     ret = recv(sock, buffer, sizeof(buffer) - 1, 0);
//     if (ret > 0) {
//         buffer[ret] = '\0';
//         printk("Received: %s\n", buffer);
//
//         if (strcmp(test_msg, buffer) == 0) {
//             printk("SUCCESS: Echo matched!\n");
//         } else {
//             printk("WARNING: Echo mismatch\n");
//         }
//     } else {
//         printk("Receive failed: %d\n", errno);
//     }
//     close(sock);
//     return 0;
}



// static void ipv4_mgmt_event_handler(struct net_mgmt_event_callback *cb,
//                                     uint64_t mgmt_event, struct net_if *iface) {
//   if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
//
//     char addr_str[NET_IPV4_ADDR_LEN];
//     for (int i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
//
//       struct net_if_addr *if_addr = &iface->config.ip.ipv4->unicast[i];
//       if (if_addr->addr_type != NET_ADDR_DHCP) {
//         continue;
//       }
//       LOG_INF("IPv4 address obtained: %s",
//               net_addr_ntop(AF_INET, &if_addr->address.in_addr, addr_str,
//                             sizeof(addr_str)));
//       break;
//     }
//   }
// }
//
// static int tcp_echo_client(void) {
//
//   int sock;
//   int ret;
//   struct sockaddr_in server_addr;
//   char send_buf[SEND_BUF_SIZE];
//   char recv_buf[RECV_BUF_SIZE];
//   int msg_count = 0;
//
//   /* Create TCP socket */
//   sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//   if (sock < 0) {
//     LOG_ERR("Failed to create TCP socket: %d", errno);
//     return -errno;
//   }
//
//   LOG_INF("TCP socket created: %d", sock);
//
//   /* Configure server address */
//   memset(&server_addr, 0, sizeof(server_addr));
//   server_addr.sin_family = AF_INET;
//   server_addr.sin_port = htons(SERVER_PORT);
//
//   ret = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
//   if (ret != 1) {
//     LOG_ERR("Invalid server IP address");
//     close(sock);
//     return -EINVAL;
//   }
//
//   /* Connect to server */
//   LOG_INF("Connecting to server %s:%d...", SERVER_IP, SERVER_PORT);
//
//   ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
//   if (ret < 0) {
//     LOG_ERR("Failed to connect: %d", errno);
//     close(sock);
//     return -errno;
//   }
//
//   LOG_INF("Connected to server!");
//
//   while (1) {
//
//     snprintf(send_buf, sizeof(send_buf), "Hello from GoodEgg! Message #%d",
//              msg_count++);
//     ret = send(sock, send_buf, strlen(send_buf), 0);
//     if (ret < 0) {
//
//       LOG_ERR("Send failed: %d", errno);
//       break;
//     }
//     LOG_INF("Sent %d bytes", ret);
//
//     /* Receive echo response */
//     memset(recv_buf, 0, sizeof(recv_buf));
//     ret = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
//     if (ret < 0) {
//
//       LOG_ERR("Receive failed: %d", errno);
//       break;
//     } else if (ret == 0) {
//
//       LOG_INF("Server closed connection");
//       break;
//     }
//     recv_buf[ret] = '\0';
//     LOG_INF("Received (%d bytes): %s", ret, recv_buf);
//     k_sleep(K_SECONDS(2));
//   }
//   close(sock);
//   return 0;
// }
//
// int main(void) {
//
//   int ret;
//
//   printk("Quectel BG9x Modem Example\n");
//   /* Wait for modem to be ready and registered */
//   k_sleep(K_SECONDS(10));
//
//   /* Check network interface */
//   struct net_if *iface = net_if_get_default();
//   if (!iface) {
//     printk("No network interface found\n");
//     return -1;
//   }
//   /* Wait for network to be up */
//   while (!net_if_is_up(iface)) {
//
//     printk("Waiting for network...\n");
//     k_sleep(K_SECONDS(2));
//   }
//   printk("Network is up!\n");
//
//   ret = tcp_echo_client();
//   LOG_INF("Echo client finished with status: %d", ret);
//   return 0;
// }
