#include <stdio.h>
#include <string.h>
#include "interface.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

struct Interface get_interface()
{
    struct Interface interface;
    int fd;
    struct ifreq ifr;
    unsigned char *mac = NULL;

    memset(&ifr, 0, sizeof(ifr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

    if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
        mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
        sprintf(interface.mac_address, "%.2X%.2X%.2X%.2X%.2X%.2X" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
          interface.mac_address = '\0';
    }

    return interface;
}