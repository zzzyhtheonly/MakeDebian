#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "func.h"
#include "public.h"

char Upchangelog = 1;

void TraverseSourceDir(char *dir, char *target, char *oldText[], char *newText[], int length, int tag);

void printHelp()
{
    char *info = "Usage: MakeTencentDeb <commands>\n"
		"This is help info.\n";
	printf("%s", info);
}

int main(int argc, char *argv[])
{
    int opt;
	struct option longopts[] = 
	{
		{"help", 0, NULL, 'h'},
		{"source", 1, NULL, 's'},
		{"target", 1, NULL, 't'},
		{"changeAll", 0, NULL, 'a'},
		{"changeName", 0, NULL, 'n'},
		{"changeContext", 0, NULL, 'c'},
		{"ignoreDebian", 0, NULL, 'i'},
		{"maintainer", 1, NULL, 'm'},
		{"email", 1, NULL, 'e'},
		{"no-updatechangelog", 0, NULL, 'U'},
		{0,0,0,0}
	};

	char *source = NULL, *target = NULL;
	char tag = 'z';
	
	while ((opt = getopt_long(argc, argv, ":hanciUs:t:m:e:", longopts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'h':
				printHelp();
				return 0;
			case 's':
				source = (char*)malloc(strlen(optarg) * sizeof(char) + 1);
				memset(source, 0, strlen(optarg) * sizeof(char) + 1);
				strcpy(source, optarg);
				break;	
			case 't':
				target = (char*)malloc(strlen(optarg) * sizeof(char) + 1);
				memset(target, 0, strlen(optarg) * sizeof(char) + 1);
				strcpy(target, optarg);
				break;
			case 'U':
				Upchangelog = 0;
				break;
			case 'm':
				MAINTAINER = (char*)malloc(strlen(optarg) * sizeof(char) + 1);
				memset(MAINTAINER, 0, strlen(optarg) * sizeof(char) + 1);
			    strcpy(MAINTAINER, optarg);
				break;
			case 'e':
				EMAILADDR = (char*)malloc(strlen(optarg) * sizeof(char) + 1);
				memset(EMAILADDR, 0, strlen(optarg) * sizeof(char) + 1);
			    strcpy(EMAILADDR, optarg);
				break;
			case 'a':
			case 'n':
			case 'c':
			case 'i':
				tag = opt;
				break;
			case '?':
				printf("Unknown option: %c\nPlease check help info.", optopt);
				printHelp();
			    if(source)
		            free(source);
	            if(target)
		            free(target);
                return 1;
				break;
		}
	}

	int length = argc - optind;
    if(length & 1)
    {
        fprintf(stderr, "No pair to change %s! Please check the help info!\n", argv[argc-1]);
		printHelp();
        if(source)
		    free(source);
	    if(target)
		    free(target);
        return 0;
    }
	
	int i = 0;
	char *args[length];
	while(optind < argc)
	{
	    args[i++] = argv[optind++];
	}

    int real_len = length/2;
	char *oldText[real_len];
	char *newText[real_len];
	parseTextArgs(args, length, oldText, newText);

	if(!source)
	{
	    fprintf(stderr, "A source directory need to be given!\n");
		printHelp();
		goto ret;
	}

	init();
	if(!ROOTDIR)
	{
	    goto ret;
	}

    // change target directory into absolute path
	if(target)
	{
		DIR *dp;
		if(NULL == (dp = opendir(target)))
		{
		    fprintf(stderr, "Cannot open target directory: %s\n", target);
			printHelp();
			goto ret;
		}
		closedir(dp);
		
	    chdir(target);		
		free(target);
		target = NULL;
		target = pwd();
		chdir(ROOTDIR);
		if(!target)
		{
		    goto ret;
		}
	}
	
	TraverseSourceDir(source, target, oldText, newText, real_len, tag);

ret:
	if(source)
		free(source);
	if(target)
		free(target);
	if(ROOTDIR)
	    free(ROOTDIR);
	if(EMAILADDR)
		free(EMAILADDR);
	if(MAINTAINER)
		free(MAINTAINER);
    return 0;
}
