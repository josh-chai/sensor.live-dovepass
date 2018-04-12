/**
----------

file: apis.c
desc: api functions

----------
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "dovepass.h"

// 存入文字檔
int putfile(char *file_path, char *contents) {
	FILE *fp;
	int len;

	fp = fopen(file_path, "wb");
	if(fp == NULL)
   		return 0;

	len = fwrite(contents, 1, strlen(contents), fp);
	fclose(fp);
	return len;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) {
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);

#ifdef DEBUG
    fprintf(stderr, "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb);
#endif

    tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}


// server api query when boot
struct connect_info api_boot_version_check(struct bootstrap_json bootstraps) {
	char versions[10];
	cJSON *json, *jdata;

    CURL *curl;
    struct connect_info connect_info;
    struct curl_slist *list = NULL;

    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    struct url_data data;
    data.size = 0;
    data.data = malloc(4096); /* reasonable size initial buffer */
    if(NULL == data.data) {
        fprintf(stderr, "Failed to allocate memory.\n");
		connect_info.status = 0;
        return connect_info;
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
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			connect_info.res_code = (int) response_code;
			connect_info.status = 1;
			strcpy(connect_info.mac_addr, bootstraps.mac_addr);
			json = cJSON_Parse(data.data);

			if (json == NULL) {
				printf("bootstrap json error !\n");
			    exit (-1);
			}

			jdata = cJSON_GetObjectItem(json, "latest_version");
			if (cJSON_IsNumber(jdata)) {
				connect_info.latest_version = jdata->valueint;
			}

			jdata = cJSON_GetObjectItem(json, "rootca_key");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(connect_info.rootca_key, jdata->valuestring);
            }

			// connect_info.private_key = cJSON_GetObjectItem(json, "private_key")->valuestring;
			jdata = cJSON_GetObjectItem(json, "certificate_key");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(connect_info.certificate_key, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "ca_id");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(connect_info.certificate_id, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "thing_name");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(connect_info.thing_name, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "mqtt_url");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(connect_info.mqtt_url, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "mqtt_port");
			if (cJSON_IsNumber(jdata)) {
				connect_info.mqtt_port = jdata->valueint;
			}

			jdata = cJSON_GetObjectItem(json, "device_config");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(connect_info.device_config, jdata->valuestring);
            }

			strcpy(connect_info.mac_addr, bootstraps.mac_addr);
			sprintf(connect_info.rawdata_mqtt_topic, "$aws/things/%s/shadow/update", connect_info.thing_name);

			// create key & defice config file
			putfile(ROOTCA_KEY, connect_info.rootca_key);
			// putfile(PRIVATE_KEY, connect_info.private_key);
			putfile(CERTIFICATE_KEY, connect_info.certificate_key);

			printf("mqtt_url:%s\nthing_name:%s\n", connect_info.mqtt_url, connect_info.thing_name);
        }

        curl_easy_cleanup(curl);
    }

    return connect_info;
}

// download file
// *ptr = curl_easy_perform(curl), *stream = CURLOPT_WRITEDATA, fp
size_t download_write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void api_download_file(char *url, char *path_file) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(path_file,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    // return 0;
}

// check key file
struct connect_info api_check_key_file(struct boot_version_check res_version_check, struct bootstrap_json bootstraps) {
	char versions[10];
	cJSON *json, *jdata;

    CURL *curl;

    struct connect_info res_connect_info;
    struct curl_slist *list = NULL;

    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    struct url_data data;
    data.size = 0;
    data.data = malloc(4096); /* reasonable size initial buffer */
    if(NULL == data.data) {
        fprintf(stderr, "Failed to allocate memory.\n");
		res_version_check.status = 0;
        return res_connect_info;
    }

    data.data[0] = '\0';

    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
		printf("getkey_url:[%s]\n", res_version_check.getkey_url);

		sprintf(versions, "%d", bootstraps.boot_version);
		// post field value
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "brand", CURLFORM_COPYCONTENTS, bootstraps.boot_brand, CURLFORM_END);
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "type", CURLFORM_COPYCONTENTS, bootstraps.boot_type, CURLFORM_END);
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "version", CURLFORM_COPYCONTENTS, versions, CURLFORM_END);
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "token", CURLFORM_COPYCONTENTS, bootstraps.boot_token, CURLFORM_END);
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "serial_number", CURLFORM_COPYCONTENTS, bootstraps.boot_serialno, CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_URL, res_version_check.getkey_url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);
        // printf("return code: %d\n", res);    // 成功為0

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			res_version_check.res_code = (int) response_code;
			printf("---------------------------\n");

			res_version_check.status = 1;
			json = cJSON_Parse(data.data);
			if (json == NULL) {
                printf("get_key json error !\n");
                exit (-1);
            }

			jdata = cJSON_GetObjectItem(json, "status");
			if (cJSON_IsNumber(jdata)) {
				res_connect_info.status = jdata->valueint;
			}

			jdata = cJSON_GetObjectItem(json, "rootca_key");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.rootca_key, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "private_key");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.private_key, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "certificate_key");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.certificate_key, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "device_config");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.device_config, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "thing_name");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.thing_name, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "mqtt_url");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.mqtt_url, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "mqtt_port");
			if (cJSON_IsNumber(jdata)) {
				res_connect_info.mqtt_port = jdata->valueint;
			}

			// res_connect_info.rawdata_mqtt_topic = cJSON_GetObjectItem(json, "rawdata_mqtt_topic")->valuestring;
			jdata = cJSON_GetObjectItem(json, "ota_mqtt_topic");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.ota_mqtt_topic, jdata->valuestring);
            }

			jdata = cJSON_GetObjectItem(json, "updatedevconfig_mqtt_topic");
            if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
                strcpy(res_connect_info.updatedevconfig_mqtt_topic, jdata->valuestring);
            }

			strcpy(res_connect_info.mac_addr, res_version_check.mac_addr);
			sprintf(res_connect_info.rawdata_mqtt_topic, "$aws/things/%s/shadow/update", res_connect_info.thing_name);

			printf("mqtt_url:%s\n", res_connect_info.mqtt_url);
			printf("mqtt_port:%d\n", res_connect_info.mqtt_port);

			// printf("rootca_key:%s\n", res_connect_info.rootca_key);
			// printf("private_key:%s\n", res_connect_info.private_key);
			// printf("certificate_key:%s\n", res_connect_info.certificate_key);

			printf("device_config:%s\n", res_connect_info.device_config);

			printf("-----------rawdata_mqtt_topic:%s\n", res_connect_info.rawdata_mqtt_topic);
			printf("ota_mqtt_topic:%s\n", res_connect_info.ota_mqtt_topic);
			printf("updatedevconfig_mqtt_topic:%s\n", res_connect_info.updatedevconfig_mqtt_topic);

			/*
			// create key & defice config file
			putfile(ROOTCA_KEY, res_connect_info.rootca_key);
			putfile(PRIVATE_KEY, res_connect_info.private_key);
			putfile(CERTIFICATE_KEY, res_connect_info.certificate_key);
			putfile(DEVICE_CONFIG, res_connect_info.device_config);
            */
			// add put connect file later ???
        }

        curl_easy_cleanup(curl);
    }

	return res_connect_info;
}

// access mqtt
void api_mqtt_access() {


}