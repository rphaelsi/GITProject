#include <stdio.h>
#include <zip.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_LINE_SIZE 256

int bruteforce_dico(char *zipname, char *wordlist);
int bruteforce(char *zipname);