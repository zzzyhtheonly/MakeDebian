typedef struct
{
    int argc;
    char *argv[0];
}argforthread;

void mvCompletedDebs(char *name, char *target);
void compileDsc(char *dir);
void compileSingleSource(char *dir);
void* processDebianSource(void *arg);
char* changeLine(char *line, char *oldText[], char *newText[], int length);
void parseTextArgs(char *args[], int length, char *oldText[], char *newText[]);
char* pwd();
void init();
void delete();