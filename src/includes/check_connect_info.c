/**
----------

file: check_connect_info.c
    檢查connect_info是否存在

----------
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "dovepass.h"

extern size_t write_data(void *, size_t, size_t nmemb, struct url_data *);
extern size_t download_write_data(void *, size_t, size_t, FILE*);

int check_connect_info(struct bootstrap_json bootstraps) {
    cJSON *json, *jdata;
    CURL *curl;
    struct connect_info connect_info;
    struct curl_slist *list = NULL;

    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    struct url_data data;
	  char versions[10];
    char *output;
    int *size = 0;

    // 如果connect info 不存在, 向aws連線及讀取
    if((access(CONNECT_INFO, F_OK)) == -1)  {
        data.size = 0;
        data.data = malloc(4096); /* reasonable size initial buffer */
        if(NULL == data.data) {
            fprintf(stderr, "Failed to allocate memory.\n");
            connect_info.status = 0;
            return (-1);
        }

        data.data[0] = '\0';

        CURLcode res;

        curl = curl_easy_init();
        if (curl) {
            printf("url:%s\n", bootstraps.boot_url);
            sprintf(versions, "%d", bootstraps.boot_version);
            curl_easy_setopt(curl, CURLOPT_URL, bootstraps.boot_url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

            // post field value
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "version", CURLFORM_COPYCONTENTS, versions, CURLFORM_END);
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "token", CURLFORM_COPYCONTENTS, bootstraps.boot_token, CURLFORM_END);
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "csr", CURLFORM_COPYCONTENTS, bootstraps.device_csr_key_contents, CURLFORM_END);
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "serial_number", CURLFORM_COPYCONTENTS, bootstraps.boot_serialno, CURLFORM_END);
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

            res = curl_easy_perform(curl);

            if(res != CURLE_OK) {
                fprintf(stderr, "curl fail:%s \n", curl_easy_strerror(res));
                return(-2);
            }
            else {
                long response_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                connect_info.res_code = (int) response_code;
                connect_info.status = 1;
                strcpy(connect_info.mac_addr, bootstraps.mac_addr);
                json = cJSON_Parse(data.data);

                if (json == NULL) {
                    printf("bootstrap.json error !\n");
                    exit (-3);
                }

                jdata = cJSON_GetObjectItem(json, "rootca_key");
                if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                    strcpy(connect_info.rootca_key, jdata->valuestring);
                }

                jdata = cJSON_GetObjectItem(json, "certificate_key");
                if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                    strcpy(connect_info.certificate_key, jdata->valuestring);
                }

                jdata = cJSON_GetObjectItem(json, "device_config");
                output = cJSON_Print(jdata);
                // cJSON *current_element = NULL;
                // char *current_key = NULL;
                // cJSON_ArrayForEach(current_element, jdata) {
                //     current_key = current_element->string;
                //     if (current_key != NULL)
                //     {
                //         printf("%s\n", current_key);
                //     }
                // }

                // create key & defice config file
                putfile(ROOTCA_KEY, connect_info.rootca_key);
                putfile(CERTIFICATE_KEY, connect_info.certificate_key);
                // create key & defice config file
                putfile(ROOTCA_KEY, connect_info.rootca_key);
                putfile(CERTIFICATE_KEY, connect_info.certificate_key);
                putfile(DEV_CONFIG_MODBUS, output);

                putfile(CONNECT_INFO, data.data);
            }

            curl_easy_cleanup(curl);
            return (1);
        }
        else {
            return (2);
        }
    }
}