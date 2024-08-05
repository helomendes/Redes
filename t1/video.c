#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "video.h"

void is_dir( char *dir )
{
    struct stat dir_stats;
    stat(dir, &dir_stats);
    if (!S_ISDIR(dir_stats.st_mode)) {
        printf("Erro, caminho fornecido nao eh um diretorio\n");
        exit(1);
    }
}

int is_video( char* filename )
{
    int size = 0;

    while(filename[size] != '\0') size++;

    if (size < 5) return 0;

    if ((! strncmp(filename + size - 4, ".avi", 4)) || (! strncmp(filename + size - 4, ".mp4", 4))) {
        return size;
    }
    return 0;
}

void preprocess_video_path( char *videos_dir )
{
    int null_char = 0;
    while (videos_dir[null_char] != '\0') null_char++;
    if (null_char == 0) {
        printf("Erro ao processar caminho para os videos: '%s'\n", videos_dir);
        exit(1);
    }
    if (videos_dir[null_char-1] != '/') {
        videos_dir[null_char] = '/';
        videos_dir[null_char+1] = '\0';
    }
}

void create_video_path( char *videos_dir, char *video_basename, char *video_path )
{
    int null_char = 0;
    while (videos_dir[null_char] != '\0') null_char++;
    if (null_char == 0) {
        printf("Erro ao criar caminho para o video escolhido\n");
        printf("Diretorio dos videos: %s\n", videos_dir);
        printf("Video escolhidos: %s\n", video_basename);
        exit(1);
    }
    strncpy(video_path, videos_dir, null_char);
    strcpy(video_path + null_char, video_basename);
}

void play_video( char *path )
{
    char command[PATH_MAX];
    snprintf(command, PATH_MAX, "xdg-open \"%s\"", path);

    int result = system(command);
    if (result == -1) {
        printf("Erro ao reproduzir o video %s\n", path);
        exit(1);
    }
}