#include "ipv4/lwip/ip_addr.h"
#include "ipv4/lwip/ip.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include <mod.h>

static struct netif netif;

void ethernetif_input_dde(void *data, int len) {
    ethernetif_input(&netif, data, len);
}
EXPORT_SYMBOL(ethernetif_input_dde);

uint8_t mac_address[6];
void set_mac_address(uint8_t *mac_addr) {
    memcpy(mac_address, mac_addr, 6);
}
EXPORT_SYMBOL(set_mac_address);


err_t ethernetif_init(struct netif *netif);
err_t ethernet_input(struct pbuf *p, struct netif *netif);

void init_lwip_dev() {
	tcpip_init(0, 0);

    struct ip_addr ipaddr;
    IP4_ADDR(&ipaddr, 10, 0, 2, 15);
    struct ip_addr netmask;
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    struct ip_addr gw;
    IP4_ADDR(&gw, 10, 0, 2, 2);

    netif_add(&netif, &ipaddr, &netmask, &gw, 0, ethernetif_init, ethernet_input);
    netif.hwaddr_len = 6;
    memcpy(netif.hwaddr, mac_address, 6);
    if (!(netif.flags & NETIF_FLAG_UP)) {
        netif.flags |= NETIF_FLAG_UP;
    }

    void httpd_main(int argc, char **argv);
    sys_thread_new("http_server", httpd_main, 0, 0, 0);
}
EXPORT_SYMBOL(init_lwip_dev);
