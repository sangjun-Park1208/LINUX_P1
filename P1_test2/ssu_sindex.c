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
void print_dirFileList(char* path_FILENAME);
char* map_fileType(struct stat *st);
void debug_fileExist();
void debug_relativePathError();
void dirFile_Recursive(char* FILENAME, int FILESIZE, char* path);
int get_dirSize(char* path);

char* regFileList_Candidate[1024];
char* dirFileList_Candidate[1024];
char* pathSet[1024]; // check regular file
char* d_pathSet[1024]; // check directory
char* ds_pathSet[1024]; // check directory size
int j; // regFileList_Candidate[j]
int k; // pathSet[k]
int t; // dirFileList_Candidate[t]
int d; // d_pathSet[d]
int w; // ds_pathSet[w]
char* rwx[8] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};

int main(int argc, char** argv){
	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL); // start time cheking
	
	for(int i=0; i<1024; i++){
		*(regFileList_Candidate + i) = (char*)malloc(1024);
		*(dirFileList_Candidate + i) = (char*)malloc(1024);
		*(ds_pathSet + i) = (char*)malloc(1024);
		*(d_pathSet + i) = (char*)malloc(1024);
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
							debug_relativePathError();
							break;
						}
						else{
							if(S_ISREG(st.st_mode)){
								regFile_Recursive(FILENAME_realpath, st.st_size, realPath);
							}
							if(S_ISDIR(st.st_mode)){
								printf("FILENAME_realpath : %s\n", FILENAME_realpath);
								printf("FILENAME's size(dir) : %d\n", get_dirSize(FILENAME_realpath));
								dirFile_Recursive(FILENAME_realpath, get_dirSize(FILENAME_realpath), realPath);
								w = 0;
							}
						}
					}
					else{ // if [PATH] is real path
						realPath = parsedInput[2];
						if(S_ISREG(st.st_mode)){
							regFile_Recursive(FILENAME_realpath, st.st_size, realPath);
						}
						if(S_ISDIR(st.st_mode)){
							dirFile_Recursive(FILENAME_realpath, get_dirSize(FILENAME_realpath), realPath);
							w = 0;
						}
					}
					realPath = NULL;
				}	
				else{ // [FILENAME] isn't exist.
					debug_fileExist();
					break;
				}				
				if(S_ISREG(st.st_mode))
					print_regFileList(FILENAME_realpath);
				
				if(S_ISDIR(st.st_mode))
					print_dirFileList(FILENAME_realpath);
				
				
				memset(&st, 0, sizeof(struct stat));
				FILENAME_realpath = NULL;
				k = 0;
				w = 0;
				t = 0;
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

void debug_relativePathError(){
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
}

void debug_fileExist(){
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
	char* extracted_RegFileName = (char*)malloc(1024);
	extracted_RegFileName = strrchr(FILENAME, '/');
	extracted_RegFileName++;

	/*** get Directory file count ***/
	for(int i=0; i<fileCount; i++){
		char* slash = "/";
		char* tmp_path = (char*)malloc(1024);
		strcpy(tmp_path, path);
		strcat(tmp_path, slash);
		strcat(tmp_path, namelist[i]->d_name);
		stat(tmp_path, &st);
		free(tmp_path);
		if(S_ISREG(st.st_mode)){
		}
		if(S_ISDIR(st.st_mode)){
			DIRCOUNT++;
		}
	}
	
	if(DIRCOUNT == 0){ // if there is no directory
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
			if( (strcmp(extracted_RegFileName, namelist[i]->d_name) == 0) && (st.st_size == FILESIZE) ){
				strcpy(regFileList_Candidate[j++], pathSet[k]);
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
			if(i==0 || i==1){}
			else{
				printf("real path : %s\n", tmp_path);
			}

			if(S_ISDIR(st.st_mode)){
				if(strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0 ){}
				else if(strcmp(namelist[i]->d_name, "proc") == 0 || strcmp(namelist[i]->d_name, "run") == 0 || strcmp(namelist[i]->d_name, "snap") == 0){}
				else if(strcmp(namelist[i]->d_name, "bin") == 0 || strcmp(namelist[i]->d_name, "sys") == 0 || strcmp(namelist[i]->d_name, "opt") == 0){}
				else{
					regFile_Recursive(FILENAME, FILESIZE, tmp_path);
				}
			}
			else {
				if( (strcmp(extracted_RegFileName, namelist[i]->d_name) == 0) && (st.st_size == FILESIZE)){
					strcpy(regFileList_Candidate[j++], pathSet[k]);
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

	if(j>2){
		for(int s=j-1; s>0; s--){ // loop count : 6
			for(int i=1; i <= s-1; i++){
				char* tmp = (char*)malloc(1024);
				if(strlen(regFileList_Candidate[i]) > strlen(regFileList_Candidate[i+1])){
					strcpy(tmp, regFileList_Candidate[i]);
					strcpy(regFileList_Candidate[i], regFileList_Candidate[i+1]);
					strcpy(regFileList_Candidate[i+1], tmp);
					free(tmp);
				}
			}
		}
	}
	
	printf("Index Size Mode       Blocks Links UID  GID  Access         Change         Modify         Path\n");

	printf("%-5d %-4ld %-10s %-6ld %-5ld %-4d %-4d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %s\n",
			0, st.st_size, map_fileType(&st), st.st_blocks, st.st_nlink, st.st_uid, st.st_gid,
			aT.tm_year+1900-2000, aT.tm_mon+1, aT.tm_mday+1, aT.tm_hour, aT.tm_min,
			cT.tm_year+1900-2000, cT.tm_mon+1, cT.tm_mday+1, cT.tm_hour, cT.tm_min,
			mT.tm_year+1900-2000, mT.tm_mon+1, mT.tm_mday+1, mT.tm_hour, mT.tm_min,
		  	path_FILENAME);
	memset(&st, 0, sizeof(struct stat));

	for(int i=1; i <= j-1; i++){
		struct stat st1;
		char* tmpreg = (char*)malloc(1024);
		strcpy(tmpreg, regFileList_Candidate[i]);
		stat(tmpreg, &st1);

		time_t at = st1.st_atime;
		time_t ct = st1.st_ctime;
		time_t mt = st1.st_mtime;
		struct tm aT;
		struct tm cT;
		struct tm mT;
	
		localtime_r(&at, &aT);
		localtime_r(&ct, &cT);
		localtime_r(&mt, &mT);
	
		printf("%-5d %-4ld %-10s %-6ld %-5ld %-4d %-4d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %s\n",
			i, st1.st_size, map_fileType(&st1), st1.st_blocks, st1.st_nlink, st1.st_uid, st1.st_gid,
			aT.tm_year+1900-2000, aT.tm_mon+1, aT.tm_mday+1, aT.tm_hour, aT.tm_min,
			cT.tm_year+1900-2000, cT.tm_mon+1, cT.tm_mday+1, cT.tm_hour, cT.tm_min,
			mT.tm_year+1900-2000, mT.tm_mon+1, mT.tm_mday+1, mT.tm_hour, mT.tm_min,
		  	regFileList_Candidate[i]);
		memset(&st1, 0, sizeof(struct stat));
		free(tmpreg);
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

void dirFile_Recursive(char* FILENAME, int FILESIZE, char* path){
	struct dirent** namelist = NULL;
	struct stat st;
	int fileCount = scandir(path, &namelist, NULL, alphasort);
	int DIRCOUNT = 0;
	char* extracted_DirFileName = (char*)malloc(1024);
	extracted_DirFileName =strrchr(FILENAME, '/');
	extracted_DirFileName++;

	/*** get Directory file count ***/
	for(int i=0; i<fileCount; i++){
		char* slash = "/";
		char* tmp_path = (char*)malloc(1024);
		strcpy(tmp_path, path);
		strcat(tmp_path, slash);
		strcat(tmp_path, namelist[i]->d_name);
		stat(tmp_path, &st);
		free(tmp_path);
		if(S_ISREG(st.st_mode)){}
		if(S_ISDIR(st.st_mode)){
			DIRCOUNT++;
		}
	}

	if(DIRCOUNT == 0){
		for(int i=0; i<fileCount; i++){
			char* slash = "/";
			char* tmp_path = (char*)malloc(1024);
			strcpy(tmp_path, path);
			strcat(tmp_path, slash);
			strcat(tmp_path, namelist[i]->d_name);
			strcpy(d_pathSet[d], tmp_path);			
			stat(d_pathSet[d], &st);
			free(tmp_path);
			int fileCount1 = scandir(d_pathSet[d], &namelist, NULL, alphasort);
			
			if( (strcmp(extracted_DirFileName, namelist[i]->d_name) == 0) && (get_dirSize(d_pathSet[d]) == FILESIZE) ) {
				w = 0;
				strcpy(dirFileList_Candidate[t++], d_pathSet[d]);
				d++;
				return;
			}
		}
	}
	else{
		for(int i=0; i<fileCount; i++){
			char* slash = "/";
			char* tmp_path = (char*)malloc(1024);
			strcpy(tmp_path, path);
			strcat(tmp_path, slash);
			strcat(tmp_path, namelist[i]->d_name);
			strcpy(d_pathSet[d], tmp_path);			
			stat(d_pathSet[d], &st);
			if(i==0 || i==1){}
			else{
				printf("real path : %s\n", tmp_path);
			}
			free(tmp_path);

			if(S_ISDIR(st.st_mode)){
				if(strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0){}
				else if(strcmp(namelist[i]->d_name, "proc") == 0 || strcmp(namelist[i]->d_name, "run") == 0 || strcmp(namelist[i]->d_name, "snap") == 0){}
				else if(strcmp(namelist[i]->d_name, "bin") == 0 || strcmp(namelist[i]->d_name, "sys") == 0 || strcmp(namelist[i]->d_name, "opt") == 0){}
				else{
					if( (strcmp(extracted_DirFileName, namelist[i]->d_name) == 0) && (get_dirSize(d_pathSet[d]) == FILESIZE) ){
						w = 0;
						strcpy(dirFileList_Candidate[t++], d_pathSet[d]);
					}
					dirFile_Recursive(FILENAME, FILESIZE, d_pathSet[d++]);
				}
			}
		}
	}
}

void print_dirFileList(char* path_FILENAME){
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

	if(t>2){
		for(int r=t-1; r>0; r--){
			for(int i=1; i <= r-1; i++){
				char* tmp= (char*)malloc(1024);
				if(strlen(dirFileList_Candidate[i+1]) < strlen(dirFileList_Candidate[i])){
					strcpy(tmp, dirFileList_Candidate[i]);
					strcpy(dirFileList_Candidate[i], dirFileList_Candidate[i+1]);
					strcpy(dirFileList_Candidate[i+1], tmp);
					free(tmp);
				}
			}
		}
	}
	printf("Index Size Mode       Blocks Links UID  GID  Access         Change         Modify         Path\n");

	printf("%-5d %-4d %-10s %-6ld %-5ld %-4d %-4d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %s\n",
			0, get_dirSize(path_FILENAME), map_fileType(&st), st.st_blocks, st.st_nlink, st.st_uid, st.st_gid,
			aT.tm_year+1900-2000, aT.tm_mon+1, aT.tm_mday+1, aT.tm_hour, aT.tm_min,
			cT.tm_year+1900-2000, cT.tm_mon+1, cT.tm_mday+1, cT.tm_hour, cT.tm_min,
			mT.tm_year+1900-2000, mT.tm_mon+1, mT.tm_mday+1, mT.tm_hour, mT.tm_min,
		  	path_FILENAME);
	memset(&st, 0, sizeof(struct stat));
	w = 0;
	for(int i=1; i <= t-1; i++){
		struct stat st2;
		char* tmpdir = (char*)malloc(1024);
		strcpy(tmpdir, dirFileList_Candidate[i]);
		stat(tmpdir, &st2);

		time_t at = st2.st_atime;
		time_t ct = st2.st_ctime;
		time_t mt = st2.st_mtime;
		struct tm aT;
		struct tm cT;
		struct tm mT;
		localtime_r(&at, &aT);
		localtime_r(&ct, &cT);
		localtime_r(&mt, &mT);

		printf("%-5d %-4d %-10s %-6ld %-5ld %-4d %-4d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %d-%02d-%02d %02d:%02d %s\n",
			i, get_dirSize(path_FILENAME), map_fileType(&st2), st2.st_blocks, st2.st_nlink, st2.st_uid, st2.st_gid,
			aT.tm_year+1900-2000, aT.tm_mon+1, aT.tm_mday+1, aT.tm_hour, aT.tm_min,
			cT.tm_year+1900-2000, cT.tm_mon+1, cT.tm_mday+1, cT.tm_hour, cT.tm_min,
			mT.tm_year+1900-2000, mT.tm_mon+1, mT.tm_mday+1, mT.tm_hour, mT.tm_min,
		  	dirFileList_Candidate[i]);
		w = 0;
		memset(&st2, 0, sizeof(struct stat));
		free(tmpdir);
	}
}

int get_dirSize(char* path){
	int sum = 0;
	struct dirent** namelist = NULL;
	int fileCount = scandir(path, &namelist, NULL, alphasort);
	for(int i=0; i<fileCount; i++){
		struct stat st;
		char* slash = "/";
		char* tmp_path = (char*)malloc(1024);
		strcpy(tmp_path, path);
		strcat(tmp_path, slash);
		strcat(tmp_path, namelist[i]->d_name);
		stat(tmp_path, &st);
		strcpy(ds_pathSet[w], tmp_path);
		free(tmp_path);

		if(strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0){} // except . and ..
		else if(S_ISREG(st.st_mode)){
			sum += st.st_size;
		}
		else if(S_ISDIR(st.st_mode)){
			sum += get_dirSize(ds_pathSet[w++]);
		}
	}
	return sum;
}




















