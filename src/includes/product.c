#include <stdio.h>
#include "product.h"
#include "interface.h"

struct Product get_product() {
    struct Product product;
    struct Interface interface;
    char *prefix = "BANANAPIMODBUS";
    interface = get_interface();
    printf("%s", interface.mac_address);
    sprintf(product.serial_number, "%s:%s", prefix, interface.mac_address);

    return product;
}