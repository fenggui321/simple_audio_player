
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <mosquitto.h>
#include <json-c/json.h>
#include "mqtt_loop.h"
#include "conf.h"
#include "debug.h"
#include "safe.h"
#include "player.h"

struct mosquitto *g_mosq = NULL;
typedef struct _playRecord{
    int sid;
    int offset;
}playRecord;

static playRecord record;

/* response topic : playerNotify */
static void send_response_offset(struct mosquitto *mosq, char *op, int sid, int result, int offset, const char *msg)
{
    char *res_data = NULL;
    safe_asprintf(&res_data, "{\"sid\":%d,\"op\":\"%s\",\"response\":%d, \"offset\":%d, \"msg\":\"%s\"}",
                               sid, op, result, offset, msg==NULL?"null":msg);

    debug(LOG_DEBUG, "send mqtt msg: topic [%s] msg[%s]\n", "playerNotify", res_data);

    mosquitto_publish(mosq, NULL, "playerNotify", strlen(res_data), res_data, 0, false);

    free(res_data);
}

/* response topic : playerNotify */
static void send_response(struct mosquitto *mosq, char *op, int sid, int result, const char *msg)
{
    char *res_data = NULL;
    safe_asprintf(&res_data, "{\"sid\":%d,\"op\":\"%s\",\"response\":%d, \"msg\":\"%s\"}",
                               sid, op, result, msg==NULL?"null":msg);

    debug(LOG_DEBUG, "send mqtt msg: topic [%s] msg[%s]\n", "playerNotify", res_data);

    mosquitto_publish(mosq, NULL, "playerNotify", strlen(res_data), res_data, 0, false);

    free(res_data);
}

/* topic player/start
 * message: { "sid":int, "op":"start","url":"url","offset":int }
 *
 *response: { "sid":int, "op":"startResult","response":int,"offset":int, "msg":"msg"}
*/
static void start_play(void *mosq, json_object *msg)
{
    int offset;
    const char *url;
    int ret;
    int sid;

    url = json_object_get_string(json_object_object_get(msg, "url"));
    offset = json_object_get_int(json_object_object_get(msg, "offset"));
    sid = json_object_get_int(json_object_object_get(msg, "sid"));
    if (url){
        /* 外部offset 无效 player本地维护上次播放记录 */
        if (record.sid == sid && offset == -1){
            offset = record.offset;
        }
        if (offset == -1){
            offset = 0;
        }
        ret = player_play(url, sid, offset, false);
        if(0 == ret){
            send_response_offset(mosq, "startResult", sid, 0, offset, "start play success");
            return;
        }
    }
    /* notify error */
    debug(LOG_ERR, "PLAY Url:%s failed\n", url);

    send_response_offset(mosq, "startResult", sid, -1, offset, "start play failed");

    return;
}

/* topic player/stop
 * message: { "sid":int, "op":"stop" }
 *
 *response: { "sid":int, "op":"stopResult","response":int, "offset":int,"msg":"msg"}
*/
static void stop_play(void *mosq, json_object *msg)
{
    int sid;

    sid = json_object_get_int(json_object_object_get(msg, "sid"));

    record.sid = sid;
    record.offset = player_getposition();

    player_stop();

    send_response_offset(mosq, "stopResult", sid, 0, 0, "stop play success");
}

/* topic player/pause
 * message: { "sid":int, "op":"pause"}
 *
 *response: { "sid":int, "op":"pauseResult","response":int, "offset":int,"msg":"msg"}
*/
static void pause_play(void *mosq, json_object *msg)
{
    int sid;
    int offset;

    sid = json_object_get_int(json_object_object_get(msg, "sid"));

    offset = player_getposition();

    player_pause();

    send_response_offset(mosq, "pauseResult", sid, 0, offset, "pause play success");
}


/* topic player/resume
 * message: { "sid":int, "op":"resume" }
 *
 *response: { "sid":int, "op":"resumeResult", "response":int, "msg":"msg"}
*/
static void resume_play(void *mosq, json_object *msg)
{
    int sid;

    sid = json_object_get_int(json_object_object_get(msg, "sid"));

    player_resume();

    send_response(mosq, "resumeResult", sid, 0, "resume play success");
}

/* topic player/speak
 * message: { "sid":int, "op":"speak", "url":"url", offset":int }
 *
 *response: { "sid":int, "op":"speakResult", "response":int, "msg":"msg"}
*/
static void speak_play(void *mosq, json_object *msg)
{
    int sid;
    //enum player_status status;
    int offset;
    const char *url;
    int ret;

    /* 需要考虑播放器实现 播放器 pause状态和stop 状态 */
    url = json_object_get_string(json_object_object_get(msg, "url"));
    offset = json_object_get_int(json_object_object_get(msg, "offset"));
    sid = json_object_get_int(json_object_object_get(msg, "sid"));
    if (url){
        /* notify wrong status ? */
        /* player_status();   */
        if (offset == -1){
            offset = 0;
        }
        ret = player_play(url, sid, offset, true);
        if(0 == ret){
            send_response(mosq, "speakResult", sid, 0, "start speak success");
            return;
        }
    }
    /* notify error */
    debug(LOG_ERR, "PLAY speak Url:%s failed\n", url);

    send_response(mosq, "speakResult", sid, -1, "start speak failed");
}

/* topic player/debug
 * message: { "sid":int, "op":"debug", "debuglevel":int}
 *
 *response: { "sid":int, "op":"debugResult", "response":int, "msg":"msg"}
*/
static void internal_debug(void *mosq, json_object *msg)
{
    int sid;
    int debuglevel;
    json_object *obj = json_object_object_get(msg, "debuglevel");

    sid = json_object_get_int(json_object_object_get(msg, "sid"));

    if(obj){
        debuglevel = json_object_get_int(obj);
        setDebug(debuglevel);
        send_response(mosq, "debug", sid, 0, "set debug success");
    }else{
        send_response(mosq, "debugResult", sid, 0, "set debug failed");
    }
}

static struct player_mqtt_op {
    char    *operation;
    void    (*process_mqtt_op)(void *, json_object *);
} mqtt_op[] = {
    {"start", start_play},
    {"stop",  stop_play},
    {"pause", pause_play},
    {"resume",resume_play},
    {"speak",  speak_play},
    {"debug", internal_debug },
    {NULL, NULL}
};

static void process_mqtt_reqeust(struct mosquitto *mosq, const char *data, s_config *config)
{
    json_object *json_request = json_tokener_parse(data);
    if (is_error(json_request)) {
        debug(LOG_INFO, "user request is not valid");
        return;
    }

    const char *op = json_object_get_string(json_object_object_get(json_request, "op"));
    if (!op) {
        debug(LOG_INFO, "No op item get");
        return;
    }

    int i = 0;
    for (; mqtt_op[i].operation != NULL; i++) {
        if (strcmp(op, mqtt_op[i].operation) == 0 && mqtt_op[i].process_mqtt_op) {
            mqtt_op[i].process_mqtt_op(mosq, json_request);
            break;
        }
    }

    json_object_put(json_request);
}

static void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    s_config *config = obj;

    if (message->payloadlen) {
        debug(LOG_DEBUG, "topic is %s, data is %s", message->topic, message->payload);
        process_mqtt_reqeust(mosq, message->payload, config);
    }
}

static void connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    if(rc){
        /* error happen */
        debug(LOG_ERR, "error: connect failed notify.");
        return;
    }
    mosquitto_subscribe(mosq, NULL, "player", 0);
    debug(LOG_INFO, "connect mosq success,sub player topic.");
}

/* topic playerNotify
 *response: { "sid":int, "op":"SpeakEnd", "msg":"msg"}
*/
int mqtt_notify_speakFinished(int sid)
{
    char *res_data = NULL;
    int payloadlen;

    payloadlen = safe_asprintf(&res_data, "{\"sid\":%d,\"op\":\"speakEnd\", \"msg\":\"%s\"}",
                               sid,"play speak voice end");

    debug(LOG_DEBUG, "send mqtt msg: topic [%s] msg[%s]\n", "playerNotify", res_data);
    mosquitto_publish(g_mosq, NULL,"playerNotify", payloadlen, res_data, 0, 0);

    free(res_data);

    return 0;
}

/* topic playerNotify
 *
 *response: { "sid":int, "op":"PlayEnd", "msg":"msg"}
*/
int mqtt_notify_playItemFinish(int sid)
{
    char *res_data = NULL;
    int payloadlen;

    payloadlen = safe_asprintf(&res_data, "{\"sid\":%d,\"op\":\"PlayEnd\", \"msg\":\"%s\"}",
                               sid, "play item end");

    debug(LOG_DEBUG, "send mqtt msg: topic [%s] msg[%s]\n", "playerNotify", res_data);
    mosquitto_publish(g_mosq, NULL,"playerNotify", payloadlen, res_data, 0, 0);

    free(res_data);
    return 0;
}

void main_mqtt_init(s_config *config)
{
    int major = 0, minor = 0, revision = 0;

    char *host     = config->mqtt_server.hostname;
    int  port     = config->mqtt_server.port;
    int  keepalive = 60;
    int  retval = 0;

    mosquitto_lib_version(&major, &minor, &revision);
    debug(LOG_DEBUG, "Mosquitto library version : %d.%d.%d\n", major, minor, revision);

    memset(&record, 0, sizeof(record));
    /* Init mosquitto library */
    mosquitto_lib_init();

     /* Create a new mosquitto client instance */
    g_mosq = mosquitto_new("miplayer", true, config);
    if (g_mosq == NULL) {
        switch(errno){
            case ENOMEM:
                debug(LOG_INFO, "Error: Out of memory.\n");
                break;
            case EINVAL:
                debug(LOG_INFO, "Error: Invalid id and/or clean_session.\n");
                break;
        }
        mosquitto_lib_cleanup();
        return ;
    }

       // Attention! this setting is not secure, but can simplify our deploy
    if (mosquitto_tls_insecure_set(g_mosq, false)) {
        debug(LOG_INFO, "Error : Problem setting TLS insecure option");
        mosquitto_destroy(g_mosq);
        mosquitto_lib_cleanup();
        return ;
    }

    mosquitto_connect_callback_set(g_mosq, connect_callback);
    mosquitto_message_callback_set(g_mosq, message_callback);

       switch( retval = mosquitto_connect(g_mosq, host, port, keepalive) ) {
        case MOSQ_ERR_INVAL:
            debug(LOG_INFO, "Error : %s\n", mosquitto_strerror(retval));
            break;
        case MOSQ_ERR_ERRNO:
            debug(LOG_INFO, "Error : %s\n", strerror(errno));
            break;

        mosquitto_destroy(g_mosq);
        mosquitto_lib_cleanup();
        return ;
    }

    retval = mosquitto_loop_forever(g_mosq, -1, 1);

    mosquitto_destroy(g_mosq);
    mosquitto_lib_cleanup();
}

void main_mqtt_exit(void)
{
    mosquitto_destroy(g_mosq);
    mosquitto_lib_cleanup();
}
