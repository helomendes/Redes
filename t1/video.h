#ifndef VIDEO_H_
#define VIDEO_H_

void is_dir( char *dir );

int is_video( char *filename );

void preprocess_video_path( char *videos_dir );

void create_video_path( char *videos_dir, char *video_basename, char *video_path );

void play_video( char *path );

#endif