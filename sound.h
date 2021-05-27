#ifndef __SOUND_HEADER__
#define __SOUND_HEADER__
#ifdef __cplusplus
extern "C" {
#endif
void Sound_init();
//void Sound_clear();
int Sound_play(const char *data, long length, float vol);
void Sound_exit();
#ifdef __cplusplus
}
#endif
#endif
