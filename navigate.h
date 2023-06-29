#include <zip.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h> 
#include <stdlib.h> 

#define PATH_MAX 4096
#define COMMAND_MAX 256
#define MAX_DEPTH 100

int open_zip(const char *zipname, const char *password);
int change_dir(const char *dir);
void list_files();
int extract(const char *filename);
int import(const char *filename);
int touch(const char *filename);
void interactive_session();

