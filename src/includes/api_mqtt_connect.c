/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the aws_iot_config.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "$aws/things/banana-pi-thing/shadow/get/accepted"
 *
 * If all the certs are correct, you should see the messages received by the application in a loop.
 *
 * The application takes in the certificate path, host name , port and the number of times the publish should happen.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <cjson/cJSON.h>

// for local mqtt
#include <mosquitto.h>

// Server connection parameters
#define MQTT_HOSTNAME "localhost"
#define MQTT_PORT 1883
#define MQTT_USERNAME "pi"
#define MQTT_PASSWORD "bananapi"
#define MQTT_TOPIC_DEV2AWS "/jitr/dev/aws"
#define MQTT_TOPIC_AWS2DEV "/jitr/aws/dev"

struct mosquitto *mosq = NULL;
// static char *sub_topic = "$aws/things/cbd062743be8d5f93fe6b6e7ae597114747a5561/shadow/update/accepted";
static char *sub_topic = "$aws/things/cbd062743be8d5f93fe6b6e7ae597114747a5561/shadow/update/delta";
// static char *sub_topic = "$aws/things/cbd062743be8d5f93fe6b6e7ae597114747a5561/shadow/update";

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "dovepass.h"


/**
 * @brief Default cert location
 */
char certDirectory[PATH_MAX + 1] = "/var/aws/certs/";

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

static char oldbuf[255] = "\0";

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
uint32_t publishCount = 0;

char cPayload[1024];
char topic_buf[256];

IoT_Publish_Message_Params paramsQOS0;
IoT_Publish_Message_Params paramsQOS1;

AWS_IoT_Client client;

char *topic_thing_name;
char *topic_thing_type_name;

static char raw_buf_in[1000], buf_in[1000] = "\0", config[1000] = "\0";

void  iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, char *pData) {
	cJSON *json, *jdata;
  printf("######[%s]\n", params->payload);
  json = cJSON_Parse(params->payload);
  if (json == NULL) {
		//   return (void) NULL;
		return (void) NULL;
	}
  json = cJSON_GetObjectItem(json, "state");
  jdata = cJSON_GetObjectItem(json, "device_config");
  //   printf("%s\n", jdata->valuestring);

  if (jdata) {
      strcpy(raw_buf_in, cJSON_Print(jdata));
      strcpy(config, jdata->valuestring);
      printf("+++raw_buf_in:[%s]\n", raw_buf_in);
      printf("+++config:[%s]\n", config);
      printf("+++buf_in:[%s]\n", buf_in);
      if (strcmp(buf_in, config)) {
        strcpy(buf_in, config);
        putfile(DEV_CONFIG_MODBUS, buf_in);
        mosquitto_publish (mosq, NULL, MQTT_TOPIC_AWS2DEV, strlen(buf_in), buf_in, 0, false);

        // disable delta

        sprintf(topic_buf, "$aws/things/%s/shadow/update", topic_thing_name);
        sprintf(cPayload, "{\"state\":{\"reported\":{\"device_config\":%s}}}", raw_buf_in);
        printf("+++before send:[%s]\n", cPayload);
	      paramsQOS0.payload = cPayload;
        paramsQOS0.payloadLen = strlen(cPayload);
        aws_iot_mqtt_publish(&client, topic_buf, strlen(topic_buf), &paramsQOS0);
      }
  }
  else {
    return (void) NULL;
  }
}


void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
	IOT_WARN("MQTT Disconnect");
	IoT_Error_t rc = FAILURE;

	if(NULL == pClient) {
		return;
	}

	IOT_UNUSED(data);

	if(aws_iot_is_autoreconnect_enabled(pClient)) {
		IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	} else {
		IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect(pClient);
		if(NETWORK_RECONNECTED == rc) {
			IOT_WARN("Manual Reconnect Successful");
		} else {
			IOT_WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}


// upload message to aws
upload_aws_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
  int rc;

  sprintf(cPayload, "{\"state\":%s}", (char *)message->payload);
	paramsQOS0.qos = QOS0;
	paramsQOS0.isRetained = 0;
	paramsQOS0.payload = cPayload;
	paramsQOS0.payloadLen = strlen(cPayload);
//   printf("publish=>[%s]\n", cPayload);
   printf("payload=>len[%s][%d]\n", paramsQOS0.payload, paramsQOS0.payloadLen);

  sprintf(topic_buf, "$aws/things/%s/shadow/update", topic_thing_name);
  // rc = aws_iot_mqtt_publish(&client, "$aws/things/cbd062743be8d5f93fe6b6e7ae597114747a5561/shadow/update", 66, &paramsQOS0);
  rc = aws_iot_mqtt_publish(&client, topic_buf, strlen(topic_buf), &paramsQOS0);
  sprintf(topic_buf, "@sensor.live/thing_types/%s/things/%s/history", topic_thing_type_name, topic_thing_type_name);
  rc = aws_iot_mqtt_publish(&client, topic_buf, strlen(topic_buf), &paramsQOS0);
}


// ------------- test mqtt here ------------------------------------------
// int main(int argc, char **argv) {
// int api_mqtt_connect(struct connect_info connect_info) {
int api_mqtt_connect(struct connect_info connect_info) {
	bool infinitePublishFlag = true;

	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];

  topic_thing_name = connect_info.thing_name;
  topic_thing_type_name = connect_info.thing_type_name;

	int32_t i = 0;

	IoT_Error_t rc = FAILURE;

	// AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;


  // ---------- mqtt init ----------
  // Initialize the Mosquitto library
  mosquitto_lib_init();
  mosq = mosquitto_new (NULL, true, NULL);
  if (!mosq)
    {
    fprintf (stderr, "Can't init Mosquitto library\n");
    exit (-1);
    }

  // Set up username and password
  mosquitto_username_pw_set (mosq, MQTT_USERNAME, MQTT_PASSWORD);

  // Establish a connection to the MQTT server. Do not use a keep-alive ping
  int ret = mosquitto_connect (mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
  if (ret)
    {
    fprintf (stderr, "Can't connect to Mosquitto server\n");
    exit (-1);
    }

  // Subscribe to the specified topic. Multiple topics can be
  //  subscribed, but only one is used in this simple example.
  //  Note that we don't specify what to do with the received
  //  messages at this point
  ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC_DEV2AWS, 0);
  if (ret)
    {
    fprintf (stderr, "Can't publish to Mosquitto server\n");
    exit (-1);
    }

  // Specify the function to call when a new message is received
  mosquitto_message_callback_set (mosq, upload_aws_message_callback);


	/*
	getcwd(CurrentWD, sizeof(CurrentWD));
	snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
	snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
	snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);
    */

	sprintf(rootCA, "%s", ROOTCA_KEY);
	sprintf(clientCRT, "%s", CERTIFICATE_KEY);
	sprintf(clientKey, "%s", PRIVATE_KEY);

	printf("###############\n");
	IOT_DEBUG("rootCA %s", rootCA);
	IOT_DEBUG("clientCRT %s", clientCRT);
	IOT_DEBUG("clientKey %s", clientKey);
	printf("###############\n");
	mqttInitParams.enableAutoReconnect = false; // We enable this later below
	// mqttInitParams.pHostURL = HostAddress;
	mqttInitParams.pHostURL = connect_info.mqtt_url;
	// mqttInitParams.port = port;
	mqttInitParams.port = connect_info.mqtt_port;
	mqttInitParams.pRootCALocation = rootCA;
	mqttInitParams.pDeviceCertLocation = clientCRT;
	mqttInitParams.pDevicePrivateKeyLocation = clientKey;
	mqttInitParams.mqttCommandTimeout_ms = 20000;
	mqttInitParams.tlsHandshakeTimeout_ms = 5000;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	rc = aws_iot_mqtt_init(&client, &mqttInitParams);
	if(SUCCESS != rc) {
		IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
		return rc;
	}

	connectParams.keepAliveIntervalInSec = 600;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	// connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
	connectParams.pClientID = connect_info.thing_name;
	// connectParams.pClientID = connect_info.mac_addr;
	// connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
	connectParams.clientIDLen = (uint16_t) strlen(connectParams.pClientID);
	connectParams.isWillMsgPresent = false;

	IOT_INFO("Connecting...");
    while(true) {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if(SUCCESS != rc) {
            IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);

            ClientState mqtt_status;
            mqtt_status = aws_iot_mqtt_get_client_state(&client);
            IOT_INFO("isPingOutstanding: %d", mqtt_status);

            sleep(10);
        }
        else {
            break;
        }

    }
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) {
		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	IOT_INFO("Subscribing...");
  printf("[%s][%d]topic ...\n", sub_topic, strlen(sub_topic));
	// rc = aws_iot_mqtt_subscribe(&client, sub_topic, strlen(sub_topic), QOS0, iot_subscribe_callback_handler, NULL);
	rc = aws_iot_mqtt_subscribe(&client, sub_topic, strlen(sub_topic), QOS0, iot_subscribe_callback_handler, NULL);

	if(SUCCESS != rc) {
		IOT_ERROR("Error subscribing : %d ", rc);
		return rc;
	}
	// printf("rc:%d\n", rc);
	sprintf(cPayload, "{\"state\":{\"reported\":{\"test\":\"use our credentials success!test!\",\"seq\":%d}}}\n", i);

	paramsQOS0.qos = QOS0;
	paramsQOS0.payload = (void *) cPayload;
	paramsQOS0.isRetained = 0;

	paramsQOS1.qos = QOS1;
	paramsQOS1.payload = (void *) cPayload;
	paramsQOS1.isRetained = 0;

	if(publishCount != 0) {
		infinitePublishFlag = false;
	}
  /*
	while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)
		  && (publishCount > 0 || infinitePublishFlag)) {

		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(&client, 100);
		if(NETWORK_ATTEMPTING_RECONNECT == rc) {
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}

		IOT_INFO("-->sleep");
		sleep(10);
		sprintf(cPayload, "{\"state\":{\"reported\":{\"test\":\"1.use our credentials success!test!\",\"seq\":%d}}}\n", i++);
		// sprintf(cPayload, "%s : %d ", "hello from SDK QOS0", i++);
		paramsQOS0.payloadLen = strlen(cPayload);
		// rc = aws_iot_mqtt_publish(&client, "$aws/things/banana-pi-thing/shadow/get/accepted", 47, &paramsQOS0);

		rc = aws_iot_mqtt_publish(&client, connect_info.rawdata_mqtt_topic, strlen(connect_info.rawdata_mqtt_topic), &paramsQOS0);
		if(publishCount > 0) {
			publishCount--;
		}

		if(publishCount == 0 && !infinitePublishFlag) {
			break;
		}

		sprintf(cPayload, "{\"state\":{\"reported\":{\"test\":\"2.use our credentials success!test!\",\"seq\":%d}}}\n", i++);
		paramsQOS1.payloadLen = strlen(cPayload);
		// rc = aws_iot_mqtt_publish(&client, "$aws/things/banana-pi-thing/shadow/get/accepted", 47, &paramsQOS1);

		rc = aws_iot_mqtt_publish(&client, connect_info.rawdata_mqtt_topic, strlen(connect_info.rawdata_mqtt_topic), &paramsQOS0);
		if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
			IOT_WARN("QOS1 publish ack not received.\n");
			rc = SUCCESS;
		}
		if(publishCount > 0) {
			publishCount--;
		}
	}
  */

//   printf("after loop\n");
	while(true) {
		aws_iot_shadow_yield(&client, 500);
    mosquitto_loop (mosq, 500, 1);
	}

	if(SUCCESS != rc) {
		IOT_ERROR("An error occurred in the loop.\n");
	} else {
		IOT_INFO("Publish done\n");
	}

	return rc;
}

