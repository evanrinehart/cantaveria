/*
zip
read from a zip file

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.
*/

typedef struct zip_archive zip_archive;
typedef struct zip_file zip_file;
typedef struct zip_dir zip_dir;
typedef unsigned char byte;

typedef struct {
  int (*read)(void* userdata, byte buf[], int count);
  int (*seek)(void* userdata, int offset, int whence);
  void (*close)(void* userdata);
  void* userdata;
} zip_reader;

zip_archive* zip_aropenf(char* filename);
zip_archive* zip_aropen(zip_reader* rd);
void zip_arclose(zip_archive* arc);

zip_file* zip_fopen(zip_archive* arc, char* path);
void zip_fclose(zip_file* f);
int zip_fread(zip_file* f, byte buf[], int count);
int zip_feof(zip_file* f);

zip_dir* zip_opendir(zip_archive* arc, char* path);
char* zip_readdir(zip_dir* dir);
void zip_closedir(zip_dir* dir);

