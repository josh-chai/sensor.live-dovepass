/**
----------

file: get_connect_info.c
    讀取connect_info

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

struct connect_info get_connect_info() {
    struct bootstrap_json bootstrap_go();
    struct bootstrap_json bootstraps;
    struct connect_info connect_info;
    int check_connect_info(struct bootstrap_json);
    FILE *fp;
    int fd, fsize;
    struct stat fbuf;
    char *mbuf;
	cJSON *json, *jdata;

    // 如果connect_info不存在, 新建之
    if((access(CONNECT_INFO, F_OK)) == -1)  {
	    bootstraps = bootstrap_go();
        check_connect_info(bootstraps);
    }

    if((fp = fopen(CONNECT_INFO, "r")) != NULL) {
        fd = fileno(fp);
        fstat(fd, &fbuf);
        fsize = fbuf.st_size;
        mbuf = malloc(sizeof(char)*fsize);		// 分配記憶體容納json字串

        fread(mbuf, sizeof(char), fsize, fp);	// 讀入的json置入記憶體內
		json = cJSON_Parse(mbuf);
        free(mbuf);
		if (json == NULL) {
		  	printf("connect_info.json error !\n");
			return (connect_info);
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

        /*
        jdata = cJSON_GetObjectItem(json, "rawdata_mqtt_topic");
        if (cJSON_IsString(jdata) && (jdata->valuestring != NULL)) {
            strcpy(connect_info.rawdata_mqtt_topic, jdata->valuestring);
        }
        */

        sprintf(connect_info.rawdata_mqtt_topic, "$aws/things/%s/shadow/get/accepted", connect_info.thing_name);
    }

    return connect_info;
}