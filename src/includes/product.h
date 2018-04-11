#ifndef PRODUCT_H
#define PRODUCT_H

struct Product {
    char serial_number[32];
};

struct Product get_product();

#endif