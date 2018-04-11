/**
----------

file: dovepass.c	飛鴿傳書主程式
desc: 設備端主程式, 每次開機後自動啟動本程式

gcc dovepass.c cJSON.c -o dovepass -lm -lcurl

./test2 https://uat-api.sensor.live/api/projects/search

./datas有3個檔
bootstrap.json  local設定檔, 指定最基本的設備型號、版本、連線url及通關密語(token)
connect.json    連線相關資訊
devconfig.json  本設備的各項設定檔

./datas/CA file
1. rootCA.key
2. certificate.key
3. private.key

----------
**/

#include <curl/curl.h>
#include "includes/dovepass.h"
#include "includes/product.h"

int main(int argc, char* argv[]) {
    /*
    struct bootstrap_json bootstrap_go();
    struct connect_info api_boot_version_check(struct bootstrap_json);
    void api_download_file(char*, char*);
    int api_mqtt_connect(struct connect_info);
    void api_mqtt_access();
    */
	struct bootstrap_json bootstraps;
	struct boot_version_check boot_version_check;
	struct connect_info connect_info;
	int mqtt_connect;

    if (argc > 1 && strcmp(argv[1], "-sn") == 0) {
        struct Product product;
        product = get_product();
        printf("Your Serial Number is %s\n", product.serial_number);
        return(0);
    }

	bootstraps = bootstrap_go();

	connect_info = api_boot_version_check(bootstraps);

	// if version update, download bootstrap.bin(OTA)
	api_download_file(boot_version_check.url, BOOTSTRAP_BIN);

	// connect to mqtt
	mqtt_connect = api_mqtt_connect(connect_info);

	// mqtt publish and subscribe
	api_mqtt_access();

    return(0);
}