#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "func.h"
#include "rules.h"
#include "public.h"

void reverseChangelog();

void mvCompletedDebs(char *name, char *target)
{
    FILE *pp;
	char buf = 0;
	int chars_read = 0;
	pp = popen("ls | grep .deb$ | wc -l", "r");
	if(NULL == pp)
	{
	    fprintf(stderr, "Open pipe failed!\n");
		return;
	}
	
	chars_read = fread(&buf, sizeof(char), 1, pp);
	if(chars_read <= 0)
	{
	    fprintf(stderr, "Read pipe failed!\n");
		pclose(pp);
		return;
	}

    // there is no deb, which means build failed
	if(buf == '0')
	{
	    fprintf(stderr, "Build %s failed!\n", name);
	    
		if(Upchangelog)
		{
		    chdir(name);
		    if(access("debian", 0) == 0)
		    {
		        chdir("debian");
			    reverseChangelog();
			    chdir("../");
		    }
		    else if(access("DEBIAN", 0) == 0)
		    {
		        chdir("DEBIAN");
			    reverseChangelog();
			    chdir("../");
		    }
		    chdir("../");
		}
	
	    char *logname = (char*)malloc(sizeof(char)*strlen(ROOTDIR) + 9);
		memset(logname, 0, sizeof(char)*strlen(ROOTDIR) + 9);
		strcpy(logname, ROOTDIR);
		strcat(logname, "/log/err");

		FILE *fp;
		if(NULL == (fp = fopen(logname, "a+")))
	    {
	        fprintf(stderr, "Writting errlog ERROR : Cannot open errlog file %s\n", logname);
			pclose(pp);
			free(logname);
			return;
		}

		fputs(name, fp);
		fputs("\n", fp);
		fclose(fp);
		free(logname);

		pclose(pp);
		return;
	}
	pclose(pp);

    fprintf(stdout, "Successfully build %s!\n", name);	
    if(NULL == target)
    {
        return;
    }
	
    char command_base[] = "sudo mv -f *.deb *.dsc *.xz *.gz ";
	int len = strlen(target);
	char *command = (char*)malloc(len * sizeof(char) + sizeof(command_base));
	memset(command, 0, len * sizeof(char) + sizeof(command_base));
	strcpy(command, command_base);
	strcat(command, target);
    system(command);
	system("sudo rm -rf *.ddeb");	
	free(command);
}

void compileDsc(char *dir)
{
    fprintf(stdout, "Start building dsc for %s...\n", dir);
	int len = strlen(dir) + 21;
	char *command = (char*)malloc(sizeof(char) * len);
	memset(command, 0, sizeof(char) * len);
    strcpy(command, "sudo dpkg-source -b ");
	strcat(command, dir);
	system(command);
}

void compileSingleSource(char *dir)
{
    char *real_dir = strrchr(dir, '/');
	if(real_dir != NULL)
		dir = real_dir;
	
    fprintf(stdout, "Start building source %s...\n", dir);
 
    char command_base[] = "sudo debuild -b --no-sign ";

    int len = strlen(command_base) + 2*strlen(ROOTDIR) + 2*strlen(dir) + 30;
	char *command = (char*)malloc(sizeof(char) * len);
	memset(command, 0, sizeof(char) * len);

	strcpy(command, command_base);
	strcat(command, "2>");	
    strcat(command, ROOTDIR);
	strcat(command, "/log/");
	strcat(command, dir);
	strcat(command, "_errlog ");
	strcat(command, "1>");
	strcat(command, ROOTDIR);
	strcat(command, "/log/");
	strcat(command, dir);
	strcat(command, "_outlog");

    system(command);

	free(command);
}

char* changeLine(char *line, char *oldText[], char *newText[], int length)
{
    unsigned int NameLen = strlen(line);
	char *res = (char*)malloc(NameLen * sizeof(char) + 1);
	strcpy(res, line);

    for(int i = 0;i < length;++i)
	{
	    char *temp;
		int offset = 0;
		if(newText[i] == NULL || strlen(newText[i]) == 0)
			continue;
	    while(NULL != (temp = strstr(res + offset, oldText[i])))
	    {
			unsigned int beforeLen = NameLen - strlen(temp);
			char before[beforeLen+1];
			memset(before, 0, beforeLen + 1);
			strncpy(before, res, beforeLen);
			
			temp += strlen(oldText[i]);

			unsigned int afterLen = NameLen - strlen(oldText[i]) - beforeLen;
			char after[afterLen+1];
			memset(after, 0, afterLen + 1);
			strcpy(after, temp);
			
			NameLen = beforeLen + strlen(newText[i]) + afterLen;
			offset += (beforeLen + strlen(newText[i]));
			char final[NameLen+1];
			memset(final, 0, NameLen + 1);
			
			strcpy(final, before);
			strcat(final, newText[i]);
			strcat(final, after);

			free(res);
			res = NULL;

			res = (char*)malloc(NameLen * sizeof(char) + 1);
			memset(res, 0, NameLen + 1);
			strcpy(res, final);
			
			temp = NULL;
			if(offset >= NameLen)
			{
			    break;
			}
	    }
		temp = NULL;
	}

	return res;
}

void parseTextArgs(char *args[], int length, char *oldText[], char *newText[])
{
    int i = 0;
	int j = 0;
	
	while(i < length)
	{
	   oldText[j] = args[i++];
	   newText[j++] = args[i++];
	}
}

char* pwd()
{    
    char *res = (char*)malloc(MAX_DIR_LEN*sizeof(char));
	memset(res, 0, MAX_DIR_LEN*sizeof(char));
    if(NULL == getcwd(res, MAX_DIR_LEN*sizeof(char)))
	{
	    fprintf(stderr, "The name of current directory is too long!\n");
		free(res);
		return NULL;
	}

	return res;
}

void clean()
{
    system("sudo rm -rf log/");
}

void init()
{
    clean();
    ROOTDIR = pwd();
	system("sudo mkdir log");
	system("sudo touch log/err");
	
    skipFileList = createListReg(skipFileReg, skipFileReg_len);
	checkFileList = createListReg(checkFileReg, checkFileReg_len);
    skipTextList = createListReg(skipTextReg, skipTextReg_len);
}

void delete()
{
    if(skipFileList)
		free(skipFileList);
	if(checkFileList)
		free(checkFileList);
	if(skipTextList)
		free(skipTextList);
}
