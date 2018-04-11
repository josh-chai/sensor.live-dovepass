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

    strcpy(interface.mac_address, "7831c1bd9b30");

    return interface;
}

void get_mac(char *iface, char *mac_addr)
{
    int fd;
    struct ifreq ifr;
    // char *iface = "enp0s3";
    unsigned char *mac = NULL;
    // static char mac_addr[13] = "\0";

    memset(&ifr, 0, sizeof(ifr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

    if (0 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
        mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

        //display mac address
        sprintf(mac_addr, "%.2X%.2X%.2X%.2X%.2X%.2X" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else {
          *mac_addr = '\0';
    }
}