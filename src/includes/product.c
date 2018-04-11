#include <stdio.h>
#include "product.h"
#include "interface.h"

struct Product get_product() {
    struct Product product;

    struct Interface interface;

    char *prefix = "BM";
    char temp[16];
    interface = get_interface();
    
    char *netname = "eth0";
    char mac_address[100];
    get_mac(netname, mac_address);
    printf("%s", mac_address);

    sprintf(temp, "%s:%s", prefix, interface.mac_address);

    dove_encode(temp, "", product.serial_number);

    return product;
}