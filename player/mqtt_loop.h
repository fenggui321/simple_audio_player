
#ifndef	_MQTT_LOOP_H_
#define	_MQTT_LOOP_H_

#include "conf.h"

void main_mqtt_init(s_config *config);
void main_mqtt_exit(void);

int mqtt_notify_speakFinished(int sid);

int mqtt_notify_playItemFinish(int sid);

#endif
