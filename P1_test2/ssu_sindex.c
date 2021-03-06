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

#define max(a,b) (((a) > (b)) ? (a) : (b))

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
//void reg_diff(char* regFile_selected, char* option);
void reg_diff(char* regFile_selected);
//void dir_diff(char* dirFile_selected, char* option);
void dir_diff(char* dirFile_selected);
int getTotalLine(char* filePath);
//void LCS(char* sourcePath, char* targetPath, int opt_type);
void LCS(char* sourcePath, char* targetPath);
void printOneLine(char* fileName, int lineNum, int type);
void printManyLine(char* fileName, int min, int max, int type);

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
//char* opt[16] = {
//				"----", "---r", "--i-", "--ir",	"-s--", "-s-r", "-si-", "-sir",
//				"q---", "q--r", "q-i-", "q-ir",	"qs--", "qs-r", "qsi-", "qsir"
//				}; // q, s, i, r

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
//								printf("FILENAME_realpath : %s\n", FILENAME_realpath);
//								printf("FILENAME's size(dir) : %d\n", get_dirSize(FILENAME_realpath));
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
				if(S_ISREG(st.st_mode)){ // if regular file
					print_regFileList(FILENAME_realpath); // print regular file list
					if(j == 1){
						j = 0;
						break;
					}
					
					int reg_index;
					char* reg_option = (char*)malloc(1024);
					printf(">> ");
//					scanf("%d%[^\n]", &reg_index, reg_option);
					scanf("%d", &reg_index);
//					printf("reg_option : %s\n", reg_option);
//					reg_diff( regFileList_Candidate[reg_index], reg_option);
					
					if(reg_index > j-1){
						printf("Select index below %d\n", j);
						j = 0;
						break;
					}
					j = 0;
					reg_diff( regFileList_Candidate[reg_index]);
					break;
				}
				
				if(S_ISDIR(st.st_mode)){ // if directory file
					print_dirFileList(FILENAME_realpath); // print directory list
					if(t == 1){
						t = 0;
						break;
					}

					int dir_index;
					char* dir_option = (char*)malloc(1024);
					printf(">> ");
//					scanf("%d %[^\n]", &dir_index, dir_option);
					scanf("%d", &dir_index);
//					dir_diff( dirFileList_Candidate[dir_index], dir_option);
					
					if(dir_index > t-1){
						printf("Select index below %d\n", t);
						t = 0;
						break;
					}
					dir_diff( dirFileList_Candidate[dir_index]);
					for(int i=0; i<t; i++){
						memset(dirFileList_Candidate[i], '\0', 1024);
					}
					t=0;
					break;
				}
				
				
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
//	printf("		>> [INDEX] [OPTION ... ]\n");
	printf("		>> [INDEX]\n");
	printf("	> help\n");
	printf("	> exit\n\n");
//	printf("	[OPTIONS ... ]\n");
//	printf("	q : report only when files differ\n");
//	printf("	s : report when two files are the same\n");
//	printf("	i : ignore case differences in file contents\n");
//	printf("	r : recursively compare any subdirectories found\n");
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
//				printf("real path : %s\n", tmp_path);
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
		for(int s=j-1; s>0; s--){
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
	if(j == 1){
		printf("(None)\n");
		return;
	}
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

//	j = 0;
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
	char* extracted_DirFileName = (char*)malloc(1024);
	extracted_DirFileName =strrchr(FILENAME, '/');
	extracted_DirFileName++;
	int DIRCOUNT = 0;

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
				strcpy(dirFileList_Candidate[t++], d_pathSet[d++]);
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
//				printf("real path : %s\n", tmp_path);
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

	if(t == 1){
		printf("(None)\n");
		return;
	}
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



//void reg_diff(char* regFile_selected, char* option){
void reg_diff(char* regFile_selected){
//	printf("regFileList_Candidate[0] : %s\n", regFileList_Candidate[0]);

/*	
	int opt_type = 0;
	if(strchr(option, 'q') != NULL){
		opt_type = opt_type | 0b1000;
	}
	if(strchr(option, 's') != NULL){
		opt_type = opt_type | 0b0100;
	}
	if(strchr(option, 'i') != NULL){
		opt_type = opt_type | 0b0010;
	}
	if(strchr(option, 'r') != NULL){
		opt_type = opt_type | 0b0001;
	}
*/

//	printf("opt_type : %s\n", opt[opt_type]);
//	LCS(regFileList_Candidate[0], regFile_selected, opt_type);
	LCS(regFileList_Candidate[0], regFile_selected);

}

//void dir_diff(char* dirFile_selected, char* option){
void dir_diff(char* dirFile_selected){

/*
	int opt_type = 0;
	if(strchr(option, 'q') != NULL){
		opt_type = opt_type | 0b1000;
	}
	if(strchr(option, 's') != NULL){
		opt_type = opt_type | 0b0100;
	}
	if(strchr(option, 'i') != NULL){
		opt_type = opt_type | 0b0010;
	}
	if(strchr(option, 'r') != NULL){
		opt_type = opt_type | 0b0001;
	}
//	printf("opt_type : %s\n", opt[opt_type]);
*/

	struct dirent** source_namelist = NULL;
	struct dirent** target_namelist = NULL;

	int source_filecount = scandir(dirFileList_Candidate[0], &source_namelist, NULL, alphasort);
	int target_filecount = scandir(dirFile_selected, &target_namelist, NULL, alphasort);
	struct stat st_source;
	struct stat st_target;
	
	char* sourcefile_path = (char*)malloc(1024);
	char* targetfile_path = (char*)malloc(1024);
	char* slash = "/";
	
	int file_max_size = max(source_filecount,target_filecount);

	int source_only = 0; // check if only in source file
	for(int i=2; i<source_filecount; i++){
		strcpy(sourcefile_path, dirFileList_Candidate[0]);
		strcat(sourcefile_path, slash);
		strcat(sourcefile_path, source_namelist[i]->d_name);
//		printf("sourcefile_path : %s\n", sourcefile_path);
		stat(sourcefile_path, &st_source);
		
		for(int s=2; s<target_filecount; s++){
			strcpy(targetfile_path, dirFile_selected);
			strcat(targetfile_path, slash);
			strcat(targetfile_path, target_namelist[s]->d_name);
//			printf("targetfile_path : %s\n", targetfile_path);
			stat(targetfile_path, &st_target);

			if(strcmp(source_namelist[i]->d_name, target_namelist[s]->d_name) == 0){
				source_only = 1;
				if(S_ISREG(st_source.st_mode) && S_ISREG(st_target.st_mode)){
//					printf("diff -%s %s %s\n", option, sourcefile_path, targetfile_path);
					printf("diff %s %s\n", sourcefile_path, targetfile_path);
//					LCS(sourcefile_path, targetfile_path, opt_type);
					LCS(sourcefile_path, targetfile_path);
					break;
				}
				else if(S_ISREG(st_source.st_mode) && S_ISDIR(st_target.st_mode)){
					printf("File %s is a regular file while file %s is a directory\n", sourcefile_path, targetfile_path);
				}
				else if(S_ISDIR(st_source.st_mode) && S_ISREG(st_target.st_mode)){
					printf("File %s is a directory while file %s is a regular file\n", sourcefile_path, targetfile_path);
				}
				else{ // both are directories
					printf("Common subdirectories : %s and %s\n", sourcefile_path, targetfile_path);
				}
			}
			else{
//				printf("Not same..\n");
			}
		}
		if(source_only == 0){
			printf("Only in %s : %s\n", dirFileList_Candidate[0], source_namelist[i]->d_name);
		}
		source_only = 0;
	}

	// MUST!! Initialize sourcefile_path & targetfile_path
	free(sourcefile_path);
	free(targetfile_path);
	sourcefile_path = (char*)malloc(1024);
	targetfile_path = (char*)malloc(1024);

	int target_only = 0;
	for(int i=2; i<target_filecount; i++){
		strcpy(targetfile_path, dirFile_selected);
		strcat(targetfile_path, slash);
		strcat(targetfile_path, target_namelist[i]->d_name);
		stat(targetfile_path, &st_target);
		
		for(int s=2; s<source_filecount; s++){
			strcpy(sourcefile_path, dirFileList_Candidate[0]);
			strcat(sourcefile_path, slash);
			strcat(sourcefile_path, source_namelist[s]->d_name);
			stat(sourcefile_path, &st_source);

			if(strcmp(target_namelist[i]->d_name, source_namelist[s]->d_name) == 0){
				target_only = 1;
				if(S_ISREG(st_target.st_mode) && S_ISREG(st_source.st_mode)){} // exclude overlap : do nothing(already compared)
				else if(S_ISREG(st_target.st_mode) && S_ISDIR(st_source.st_mode)){
					printf("File %s is a directory while file %s is a regular file\n", sourcefile_path, targetfile_path);
				}
				else if(S_ISDIR(st_target.st_mode) && S_ISREG(st_source.st_mode)){
					printf("File %s is a regular file while file %s is a directory\n", sourcefile_path, targetfile_path);
				}
			}
		}
		if(target_only == 0){
			printf("Only in %s : %s\n", dirFile_selected, target_namelist[i]->d_name);
		}
		target_only = 0;
		
	}


}

int getTotalLine(char* filePath){
	FILE* fp;
	int line = 0;
	char c;
	fp = fopen(filePath, "r");
	while( (c = fgetc(fp) ) != EOF){
		if(c == '\n')
			line++;
	}
	fclose(fp);
	return line;
}

//void LCS(char* sourcePath, char* targetPath, int opt_type){
void LCS(char* sourcePath, char* targetPath){
//	printf("opt_type : %d\n", opt_type);
	FILE* fp_source; 
	FILE* fp_target;
	fp_source = fopen(sourcePath, "r");
	fp_target = fopen(targetPath, "r");
	int sr_LineCount = getTotalLine(sourcePath);
	int tg_LineCount = getTotalLine(targetPath);
	
	int row = sr_LineCount + 1;
	int col = tg_LineCount + 1;

	int** table = (int**)malloc(sizeof(int*) * (row+1));
	int** tmp_table = (int**)malloc(sizeof(int*) * (row+1));
	
	for(int i=0; i<row+1; i++){
		table[i] = (int*)malloc(sizeof(int) * (col+1));
		tmp_table[i] = (int*)malloc(sizeof(int) * (col+1));
	}

	for(int i=0; i<row+1; i++){
		for(int j=0; j<col+1; j++){
			table[i][j] = 0;
			tmp_table[i][j] = 0;
		}
	}
	
	char* sr_line = (char*)malloc(1024);
	char* tg_line = (char*)malloc(1024);

	for(int i=1; i<row+1; i++){
		fgets(sr_line, 1024, fp_source);
		for(int j=1; j<col+1; j++){
			fgets(tg_line, 1024, fp_target);
			
			if(strcmp(sr_line, tg_line) == 0){
				table[i][j] = table[i-1][j-1] + 1;
				tmp_table[i][j] = 1;
			}
			else{
				table[i][j] = max(table[i-1][j], table[i][j-1]);
			}

			for(int k=0; k<1024; k++){
				tg_line[k] = '\0';
			}
		}
		fseek(fp_target, 0, SEEK_SET);
		for(int t=0; t<1024; t++){
			sr_line[t] = '\0';
		}
	}
/*
	for(int i=0; i<row+1; i++){
		for(int j=0; j<col+1; j++){
			printf("%d ", tmp_table[i][j]);
		}
		printf("\n");
	}
	printf("\n");
*/
	

	int diff_count = 0;
	char* diff_str[1024];
	for(int i=0; i<1024; i++){
		*(diff_str + i) = (char*)malloc(1024);
	}


	int left_count = 0;
	int up_count = 0;
	int i = row;
	int j = col;
	int i_before = row;
	int j_before = col;
	int diagonal_count = 0;
	while(1){
		char tmp[1024];
		if (i==0) {
			if(diagonal_count == 1){
				sprintf(tmp, "1,%dc1,%d", row-1, col-1);
				strcpy(diff_str[diff_count++], tmp);
				for(int i=0; i<1024; i++)
					tmp[i] = '\0';
				break;
			}
			if (j==1){
				sprintf(tmp, "0a1");
				strcpy(diff_str[diff_count++], tmp);
				for(int i=0; i<1024; i++)
					tmp[i] = '\0';
			}
			else if (j>1){
				sprintf(tmp, "0a1,%d", j);
				strcpy(diff_str[diff_count++], tmp);
				for(int i=0; i<1024; i++)
					tmp[i] = '\0';
			}
			break;
		}
		if (j==0) {
			if (i==1){
				sprintf(tmp, "1d0");
				strcpy(diff_str[diff_count++], tmp);
				for(int i=0; i<1024; i++)
					tmp[i] = '\0';
			}
			else if(i>1){
				sprintf(tmp, "1,%dd0", i);
				strcpy(diff_str[diff_count++], tmp);
				for(int i=0; i<1024; i++)
					tmp[i] = '\0';
			}
			break;
		}

		if(tmp_table[i][j] != 1){
			if(table[i][j] == table[i-1][j]){
				i--;
				up_count++;
			}
			else if(table[i][j] == table[i][j-1]){
				j--;
				left_count++;
			}
		}
		else{ // if tmp_table[i][j] == 1
			diagonal_count++;
			if(up_count == 0 && left_count != 0){
				// a
				if((j_before - j) == 1){
					sprintf(tmp, "%da%d", i_before, j_before);
					strcpy(diff_str[diff_count++], tmp);
					for(int i=0; i<1024; i++)
						tmp[i] = '\0';

				}
				else if((j_before - j) > 1){
					sprintf(tmp, "%da%d,%d", i_before, j+1, j_before);
					strcpy(diff_str[diff_count++], tmp);
					for(int i=0; i<1024; i++)
						tmp[i] = '\0';

				}
				j_before = j;
				
			}

			if(up_count != 0 && left_count == 0){
				// d
				if((i_before - i) == 1){
					sprintf(tmp, "%dd%d", i_before, j_before);
					strcpy(diff_str[diff_count++], tmp);
					for(int i=0; i<1024; i++)
						tmp[i] = '\0';

				}
				else if((i_before - i) > 1){
					sprintf(tmp, "%d,%dd%d", i+1, i_before, j_before);
					strcpy(diff_str[diff_count++], tmp);
					for(int i=0; i<1024; i++)
						tmp[i] = '\0';

				}
				i_before = i;
			}

			if(up_count != 0 && left_count != 0){
				// c
				char temp[1024];
				for(int i=0; i<1024; i++)
					temp[i] = '\0';

				if((i_before - i) == 1){
					sprintf(temp, "%d", i_before);
				}
				else{
					sprintf(temp, "%d,%d", i+1, i_before);
				}
				strcat(temp, "c");
				strcpy(tmp, temp); // %dc  || %d,%dc
				
				for(int i=0; i<1024; i++)
					temp[i] = '\0';

				
				if((j_before - j) == 1){
					sprintf(temp, "%d", j_before);
				}
				else{
					sprintf(temp, "%d,%d", j+1, j_before);
				}
				strcat(tmp, temp);
				strcpy(diff_str[diff_count++], tmp);
				for(int i=0; i<1024; i++)
					tmp[i] = '\0';
				i_before = i;
				j_before = j;
			}

			i--;
			j--;
			i_before--;
			j_before--;

			up_count = 0;
			left_count = 0;
		}
	}

	char* front[diff_count];
	char* back[diff_count];
	char* mid[diff_count];
	for(int j=0; j<diff_count; j++){
		*(front + j) = (char*)malloc(30);
		*(back + j) = (char*)malloc(30);
		*(mid + j) = (char*)malloc(30);
	}

	for(int i=0; i<diff_count ; i++){
		char* tmp = (char*)malloc(1024);
		if(strchr(diff_str[i], 'c') != NULL){
			strcpy(mid[i], "c");
			tmp = strtok(diff_str[i], "c");
			strcpy(front[i], tmp);
			tmp = strtok(NULL, "c");
			strcpy(back[i], tmp);
		}
		if(strchr(diff_str[i], 'a') != NULL){
			strcpy(mid[i], "a");
			tmp = strtok(diff_str[i], "a");
			strcpy(front[i], tmp);
			tmp = strtok(NULL, "a");
			strcpy(back[i], tmp);
	
		}
		if(strchr(diff_str[i], 'd') != NULL){
			strcpy(mid[i], "d");
			tmp = strtok(diff_str[i], "d");
			strcpy(front[i], tmp);
			tmp = strtok(NULL, "d");
			strcpy(back[i], tmp);
		}
		for(int i=0; i<1024; i++)
			tmp[i] = '\0';

	}
/*
	for(int i=0; i<diff_count; i++){
		printf("front[%d] : %s, back[%d] : %s, mid[%d] : %s\n", i, front[i], i, back[i], i, mid[i]);
		
	}
*/
	for(int i=diff_count-1; i>=0; i--){
		printf("%s%s%s\n", front[i], mid[i], back[i]);
		int min, max;
		char* tmp = (char*)malloc(10);
		if(strcmp(mid[i], "a") == 0){
			if(strchr(front[i], ',') != NULL){ // front
				tmp = strtok(front[i], ",");
				min = atoi(tmp);
				tmp = strtok(NULL, ",");
				max = atoi(tmp);
				printManyLine(sourcePath, min, max, 0);
//				printf("max : %d, sr_LineCount : %d\n", max, sr_LineCount);
				if(max >= sr_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}
			}
			else{
				printOneLine(sourcePath, atoi(front[i]), 0);
				if(atoi(front[i]) >= sr_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}
			}

			if(strchr(back[i], ',') != NULL){ // back
				tmp = strtok(back[i], ",");
				min = atoi(tmp);
				tmp = strtok(NULL, ",");
				max = atoi(tmp);
				printManyLine(targetPath, min, max, 1);
//				printf("max : %d, tg_LineCount : %d\n", max, tg_LineCount);
				if(max >= tg_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}
			else{
				printOneLine(targetPath, atoi(back[i]), 1);
				if(atoi(front[i]) >= tg_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}

		}
		else if(strcmp(mid[i], "d") == 0){
			if(strchr(front[i], ',') != NULL){ // front
				tmp = strtok(front[i], ",");
				min = atoi(tmp);
				tmp = strtok(NULL, ",");
				max = atoi(tmp);
				printManyLine(sourcePath, min, max, 0);	
//				printf("max : %d, sr_LineCount : %d\n", max, sr_LineCount);
				if(max >= sr_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}
			else{
				printOneLine(sourcePath, atoi(front[i]), 0);
				if(atoi(front[i]) >= sr_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}

			if(strchr(back[i], ',') != NULL){ // back
				tmp = strtok(back[i], ",");
				min = atoi(tmp);
				tmp = strtok(NULL, ",");
				max = atoi(tmp);
				printManyLine(targetPath, min, max, 1);
//				printf("max : %d, tg_LineCount : %d\n", max, tg_LineCount);
				if(max >= tg_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}
			else{
				printOneLine(targetPath, atoi(back[i]), 1);
				if(atoi(front[i]) >= tg_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}



		}
		else if(strcmp(mid[i], "c") == 0){
			if(strchr(front[i], ',') != NULL){ // front
				tmp = strtok(front[i], ",");
				min = atoi(tmp);
				tmp = strtok(NULL, ",");
				max = atoi(tmp);
				printManyLine(sourcePath, min, max, 0);
//				printf("max : %d, sr_LineCount : %d\n", max, sr_LineCount);
				if(max >= sr_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}
			else{
				printOneLine(sourcePath, atoi(front[i]), 0);
				if(atoi(front[i]) >= sr_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}

			printf("---\n");

			if(strchr(back[i], ',') != NULL){ // back
				tmp = strtok(back[i], ",");
				min = atoi(tmp);
				tmp = strtok(NULL, ",");
				max = atoi(tmp);
				printManyLine(targetPath, min, max, 1);
//				printf("max : %d, tg_LineCount : %d\n", max, tg_LineCount);
				if(max >= tg_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}
			}
			else{
				printOneLine(targetPath, atoi(back[i]), 1);
				if(atoi(front[i]) >= tg_LineCount+1){
					printf("\n\\No newline at end of file\n");
				}

			}


		}
	}
	printf("\n");
}

void printOneLine(char* fileName, int lineNum, int type){
	FILE* fp;
	int linecount = 0;
	char buffer[1024];
	if((fp = fopen(fileName, "r")) == NULL){
		fprintf(stderr, "file open error : %s\n", fileName);
		return;
	}
	while(fgets(buffer, 1024, fp) != NULL){
		linecount++;
		if(linecount == lineNum){
			if(type == 0){
				printf("< ");
			}
			else if(type == 1){
				printf("> ");
			}
			printf("%s", buffer);
			break;
		}
	}
	fclose(fp);
	return;
}

void printManyLine(char* fileName, int min, int max, int type){ 
	FILE* fp;
	int linecount = 0;
	char* buffer_set[100];
	for(int i=0; i<100; i++){
		*(buffer_set + i) = (char*)malloc(1024);
	}
	int buf_index = 0;

	if((fp = fopen(fileName, "r")) == NULL){
		fprintf(stderr, "file open error : %s\n", fileName);
		return;
	}

	while(fgets(buffer_set[linecount], 1024, fp) != NULL){
		linecount++;
		if( (min <= linecount) && (linecount <= max) ){
			if(type == 0){
				printf("< ");
			}
			else if(type == 1){
				printf("> ");
			}
			printf("%s",buffer_set[linecount-1]);
		}
	}
	fclose(fp);
	return;
}










