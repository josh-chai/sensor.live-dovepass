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

    sprintf(temp, "%s:%s", prefix, interface.mac_address);

    dove_encode(temp, "", product.serial_number);

    return product;
}