#ifndef INTERFACE_H
#define INTERFACE_H

struct Interface {
    char mac_address[60];
};

struct Interface get_interface(char *iface);

#endif