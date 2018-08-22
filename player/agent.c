#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

/* for strerror() */
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "debug.h"
#include "conf.h"
#include "mqtt_loop.h"
#include "player.h"

extern time_t started_time;

static s_config playerConfig;

static void usage(void)
{
    fprintf(stdout, "Usage: aivs player [options]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "options:\n");
    fprintf(stdout, "  -d <level>    Debug level\n");
    fprintf(stdout, "\n");
}

/** Uses getopt() to parse the command line and set configuration values
 * also populates restartargv
 */
void parse_commandline(int argc, char **argv)
{
    int c;
    int i;

    while (-1 != (c = getopt(argc, argv, "d:m:v:h:"))) {

        switch (c) {

        case 'h':
            usage();
            exit(1);
            break;
#if 0
        case 'd':
            if (optarg) {
                playerConfig.debuglevel = atoi(optarg);
            }
            break;
#endif
        case 'v':
            //fprintf(stdout, "This is WiFiDog version " VERSION "\n");
            exit(1);
            break;

        default:
            usage();
            exit(1);
            break;

        }

    }
}

static void termination_handler(int s)
{
    player_exit();
    main_mqtt_exit();

    debug(LOG_NOTICE, "Exiting...");
    exit(s == 0 ? 1 : 0);
}

static void init_signals(void)
{
    struct sigaction sa;

    debug(LOG_DEBUG, "Initializing signal handlers");

    /* Trap SIGPIPE */
    /* This is done so that when libhttpd does a socket operation on
     * a disconnected socket (i.e.: Broken Pipes) we catch the signal
     * and do nothing. The alternative is to exit. SIGPIPE are harmless
     * if not desirable.
     */
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        debug(LOG_ERR, "sigaction(): %s", strerror(errno));
        exit(1);
    }

    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    /* Trap SIGTERM */
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        debug(LOG_ERR, "sigaction(): %s", strerror(errno));
        exit(1);
    }

    /* Trap SIGQUIT */
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        debug(LOG_ERR, "sigaction(): %s", strerror(errno));
        exit(1);
    }

    /* Trap SIGINT */
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        debug(LOG_ERR, "sigaction(): %s", strerror(errno));
        exit(1);
    }
}


/** Fork and then close any registered fille descriptors.
 * If fork() fails, we die.
 * @return pid_t 0 for child, pid of child for parent
 */
static pid_t safe_fork(void)
{
    pid_t result;
    result = fork();

    if (result == -1) {
        debug(LOG_CRIT, "Failed to fork: %s.  Bailing out", strerror(errno));
        exit(1);
    }
#if 0
    else if (result == 0) {
        /* I'm the child - do some cleanup */
        cleanup_fds();
    }
#endif

    return result;
}

/**@internal
 * Main execution loop
 */
static void main_loop(void)
{
    player_init();

    main_mqtt_init(&playerConfig);

    /* may be never reach.JUST in case */
    player_exit();
    main_mqtt_exit();

    return;
}

static void config_init(void)
{
    playerConfig.daemon = 1;
    playerConfig.debuglevel = 7;
    playerConfig.clientId = strdup(DEFAULT_MQTT_SERVER_HOSTNAME);
    playerConfig.mqtt_server.port = DEFAULT_MQTT_PORT;
    playerConfig.mqtt_server.hostname = strdup(DEFAULT_MQTT_SERVER_HOSTNAME);
}

/** Reads the configuration file and then starts the main loop */
int main(int argc, char **argv)
{
    config_init();

    parse_commandline(argc, argv);

    init_signals();

    if (playerConfig.daemon) {

        switch (safe_fork()) {
        case 0:                /* child */
            setsid();
            main_loop();
            break;

        default:               /* parent */
            exit(0);
            break;
        }
    } else {
        main_loop();
    }

    return (0);                 /* never reached */
}
