
#ifndef _CONFIG_H_
#define _CONFIG_H_
/*@{*/
/** Defines */

#define DEFAULT_WDCTL_SOCK "/tmp/playerctl.sock"
#define DEFAULT_INTERNAL_SOCK "/tmp/player.sock"

#define DEFAULT_MQTT_PORT        1883
#define DEFAULT_MQTT_SERVER_HOSTNAME  "127.0.0.1"

typedef struct _mqtt_server_t {
	char 	*hostname;
	short	port;

#ifdef SUPPORT_SSL   /* local host.no need */
	char 	*cafile;
	char 	*crtfile;
	char 	*keyfile;
#endif
}t_mqtt_server;


/**
 * Configuration structure
 */
typedef struct {
    char *clientId;                /**< @brief ID of the Gateway, sent to central does */
	t_mqtt_server mqtt_server;
    int debuglevel;
    int daemon;                 /**< @brief if daemon > 0, use daemon mode */
} s_config;

/** @brief Get the current gateway configuration */
s_config *config_get_config(void);

#endif                          /* _CONFIG_H_ */
