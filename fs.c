#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include <time.h>

#include "rules.h"
#include "func.h"
#include "public.h"

#define CHECK_TEXT 1
#define NO_CHECK_TEXT 0

int replaceFileWithoutBak(char *src, char *dest, struct stat statbuf)
{
    if(remove(dest) != 0)
	{
	    fprintf(stderr, "Replacing file ERROR : Cannot replace file %s : cannot remove original file!\n", dest);
		if(remove(src) != 0)
		{
		    fprintf(stderr, "Replacing file ERROR : Also cannot remove target file %s! Please remove it manually!\n", src);
		}
        return 1;
	}

	if(rename(src, dest) == 0)
	{
	    fprintf(stdout, "Successfully replaced %s!\n", dest);
		chmod(dest, statbuf.st_mode);
	}
	else
	{
	    fprintf(stderr, "Replacing file ERROR : function rename occurs an error! Cannot mv src file %s into dest file %s!\nPlease move it manually!\n", src, dest);
		return 1;
	}
	return 0;
}

int replaceFileWithBak(char *src, char *dest, struct stat statbuf)
{
    char *bak = (char*)malloc(strlen(dest)*sizeof(char) + 5);
	memset(bak, 0, strlen(dest)*sizeof(char) + 5);
	strcpy(bak, dest);
	strcat(bak, ".bak");
	
	if(rename(dest, bak) == 0)
	{
		chmod(bak, statbuf.st_mode);
	}
	else
	{
	    fprintf(stderr, "Replacing file ERROR : function rename occurs an error! Cannot create bak file for %s!\nPlease move it manually!\n", dest);
		if(remove(src) != 0)
		{
		    fprintf(stderr, "Replacing file ERROR : Also cannot remove target file %s! Please remove it manually!\n", src);
		}
		free(bak);
		return 1;
	}

	if(rename(src, dest) == 0)
	{
	    fprintf(stdout, "Successfully replaced %s!\n", dest);
		chmod(dest, statbuf.st_mode);
	}
	else
	{
	    fprintf(stderr, "Replacing file ERROR : function rename occurs an error! Cannot mv src file %s into dest file %s!\nPlease move it manually!\n", src, dest);
		free(bak);
		return 1;
	}
	free(bak);
	return 0;
}

void reverseChangelog()
{
    FILE *fp;
    if(NULL == (fp = fopen("changelog", "r")))
	{
	    fprintf(stderr, "Reverse changelog ERROR : Cannot open file changelog!\n");
		return;
	}
    struct stat statbuf;
	stat("changelog", &statbuf);
	fclose(fp);
	
	if(NULL == (fp = fopen("changelog.bak", "r")))
	{
	    fprintf(stderr, "Reverse changelog ERROR : Cannot open file changelog_bak!\n");
		return;
	}
	fclose(fp);
	
    replaceFileWithoutBak("changelog.bak", "changelog", statbuf);
}

void updateChangelog(char *dir)
{
    char filename[] = "changelog";
	char *tempName;
    tempName = (char*)malloc(strlen(filename) * sizeof(char) + 8);
	memset(tempName, 0, strlen(filename) * sizeof(char) + 8);
	strcpy(tempName, filename);
	strcat(tempName, ".XXXXXX");
	strcpy(tempName, mktemp(tempName));

	FILE *fp, *out;
	
	if(NULL == (fp = fopen(filename, "r+")))
	{
	    fprintf(stderr, "Updating changelog ERROR : Cannot open file changelog\n");
		goto ret;
	}
    struct stat statbuf;
	stat(filename, &statbuf);
	
	if(NULL == (out = fopen(tempName, "wb+")))
	{
	    fprintf(stderr, "Updating changelog ERROR : Cannot create temp file %s for changelog\n", tempName);
		fclose(fp);
		if(remove(tempName) != 0)
		{
		    fprintf(stderr, "Updating changelog ERROR : Also cannot remove temp file %s! Please remove it manually!\n", tempName);
		}
        goto ret;
	}

	char *lateset, *latesetS;
	char *old_version, *old_versionS;
	char ch;

	lateset = (char*)malloc(BUFFER_LEN * sizeof(char));
	memset(lateset, 0, BUFFER_LEN);
	latesetS = lateset;

	old_version = (char*)malloc(BUFFER_LEN * sizeof(char));
	memset(old_version, 0, BUFFER_LEN);
	old_versionS = old_version;

    //print last changelog for user
     unsigned int lineLen = 0;
    int isVersion = 0;
    fprintf(stdout, "\n***This is latest version of %s***\n\n", dir);
	while(EOF != (ch = fgetc(fp)))
	{
		*lateset++ = ch;
		++lineLen;
	    if(lineLen >= BUFFER_LEN)
	    {
	    	fclose(fp);
	        fclose(out);
		    free(latesetS);
	        latesetS = NULL;
			lateset = NULL;

			fprintf(stderr, "Updating changelog ERROR :  The buffer exceed 1024! Skipped processing %s!\n", filename);
			if(remove(tempName) != 0)
			{
				fprintf(stderr, "Updating changelog ERROR :  cannot remove temp file %s! Please remove it manually!\n", tempName);
			}
			
	        goto ret;
	    }
		
		if(ch == '(')
		{
		    isVersion = 1;
			continue;
		}
		if(ch == ')')
		{
		    isVersion = 0;
			continue;
		}
		if(isVersion)
		{
		    *old_version++ = ch;
		}
		if(ch == '\n')
		{
		    fprintf(stdout, "%sVersion:%s\n", latesetS, old_versionS);
			fprintf(stdout,  "\n*********************************************************************\n");
			break;
		}
	}

    //enter new version from commandline
	char *new_version;
    new_version = (char*)malloc(BUFFER_LEN * sizeof(char));
	memset(new_version, 0, BUFFER_LEN);
	fprintf(stdout, "Please enter new version:\n");
	fscanf(stdin,"%s", new_version);

	//change the first line into new version, then update changelog in format
	char *new_firstline = changeLine(latesetS, &old_versionS, &new_version, 1);
	char *new_secondline = (char*)malloc(BUFFER_LEN * sizeof(char));
	memset(new_secondline, 0, BUFFER_LEN);
	char *tmp_line = (char*)malloc(BUFFER_LEN * sizeof(char));
	memset(tmp_line, 0, BUFFER_LEN);
	int tmp_len = 0;
	fprintf(stdout, "Please enter the comment, end by new line with string \"end\":\n");

	char first = 1;
	while(1)
	{
	    gets(tmp_line);
		tmp_len += 5;
		tmp_len += strlen(tmp_line);
		if(tmp_len > BUFFER_LEN)
		{
		    fprintf(stdout, "Inputing comment ERROR: The buffer exceed 1024! Ignore this comment!\n");
			break;
		}
		if(strcmp(tmp_line, "end") == 0)
		{
		    strcat(new_secondline, "\n");
		    break;
		}
		if(!first)
		{
		    strcat(new_secondline, "\n  * ");
		    strcat(new_secondline, tmp_line);
		}
		memset(tmp_line, 0, BUFFER_LEN);
		first = 0;
	}
	free(tmp_line);
	
	char new_thirdline[BUFFER_LEN] = "\n -- ";
	if(MAINTAINER)
	{
	    strcat(new_thirdline, MAINTAINER);
		strcat(new_thirdline, " <");
	}
	else
	{
	    strcat(new_thirdline, "Yuanhan Zhang <");
	}
	if(EMAILADDR)
	{
		strcat(new_thirdline, EMAILADDR);
		strcat(new_thirdline, ">  ");
	}
	else
	{
	    strcat(new_thirdline, "johnazhang@tencent.com>  ");
	}

	time_t ti = time(0);
	char formatTime[BUFFER_LEN];
	strftime(formatTime, BUFFER_LEN, "%a, %d %b %Y %H:%M:%S +0800\n\n", localtime(&ti));
	strcat(new_thirdline, formatTime);

	fputs(new_firstline, out);
	fputs(new_secondline, out);
	fputs(new_thirdline, out);

    free(old_versionS);
	old_versionS = NULL;
	old_version = NULL;
	free(new_version);
	new_version = NULL;
	free(new_firstline);
	new_firstline = NULL;
	free(new_secondline);
	new_secondline = NULL;
	
	lineLen = 0;

	//copy the original changelog into new file
	fputs(latesetS, out);
	lateset = latesetS;
	memset(lateset, 0, BUFFER_LEN);

	while(EOF != (ch = fgetc(fp)))
	{		
	    *lateset++ = ch;
		++lineLen;

	    if(lineLen >= BUFFER_LEN)
	    {
	    	fclose(fp);
	        fclose(out);
		    free(latesetS);
	        latesetS = NULL;
			lateset = NULL;

			fprintf(stderr, "Updating changelog ERROR :  The buffer exceed 1024! Skipped processing %s!\n", filename);
			if(remove(tempName) != 0)
			{
				fprintf(stderr, "Updating changelog ERROR :  cannot remove temp file %s! Please remove it manually!\n", tempName);
			}
			
	        goto ret;
	    }
		
		if(ch == '\n')
		{			
		    fputs(latesetS, out);
			lateset = latesetS;
			memset(lateset, 0, BUFFER_LEN);
			lineLen = 0;
		}
	}

	free(latesetS);
	latesetS = NULL;
	lateset = NULL;
	fclose(fp);
	fclose(out);

	replaceFileWithBak(tempName, filename, statbuf);
	
ret:
	free(tempName);
	tempName = NULL;
	return;
}

void changeFileName(char *filename, char *oldText[], char *newText[], int length)
{
    char *newName = changeLine(filename, oldText, newText, length);
	
	if(access(newName, 0) == 0 && strcmp(newName, filename) != 0)
	{
	    fprintf(stderr, "Changing Name ERROR : File %s already exisits! Cannot mv %s into %s!\n", newName, filename, newName);
		free(newName);
		newName = NULL;
		return;
	}

	if(strcmp(newName, filename) == 0)
	{
	    fprintf(stdout, "Remained %s's name\n", filename);
	    free(newName);
		newName = NULL;
		return;
	}

	if(rename(filename, newName) == 0)
	{
	    fprintf(stdout, "Changed %s's name into %s\n", filename, newName);
	}
	else
	{
	    fprintf(stderr, "Changing Name ERROR : function rename occurs an error! Cannot mv %s into %s!\n", filename, newName);
	}

	free(newName);
	newName = NULL;
}

void changeFileText(char *filename, char *oldText[], char *newText[], int length, int tag)
{
	char *tempName;
	tempName = (char*)malloc(strlen(filename) * sizeof(char) + 8);
	memset(tempName, 0, strlen(filename) * sizeof(char) + 8);
	strcpy(tempName, filename);
	strcat(tempName, ".XXXXXX");
	strcpy(tempName, mktemp(tempName));

	FILE *fp, *out;
	
	if(NULL == (fp = fopen(filename, "r+")))
	{
	    fprintf(stderr, "Changing Text ERROR : Cannot open file %s\n", filename);
		goto ret;
	}
	struct stat statbuf;
	stat(filename, &statbuf);
	
	if(NULL == (out = fopen(tempName, "wb+")))
	{
	    fprintf(stderr, "Changing Text ERROR : Cannot create temp file %s for %s\n", tempName, filename);
		fclose(fp);
		if(remove(tempName) != 0)
		{
		    fprintf(stderr, "Changing Text ERROR : Also cannot remove temp file %s! Please remove it manually!\n", tempName);
		}
        goto ret;
	}
	char *line, *lineS;
	char ch;

	line = (char*)malloc(BUFFER_LEN * sizeof(char));
	memset(line, 0, BUFFER_LEN);
	lineS = line;

    char *newLine;
    unsigned int lineLen = 0;
	while(EOF != (ch = fgetc(fp)))
	{		
	    *line++ = ch;
		++lineLen;

	    if(lineLen >= BUFFER_LEN)
	    {
	    	fclose(fp);
	        fclose(out);
		    free(lineS);
	        lineS = NULL;
			line = NULL;

			fprintf(stderr, "Changing Text ERROR :  The buffer exceed 1024! Skipped processing %s!\n", filename);
			if(remove(tempName) != 0)
			{
				fprintf(stderr, "Changing Text ERROR :  cannot remove temp file %s! Please remove it manually!\n", tempName);
			}
			
	        goto ret;
	    }
		
		if(ch == '\n')
		{			
		    if(tag == CHECK_TEXT && compareSubstrtoText(skipTextList, skipTextText, skipTextText_len, lineS))
		    {
		        fputs(lineS, out);
		    }
			else
			{
			    newLine = changeLine(lineS, oldText, newText, length);
			
			    fputs(newLine, out);

			    free(newLine);
			    newLine = NULL;
			}
			
            line = lineS;
			memset(line, 0, BUFFER_LEN);
			lineLen = 0;
		}
	}

	free(lineS);
	lineS = NULL;
	line = NULL;
	fclose(fp);
	fclose(out);

	replaceFileWithoutBak(tempName, filename, statbuf);

ret:
	free(tempName);
	tempName = NULL;
	return;
}

void TraverseDirChange(char *dir, char *oldText[], char *newText[], int length, int tag)
{
    DIR *dp;
	if(NULL == (dp = opendir(dir)))
	{
	    fprintf(stderr, "Cannot open directory: %s\n", dir);
		return;
	}

	struct dirent *entry;
	struct stat statbuf;

	chdir(dir);
	while(NULL != (entry = readdir(dp)))
	{
	    lstat(entry->d_name, &statbuf);
		
		if(S_ISDIR(statbuf.st_mode))
		{
		    if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
		    {
		        continue;
		    }

			fprintf(stdout, "Dealing directory %s\n", entry->d_name);
			TraverseDirChange(entry->d_name, oldText, newText, length, tag);
			
			if(tag == 'n' || tag == 'a' || tag == 'i')
			{
			    changeFileName(entry->d_name, oldText, newText, length);
			}	
		}
		else
		{
		    // check if skip or check the text of current file
		    int skip = compareALLtoText(skipFileList, skipFileText, skipFileText_len, entry->d_name);
			
			int check = 0;
			if(!skip)
			{
			    check = compareALLtoText(checkFileList, checkFileText, checkFileText_len, entry->d_name);
			}

			// start processing file
		    fprintf(stdout, "Processing file %s\n", entry->d_name);
		    if(tag == 'a' || tag == 'i')
		    {
		        if(skip)
		        {
		            fprintf(stdout, "Changing file %s's context is skipped because its name or type\n", entry->d_name);
		        }
				else
				{
				    if(check)
				    {
				        changeFileText(entry->d_name, oldText, newText, length, CHECK_TEXT);
				    }
				    else
				    {
				        changeFileText(entry->d_name, oldText, newText, length, NO_CHECK_TEXT);
				    }
				}
		        
				changeFileName(entry->d_name, oldText, newText, length);
		    }
			else if(tag == 'n')
			{
			    changeFileName(entry->d_name, oldText, newText, length);
			}
			else if(tag == 'c')
			{
			    if(skip)
		        {
		            fprintf(stdout, "Changing file %s's context is skipped because its name or type\n", entry->d_name);
		        }
				else
				{
				    if(check)
				    {
				        changeFileText(entry->d_name, oldText, newText, length, CHECK_TEXT);
				    }
				    else
				    {
				        changeFileText(entry->d_name, oldText, newText, length, NO_CHECK_TEXT);
				    }
				}
			}
		}
	}
	chdir("../");
	closedir(dp);
}

int isDebianDir(char *dir)
{
    DIR *dp;
	if(NULL == (dp = opendir(dir)))
	{
		return 0;
	}

	struct dirent *entry;
	struct stat statbuf;
	
    char *curDir = pwd();
	if(!curDir)
	{
	    closedir(dp);
	    free(curDir);
	    return 0;
	}
	
	chdir(dir);
	while(NULL != (entry = readdir(dp)))
	{
	    lstat(entry->d_name, &statbuf);
		
		if(S_ISDIR(statbuf.st_mode))
		{
		    if(strcmp("debian", entry->d_name) == 0 || strcmp("DEBIAN", entry->d_name) == 0)
		    {
		        if(Upchangelog)
		        {
		            chdir(entry->d_name);
				    updateChangelog(dir);
				    chdir("../");
		        }
	
		        chdir(curDir);
				closedir(dp);
				free(curDir);
		        return 1;
		    }
		}
	}
	chdir(curDir);
	closedir(dp);
	free(curDir);
	return 0;
}

int TraverseSourceDir(char *dir, char *target, char *oldText[], char *newText[], int length, int tag)
{
    DIR *dp;
	int ret = 0;
	if(NULL == (dp = opendir(dir)))
	{
	    fprintf(stderr, "Cannot open source directory: %s\n", dir);
		return 0;
	}

    /* 
       The source dir itself is a debian dir
       Also if the tag i is set, force to change all files without judging if it is a debian dir 
    */
	if(tag == 'i' || isDebianDir(dir))
	{
        chdir(dir);
		char *abs_dir = pwd();
	    if(tag != 'z')
	    {
	        TraverseDirChange(abs_dir, oldText, newText, length, tag);
	    }
		
	    if(tag != 'i')
	    {
	        chdir(abs_dir);
            compileSingleSource(abs_dir);
		    chdir("../");
			compileDsc(abs_dir);
		    mvCompletedDebs(abs_dir, target);
			free(abs_dir);
	    }
		return 1;
	}

	struct dirent *entry;
	struct stat statbuf;

    chdir(dir);
	while(NULL != (entry = readdir(dp)))
	{
	    lstat(entry->d_name, &statbuf);
		
		if(S_ISDIR(statbuf.st_mode))
		{
		    if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
		    {
		        continue;
		    }

			if(isDebianDir(entry->d_name))
			{
			    if(tag != 'z')
			    {  
			        TraverseDirChange(entry->d_name, oldText, newText, length, tag);
			    }

				// fork (maybe we could use docker afterwards) to compile this debian directory
				int pid = fork();
				if(pid == -1)
				{
				    perror("Fork failed!");
				}
				else if(pid == 0)
				{
				    chdir(entry->d_name);
                    compileSingleSource(entry->d_name);
		            chdir("../");
					compileDsc(entry->d_name);
				    mvCompletedDebs(entry->d_name, target);
					ret = 1;
					break;
				}
			}
			else
			{
			    if(TraverseSourceDir(entry->d_name, target, oldText, newText, length, tag))
			    {
			        ret = 1;
					break;
			    }
			}
		}
	}
	chdir("../");
	closedir(dp);
	return ret;
}