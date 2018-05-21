/**
----------

file: bootstrap.c
desc: 讀取./datas/bootstrap.json

gcc edges.c bootstrap.c cJSON.c -o edges -lm -lcurl

----------
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cjson/cJSON.h>
#include "dovepass.h"
#include "product.h"

struct bootstrap_json bootstrap_go() {
    FILE *bootstrap_fp;
    FILE *private_key_fp;
    FILE *csr_key_fp;
    int boot_fd, boot_size;
	int len;
    char *boot_membuf;
	char *boot_brand;
	char *boot_type;
    struct stat boot_buf;
    struct bootstrap_json bootstrap_json;
	char mac_addr[13];
	char device_private_key_file[200];
	char device_csr_key_file[200];
	char buf[100];
	cJSON *json, *jdata;
    struct Product product_info;

    if((bootstrap_fp = fopen(BOOTSTRAP_CONFIG, "r")) != NULL) {
        boot_fd = fileno(bootstrap_fp);
        fstat(boot_fd, &boot_buf);
        boot_size = boot_buf.st_size;
        boot_membuf = malloc(sizeof(char)*boot_size);		// 分配記憶體容納json字串

        fread(boot_membuf, sizeof(char), boot_size, bootstrap_fp);	// 讀入的json置入記憶體內
		json = cJSON_Parse(boot_membuf);
		if (json == NULL) {
		  	printf("bootstrap.json format error !\n");
			exit (-1);
		}

		jdata = cJSON_GetObjectItemCaseSensitive(json, "boot_brand");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.boot_brand, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "boot_type");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.boot_type, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "boot_version");
		if (cJSON_IsNumber(jdata)) {
			bootstrap_json.boot_version = jdata->valueint;
		}

		jdata = cJSON_GetObjectItem(json, "boot_url");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.boot_url, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "boot_token");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.boot_token, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "data_path");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.data_path, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "rootca_key");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.rootca_key, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "certificate_key");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.certificate_key, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "device_private_key");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.device_private_key, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "device_csr_key");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.device_csr_key, jdata->valuestring);
		}

		jdata = cJSON_GetObjectItem(json, "netname");
		if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
			strcpy(bootstrap_json.netname, jdata->valuestring);
		}
       /*
		get_mac(bootstrap_json.netname, bootstrap_json.mac_addr);
		sprintf(bootstrap_json.mac_addr, "0123456");	// for test only
        */
		// serialno = brand + type :mac_addr
		// sprintf(bootstrap_json.boot_serialno, "%s%s:%s", bootstrap_json.boot_brand, bootstrap_json.boot_type, bootstrap_json.mac_addr);

        product_info = get_product();
        strcpy(bootstrap_json.boot_serialno, product_info.serial_number);

		// 檢查private key是否存在, 如不存在, 利用openssl建立之
		sprintf(device_private_key_file, "%s%s", bootstrap_json.data_path, bootstrap_json.device_private_key);
		sprintf(device_csr_key_file, "%s%s", bootstrap_json.data_path, bootstrap_json.device_csr_key);
		if((access(device_private_key_file, F_OK)) == -1)  {
			char cmd_buf[250];
		    sprintf(cmd_buf, "openssl req -nodes -newkey rsa:2048 -keyout %s -out %s -subj '/C=TW/ST=Taiwan/L=Taipei/O=Softchef/OU=IT/CN=sensor.live' 2> /dev/null", device_private_key_file, device_csr_key_file);

		    system(cmd_buf);
		}

		// read private key contents
		if((private_key_fp = fopen(device_private_key_file, "r")) != NULL) {
			while(fgets(buf, 100, private_key_fp) != NULL) {
				len = strlen(buf);
				buf[len - 1] = '\0';
				strcat(bootstrap_json.device_private_key_contents, buf);
				strcat(bootstrap_json.device_private_key_contents, "\\n");
    		}
			fclose(private_key_fp);
		}

        // read csr key contents
        if((csr_key_fp = fopen(device_csr_key_file, "r")) != NULL) {  	// csr key existed, read it
        	while(fgets(buf, 100, csr_key_fp) != NULL) {
				len = strlen(buf);
				buf[len - 1] = '\0';
            	strcat(bootstrap_json.device_csr_key_contents, buf);
            	strcat(bootstrap_json.device_csr_key_contents, "\\n");
			}
        }
        fclose(csr_key_fp);

        free(boot_membuf);
        fclose(bootstrap_fp);
    }
    else {
        printf("bootstrap error !\n");
		bootstrap_json.boot_status = 0;	// failure
    }

	return bootstrap_json;
}