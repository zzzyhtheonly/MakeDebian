#include "rules.h"

const char *skipFileReg[] = 
{
    ".([j|J][p|P][g|G])$",              //.jpg
	".([j|J][p|P][e|E][g|G])$",         //.jpeg
	".([p|P][n|N][g|G])$",              //.png
	".([b|B][m|M][p|P])$",              //.bmp
	".([g|G|t|T][i|I][f|F])$",          //.gif .tif
	".([p|P][c|C][x|X])$",              //.pcx
	".([s|S][v|V][g|G])$",              //.svg
	".([w|W][e|E][b|B][p|P])$",         //.webp

	".([c|C][d|D][a|A])$",              //.cda
	".([w|W][a|A][v|V])$",              //.wav
	".([a|A][i|I][f|F][f|F])$",         //.aiff
	".([a|A][u|U])$",                   //.au
	".([m|M][p|P][3])$",                //.mp3
	".([m|M][i|I][d|D][i|I])$",         //.midi
	".([m|M][i|I][d|D])$",              //.mid
	".([w|W][m|M][a|A])$",              //.wma

	".([d|D][e|E][b|B])$",              //.deb
	".([d|D][d|D][e|E][b|B])$",         //.ddeb
	".([t|T][a|A][r|R])$",              //.tar
	".([x|X][z|Z])$",                   //.xz
	".([g|G][z|Z])$",                   //.gz
	".([d|D][s|S][c|C])$",              //.dsc

	".([s|S][o|O])$"                    //.so
};

const int skipFileReg_len = 23;

const char *skipFileText[] = 
{
    /*"copyright",
	"changelog",
	"control",
	"control.in"*/
};

const int skipFileText_len = /*4*/0;

const char *checkFileReg[] = 
{
};

const int checkFileReg_len = 0;

const char *checkFileText[] = 
{
};

const int checkFileText_len = 0;

const char *skipTextReg[] = 
{
    "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$"                          //email
    //"[a-zA-z]+://[^\\s]*",                                                     //url
    //"[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+\\.?"      //domain name
};

const int skipTextReg_len = 1;

const char *skipTextText[] = 
{
    /*
    "Maintainer",
	"Homepage",
	"Vcs",
	"Uploaders"
	*/
};

const int skipTextText_len = /*4*/0;

ListReg *skipFileList;
ListReg *checkFileList;
ListReg *skipTextList;


ListReg* createListReg(const char *data[], int length)
{
    if(length == 0)
    {
        fprintf(stdout, "Do not need to create reg list since it is empty!\n");
        return NULL;
    }
	
    ListReg *res, *curr;
	res = (ListReg*)malloc(sizeof(ListReg));
	res->flag = REG_EXTENDED;
	res->nmatch = 1;
    regcomp(&res->reg, data[0], res->flag);

	curr = res;
	
	for(int i = 1;i < length;++i)
	{
	    curr->next = (ListReg*)malloc(sizeof(ListReg));
		curr = curr->next;
	    curr->flag = REG_EXTENDED;
	    curr->nmatch = 1;
	    regcomp(&curr->reg, data[i], curr->flag);
	}
	
    return res;
}

int compareRegtoText(ListReg *re, char *text)
{
    ListReg *curr = re;
	
	while(curr)
	{
	    int status = regexec(&curr->reg, text, curr->nmatch, curr->pmatch, 0);
	    if(status == 0)
	    {
		    #if 0
            for(int i = re->pmatch[0].rm_so; i < re->pmatch[0].rm_eo; i++)
		    {
                putchar(text[i]);
            }
		    #endif
		    return 1;
        }

		curr = curr->next;
	}

	return 0;
}

int compareALLtoText(ListReg *re, const char *textSet[], const int len, char *text)
{
    for(int i = 0;i < len;++i)
    {
	    if(strcmp(textSet[i], text) == 0)
		{
		    return 1;
		}
    }
	
    if(compareRegtoText(re, text))
    {
        return 1;
    }

	return 0;
}

int compareSubstrtoText(ListReg *re, const char *textSet[], const int len, char *text)
{
    for(int i = 0;i < len;++i)
    {
	    if(strstr(text, textSet[i]) != NULL)
		{
		    return 1;
		}
    }
	
    if(compareRegtoText(re, text))
    {
        return 1;
    }

	return 0;
}

