#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <unistd.h> // Lib qui permet d'utiliser la méthode access.

int importe(char *file_PATH, char *archive_PATH);
int extracte(char *archive_PATH, char *archive_file_PATH, char *system_PATH);
int delete_file_archive(char *PATH_archive, char *PATH_archive_file);
