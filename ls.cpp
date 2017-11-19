#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PATH_LN 100

typedef struct dirent*  dirPtr;
typedef struct stat	fileStat;

int openDir(char* path, unsigned int param);
int ls(struct dirent* entry, unsigned int param, char* path);


typedef enum{
	PERMIT= 	0b1,
	RECUR= 		0b10,
	L_LIST= 	0b100,
	INODE_NUM= 	0b1000,
	LINE_1= 	0b10000,
	SIZE= 		0b100000,
	ALL= 		0b1000000,
	OWNERS=		0b10000000,
	ATIME=		0b100000000,
	MTIME =		0b1000000000
	} op;


int ls(struct dirent* entry, unsigned int param, char* path){

	char newPath[200] = { '\0' };
	char entryPath[200] = { '\0' };
	fileStat buff;
	int isDir;
	
	sprintf(entryPath, "%s/%s", path, entry->d_name);
	stat(entryPath, &buff);
	char output[500] = {'\0'};
	char temp[100] = {'\0'};
	isDir = (buff.st_mode & S_IFDIR);

	if(param & PERMIT){
		char auth[10];
		for (int i = 0; i < 9 ; i++)
			auth[i] = '-';
		auth[9] = '\0';

		//owner permision
		if(buff.st_mode & S_IRUSR)
			auth[0] = 'r';
		if(buff.st_mode & S_IWUSR)
			auth[1] = 'w';
		if(buff.st_mode & S_IXUSR)
			auth[2] = 'x';
		//group
		if(buff.st_mode & S_IRGRP)
			auth[3] = 'r';
		if(buff.st_mode & S_IWGRP)
			auth[4] = 'w';
		if(buff.st_mode & S_IXGRP)
			auth[5] = 'x';
	
		//others
		if(buff.st_mode & S_IROTH)
			auth[6] = 'r';
		if(buff.st_mode & S_IWOTH)
			auth[7] = 'w';
		if(buff.st_mode & S_IXOTH)
			auth[8] = 'x';

		sprintf(temp, "%s\t", auth);
		strcat(output, temp);

	}

	if (param & OWNERS) {
		struct passwd* pwuid = getpwuid(buff.st_uid);
		struct group* guid = getgrgid(buff.st_gid);
		sprintf(temp, "%s\t%s\t", pwuid->pw_name, guid->gr_name);
		strcat(output, temp);
	}

	if (param & SIZE) {
		sprintf(temp, BOLDRED "%lu\t" ANSI_COLOR_RESET, buff.st_size);
		strcat(output, temp);
	}
	if (param & ATIME) {
		char atime[100];
		//char mtime[100];
		strftime(atime, sizeof(atime), "%D  %T", localtime(&buff.st_atim.tv_sec));
		sprintf(temp, "%s\t", atime);
		//strftime(mtime, sizeof(mtime), "%D  %T", gmtime(&buff.st_mtim.tv_sec));
		strcat(output, temp);
	}
	if (param & MTIME) {
		char mtime[100];
		strftime(mtime, sizeof(mtime), "%D  %T", localtime(&buff.st_mtim.tv_sec));
		sprintf(temp, "%s\t", mtime);
		strcat(output, temp);
	}

	if (param & INODE_NUM) {
		sprintf(temp, BOLDYELLOW "%lu\t" ANSI_COLOR_RESET, buff.st_ino);
		strcat(output, temp);
	}

	if(isDir){
		
		sprintf(temp , BOLDMAGENTA "%s"  ANSI_COLOR_RESET "\t", entry->d_name);
		strcat(output, temp);
	}else{
		sprintf(temp,"%s\t", entry->d_name);
		strcat(output, temp);
	}
	printf("%s", output);
	if (param & LINE_1)
		printf("\n");

	if (isDir && (param & RECUR)) {
		if(path[strlen(path) -1] == '/')
			sprintf(newPath, "%s%s", path, entry->d_name);
		else
		sprintf(newPath, "%s/%s", path, entry->d_name);
		printf(BOLDYELLOW"SUB FOLDER: %s:\n" ANSI_COLOR_RESET, newPath);
		openDir(newPath, param);
	}

	return 0;
}


int openDir(char* path, unsigned int param){
	DIR* dirPtr;
	struct dirent* entry;

	if((dirPtr = opendir(path)) == NULL){
		fprintf(stderr, "%s\n", strerror(errno));
		//printf("Here: %s\n", path);
		return -1;
	}

	while((entry = readdir(dirPtr))){
		if(entry->d_name[0] == '.' ){
			if(param & ALL)
				ls(entry, param, path);
		}
		else{
			ls(entry, param, path);
		}
	}
	closedir(dirPtr);
	return 0;
}


int main(int argc, char* argv[]){
	char filepath[PATH_LN] = {'\0'};
	int option = 0;
	char options[10] = { '\0' };
	unsigned int  param = 0;

	if(argc < 2){
		perror("usage: programName [option] filepath\n");
		exit(1);
	}

	//check if there is an option
	if(argc > 2){
		option = 1;
	}
	
	//buidling the option string
	for (int i = 1; i < argc; i++) {
		//options have to start with -
		if (argv[i][0] == '-') {
			strcat(options, &argv[i][1]);
		}
		else {
			strcpy(filepath, argv[i]);
			break;
		}
	}


	if(option){
		for(unsigned int i=0; i < strlen(options); i++){
			if (options[i] == 'i')
				param = (param | INODE_NUM);
			else if (options[i] == 's')
				param = (param | SIZE);
			else if (options[i] == 'a')
				param = (param | ALL);
			else if (options[i] == '1')
				param = (param | LINE_1);
			else if (options[i] == 'p')
				param = (param | PERMIT);
			else if (options[i] == 'o')
				param = (param | OWNERS);
			else if (options[i] == 't')
				param = (param | ATIME | MTIME);
			else if (options[i] == 'R')
				param = (param | RECUR | LINE_1);
			else if(options[i] == 'l')
				param = (param | L_LIST | SIZE | LINE_1 | PERMIT | OWNERS | ATIME);

		}
		//printf("OP: %s\n",options);
	}

	openDir(filepath, param);
	printf("\n");
	return 0;
}
