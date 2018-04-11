/**
----------

file: dovepass.h
desc: dovepass表頭檔, 定義自身使用的各常數

----------
**/

#ifndef DOVEPASS_H
#define DOVEPASS_H

// config file name, ???
#define DOVEPASS_DATA_PATH "/var/aws/certs/"
#define BOOTSTRAP_CONFIG DOVEPASS_DATA_PATH "bootstrap.json"
#define CONNECT_CONFIG DOVEPASS_DATA_PATH "connections.json"
#define DEVICE_CONFIG DOVEPASS_DATA_PATH "device_config.json"
#define BOOTSTRAP_BIN DOVEPASS_DATA_PATH "dovpass.bin"

// key file path
#define ROOTCA_KEY DOVEPASS_DATA_PATH "rootca.key"
#define CERTIFICATE_KEY DOVEPASS_DATA_PATH "certificate.key"
#define PRIVATE_KEY DOVEPASS_DATA_PATH "device_private.key"

// bootstrap設定檔內容
struct bootstrap_json {
	char	boot_brand[100];
	char	boot_type[100];
	int		boot_version;
	char	boot_url[200];
	char	boot_token[20];
	char	boot_serialno[150];
	char	boot_desc[256];
	char	data_path[120];
	char	rootca_key[100];
	char	certificate_key[100];
	char	device_private_key[100];
	char	device_csr_key[100];
	char 	device_private_key_contents[2048];
	char 	device_csr_key_contents[2048];
	char	netname[20];
	char	mac_addr[20];
	int		boot_status;
};

// 版本檢查資料格式設定
struct boot_version_check {
	int		latest_version;
	char	*url;
	char	*getkey_url;
	char	*mac_addr;
	int		status;
	int		res_code;
};

// 連線資訊設定
struct connect_info {
	int		status;
	int		res_code;
	int		latest_version;
	char	rootca_key[2048];
	char	private_key[2048];
	char	certificate_key[3000];
	char	certificate_id[100];
	char	device_config[4096];
	char	mqtt_url[200];
	int		mqtt_port;
	char	thing_name[200];
	char	rawdata_mqtt_topic[200];
	char	ota_mqtt_topic[200];
	char	updatedevconfig_mqtt_topic[200];
	char	mac_addr[20];
};

// mqtt connect information
struct mqtt_connect {
	char	*mqtt_conn;
};

// define function
int putfile(char *, char *);		// 存入文字檔
struct bootstrap_json bootstrap_go();
struct connect_info api_boot_version_check(struct bootstrap_json);
void api_download_file(char*, char*);
struct connect_info api_check_key_file(struct boot_version_check, struct bootstrap_json);
// struct mqtt_connect api_mqtt_connect(struct connect_info, char *);
int api_mqtt_connect(struct connect_info);
void api_mqtt_access();

void get_mac(char *, char *);

#endif