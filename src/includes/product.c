#include <stdio.h>
#include "product.h"
#include "interface.h"

struct Product get_product() {
    struct Product product;

    struct Interface interface;

    char *iface = "eth0";
    char *prefix = "BM";
    char temp[16];

    interface = get_interface(iface);

    sprintf(temp, "%s:%s", prefix, interface.mac_address);

    dove_encode(temp, NULL, product.serial_number);

    return product;
}