#ifndef PLAYER_H
#define PLAYER_H


enum player_status {
    STOPPED,
    PLAYING,
    PAUSED
};


int    player_getduration(void);    /* duration in seconds */
int    player_getposition(void);    /* position in seconds */
enum player_status    player_getstatus(void);

int player_play(const char *url, int sid, int offset, int speak);

void player_stop(void);

void player_pause(void);
void player_resume(void);

int    player_init(void);
void player_exit(void);

#endif
