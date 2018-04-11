struct Interface {
    char mac_address[60];
};

struct Interface get_interface();

void get_mac(char *iface, char *mac_addr);