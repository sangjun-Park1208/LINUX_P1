#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

void help(void);
char* get_fileType(struct stat *st);
void regFile_Recursive(char* FILENAME, int FILESIZE, char* path);
void print_regFileList(char* path_FILENAME);
void print_dirFileList(void);
char* map_fileType(struct stat *st);

char* regFileList_Candidate[1024];
char* pathSet[1024];
int j;
int k;
char* rwx[8] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};

int main(int argc, char** argv){
	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL); // start time cheking
	for(int i=0; i<1024; i++){
		*(regFileList_Candidate + i) = (char*)malloc(1024);
	}

	while(1){		
		char* parsedInput[1024];
		char inputOption[1024] = "";
		printf("20182613> ");
		scanf("%[^\n]", inputOption);
		char* ptr = strtok(inputOption, " ");
		int count = 0;
		while(ptr != NULL){
			parsedInput[count] = ptr;
			ptr = strtok(NULL, " ");
			count++;
		}
	
		/* tocken checking - get command */
		char* COMMAND_TYPE = parsedInput[0];
		int command_t;
		if(strcmp(COMMAND_TYPE, "find") == 0){
			command_t = 0;
		}
		else if(strcmp(COMMAND_TYPE, "exit") == 0){
			command_t = 1;
		}
		else{
			command_t = 2;
		}

		struct dirent** namelist = NULL;
		struct stat st;
		char* FILENAME_realpath = NULL;
		switch(command_t){ 
			case 0: // "find"
				if(parsedInput[1][0] == '.'){
					if((FILENAME_realpath = realpath(parsedInput[1], NULL)) == NULL){}
				}
				else if(parsedInput[1][0] == '/'){
					FILENAME_realpath = parsedInput[1];
				}
				else{
					char dot_slash[] = "./";
					FILENAME_realpath = strcat(dot_slash, parsedInput[1]); 
					if((FILENAME_realpath = realpath(FILENAME_realpath, NULL)) == NULL){}
				}

				if(stat(FILENAME_realpath, &st) == 0){ // if [FILENAME] exist.
					char* realPath = NULL;
					if(parsedInput[2][0] == '.'){ // if [PATH] is relative path
						if((realPath = realpath(parsedInput[2], NULL)) == NULL){ // relative path -> real path
							printf("realpath() : ");
							switch(errno){
								case EACCES:
									printf("Access Error\n");
									break;
								case EINVAL:
									printf("path is NULL.\n");
									break;
								case EIO:
									printf("I/O error.\n");
									break;
								case ELOOP:
									printf("Too many depth of symbolic link.\n");
									break;
								case ENAMETOOLONG:
									printf("path name is too long.\n");
									break;
								case ENOENT:
									printf("No such file or directory.\n");
									break;
								case ENOTDIR:
									printf("Some directory is not in path.\n");
									break;
								default:
									printf("Other Error.\n");

							}
							break;
						}
						else{
							regFile_Recursive(parsedInput[1], st.st_size, realPath);
//							j = 0;
							memset(&st, 0, sizeof(struct stat));
						}
						
//						for(int i=0; i<j; i++){
//							printf("%d) %s\n", i, regFileList_Candidate[i]);
//						}
					}
					else{ // if [PATH] is real path
						realPath = parsedInput[2];
						regFile_Recursive(parsedInput[1], st.st_size, realPath);
						memset(&st, 0, sizeof(struct stat));

//						for(int i=0; i<j; i++){
//							printf("%d) %s", i, regFileList_Candidate[i]);
//						}
//						j = 0;
					}
					realPath = NULL;
				}	
				else{ // [FILENAME] isn't exist.
					switch(errno){
						case EACCES:
							printf("Access error.\n");
							break;
						case EBADF:
							printf("fd is not a valid open file descriptor.\n");
							break;
						case EINVAL:
							printf("Invalid flag.\n");
							break;
						case EFAULT:
							printf("[FILENAME] isn't exists.\n");
							break;
						case ELOOP:
							printf("Too many symbolic links.\n");
							break;
						case ENAMETOOLONG:
							printf("Path name is too long.\n");
							break;
						case ENOENT:
							printf("Path is empty string || No directory in such path.\n");
							break;
						case ENOMEM:
							printf("Memory not enough.\n");
							break;
						case ENOTDIR:
							printf("Some file is not a directory file.\n");
							break;
						case EOVERFLOW:
							printf("Overflow.\n");
							break;
						default:
							printf("Other Error.\n");
							break;
					}
					break;
				}				
				print_regFileList(FILENAME_realpath);
				FILENAME_realpath = NULL;
				k = 0;
//				for(int i=0; i<j; i++){
//					printf("%d) %s\n", i, regFileList_Candidate[i]);
//				}
				break;
	
			case 1: // "exit"
				gettimeofday(&endTime, NULL);
				printf("Prompt End\n");
				printf("RunTime: %ld:%llu(sec:usec)\n", endTime.tv_sec - startTime.tv_sec,(unsigned long long)endTime.tv_usec - (unsigned long long)startTime.tv_usec);
				exit(0);
				break;
	
			case 2: // "help" || other input
				help();
				break;
	
			default:
				break;
		}
		
		while(getchar() != '\n') // buffer reset
			continue;
	}
	
	exit(0);
}


void help(){
	printf("Usage:\n");
	printf("	> find [FILENAME] [PATH]\n");
	printf("		>> [INDEX] [OPTION ... ]\n");
	printf("	> help\n");
	printf("	> exit\n\n");
	printf("	[OPTIONS ... ]\n");
	printf("	q : report only when files differ\n");
	printf("	s : report when two files are the same\n");
	printf("	i : ignore case differences in file contents\n");
	printf("	r : recursively compare any subdirectories found\n");
	return;
}

char* get_fileType(struct stat *st){ 
	if(S_ISREG(st->st_mode))
		return "regular";
	else if(S_ISDIR(st->st_mode))
		return "directory";
	else
		return "unknown";
}

void regFile_Recursive(char* FILENAME, int FILESIZE, char* path){ // target file's name, target file's size, path
	struct dirent** namelist = NULL;
	struct stat st;
	int fileCount = scandir(path, &namelist, NULL, alphasort);
	char* FILETYPE = get_fileType(&st);
	int DIRCOUNT = 0;

	/*** get Directory file count ***/
	printf("Traversing ..\n");
	for(int i=0; i<fileCount; i++){
		char* slash = "/";
		char* tmp_path = (char*)malloc(1024);
		strcpy(tmp_path, path);
		strcat(tmp_path, slash);
		strcat(tmp_path, namelist[i]->d_name);
		stat(tmp_path, &st);
		printf("tmp_path : %s\n", tmp_path);
		free(tmp_path);
		if(S_ISREG(st.st_mode)){
			printf("type : %s\n", "regular");
		}
		if(S_ISDIR(st.st_mode)){
			printf("type : %s\n", "directory");
			DIRCOUNT++;
		}
	}
	
	if(DIRCOUNT == 0){ // if there is no directory
		printf("No DIR file.\n");
		for(int i=0; i<fileCount; i++){ // visit all files.
			char* slash = "/";
			char* tmp_path = (char*)malloc(1024);
			strcpy(tmp_path, path);
			strcat(tmp_path, slash);
			strcat(tmp_path, namelist[i]->d_name);
			pathSet[k] = tmp_path;
			stat(pathSet[k], &st);
			free(tmp_path);
			
			int fileCount1 = scandir(pathSet[k], &namelist, NULL, alphasort);
			if( (strcmp(FILENAME, namelist[i]->d_name) == 0) && (st.st_size == FILESIZE) ){
//				regFileList_Candidate[j++] = pathSet[k];
				strcpy(regFileList_Candidate[j++], pathSet[k]);
//				printf("regFileList_Candidate : %s\n", regFileList_Candidate[j-1]);
				k++;
				return;
			}

		}
	}
	else { // if there is directory file
		for(int i=0; i<fileCount; i++){
			char* slash = "/";
			char* tmp_path = (char*)malloc(1024);
			strcpy(tmp_path, path);
			strcat(tmp_path, slash);
			strcat(tmp_path, namelist[i]->d_name);
			pathSet[k] = tmp_path;
			stat(pathSet[k], &st);
			printf("real path : %s\n", tmp_path);

			if(S_ISDIR(st.st_mode)){
				if(strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0 ){}
				else{
					printf("Recursive!!!!!\n\n");
					regFile_Recursive(FILENAME, FILESIZE, tmp_path);
				}
			}
			else {
				if( (strcmp(FILENAME, namelist[i]->d_name) == 0) && (st.st_size == FILESIZE)){
					strcpy(regFileList_Candidate[j++], pathSet[k]);
//					regFileList_Candidate[j++] = pathSet[k];
//					printf("regFileList_Candidate : %s\n", regFileList_Candidate[j-1]);
					k++;
				}
			}
			free(tmp_path);
		}
	}
}

void print_regFileList(char* path_FILENAME){
	struct stat st;
	stat(path_FILENAME, &st);
	
	time_t at = st.st_atime;
	time_t ct = st.st_ctime;
	time_t mt = st.st_mtime;
	struct tm aT;
	struct tm cT;
	struct tm mT;

	localtime_r(&at, &aT);
	localtime_r(&ct, &cT);
	localtime_r(&mt, &mT);
	printf("Index Size Mode       Blocks Links UID  GID  Access       Change       Modify       Path\n");

	printf("%-5d %-4ld %-10s %-6ld %-5ld %-4d %-4d %d-%d-%d %d:%d %d-%d-%d %d:%d %d-%d-%d %d:%d %s\n",
			0, st.st_size, map_fileType(&st), st.st_blocks, st.st_nlink, st.st_uid, st.st_gid,
			aT.tm_year+1900-2000, aT.tm_mon+1, aT.tm_mday+1, aT.tm_hour, aT.tm_min,
			cT.tm_year+1900-2000, cT.tm_mon+1, cT.tm_mday+1, cT.tm_hour, cT.tm_min,
			mT.tm_year+1900-2000, mT.tm_mon+1, mT.tm_mday+1, mT.tm_hour, mT.tm_min,
		  	path_FILENAME);
	memset(&st, 0, sizeof(struct stat));

	for(int i=1; i <= j-1; i++){
		stat(regFileList_Candidate[i], &st);
		printf("%-5d %-4ld %-10s %-6ld %-5ld %-4d %-4d %d-%d-%d %d:%d %d-%d-%d %d:%d %d-%d-%d %d:%d %s\n",
			i, st.st_size, map_fileType(&st), st.st_blocks, st.st_nlink, st.st_uid, st.st_gid,
			aT.tm_year+1900-2000, aT.tm_mon+1, aT.tm_mday+1, aT.tm_hour, aT.tm_min,
			cT.tm_year+1900-2000, cT.tm_mon+1, cT.tm_mday+1, cT.tm_hour, cT.tm_min,
			mT.tm_year+1900-2000, mT.tm_mon+1, mT.tm_mday+1, mT.tm_hour, mT.tm_min,
		  	regFileList_Candidate[i]);
	}
	j = 0;
}

char* map_fileType(struct stat *st){
	char* accessMode = (char*)malloc(1024);
	
	if(S_ISDIR(st->st_mode))
		strcpy(accessMode, "d");
	else
		strcpy(accessMode, "-");

	int mode_o = st->st_mode & S_IRWXO;
	int mode_g = (st->st_mode >> 3) & S_IRWXO;
	int mode_u = (st->st_mode >> 6) & S_IRWXO;
	
	accessMode = strcat(accessMode, rwx[mode_u]);
	accessMode = strcat(accessMode, rwx[mode_g]);
	accessMode = strcat(accessMode, rwx[mode_o]);
	return accessMode;
}



void print_dirFileList(){

}











