
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
#define SERVER_IP "192.168.1.100" /* Your Linux server IP */
#define SERVER_PORT 12345
#define USE_TCP 1 /* Set to 0 for UDP */

/* Buffer sizes */
#define RECV_BUF_SIZE 256
#define SEND_BUF_SIZE 256

static void ipv4_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint64_t mgmt_event, struct net_if *iface) {
  if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {

    char addr_str[NET_IPV4_ADDR_LEN];
    for (int i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {

      struct net_if_addr *if_addr = &iface->config.ip.ipv4->unicast[i];
      if (if_addr->addr_type != NET_ADDR_DHCP) {
        continue;
      }
      LOG_INF("IPv4 address obtained: %s",
              net_addr_ntop(AF_INET, &if_addr->address.in_addr, addr_str,
                            sizeof(addr_str)));
      break;
    }
  }
}

static int tcp_echo_client(void) {

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

    snprintf(send_buf, sizeof(send_buf), "Hello from GoodEgg! Message #%d",
             msg_count++);
    ret = send(sock, send_buf, strlen(send_buf), 0);
    if (ret < 0) {

      LOG_ERR("Send failed: %d", errno);
      break;
    }
    LOG_INF("Sent %d bytes", ret);

    /* Receive echo response */
    memset(recv_buf, 0, sizeof(recv_buf));
    ret = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    if (ret < 0) {

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

int main(void) {

  int ret;

  printk("Quectel BG9x Modem Example\n");
  /* Wait for modem to be ready and registered */
  k_sleep(K_SECONDS(10));

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

  ret = tcp_echo_client();
  LOG_INF("Echo client finished with status: %d", ret);
  return 0;
}
