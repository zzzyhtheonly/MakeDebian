#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

typedef struct listReg
{	
	regex_t reg;
	regmatch_t pmatch[1];
    size_t nmatch;
	int flag;
	struct listReg *next;
}ListReg;

extern const char *skipFileReg[];
extern const int skipFileReg_len;

extern const char *skipFileText[];
extern const int skipFileText_len;

extern const char *checkFileReg[];
extern const int  checkFileReg_len;

extern const char *checkFileText[];
extern const int checkFileText_len;

extern const char *skipTextReg[];
extern const int skipTextReg_len;

extern const char *skipTextText[];
extern const int skipTextText_len;

extern ListReg *skipFileList;
extern ListReg *checkFileList;
extern ListReg *skipTextList;

ListReg* createListReg(const char *data[], int length);
int compareRegtoText(ListReg *re, char *text);
int compareALLtoText(ListReg *re, const char *textSet[], const int len, char *text);
int compareSubstrtoText(ListReg *re, const char *textSet[], const int len, char *text);

