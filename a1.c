#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define ERR_MSG(DBG_MSG){\
perror(DBG_MSG);\
}

#define MAX 1024

bool comp_str(char *s1, char *s2){
	bool ok = true;
	for(int j = 0; j < strlen(s2); j++){
		if(s1[j] != s2[j])
			ok = false;
	}
	return ok;
}

void parse_fil(char* dir_name){
	int fd;
	if((fd = open(dir_name,O_RDONLY)) < 0)
		exit(4);
	char magic[3];
	lseek(fd, -2, SEEK_END);
	read(fd, magic, 2);
	magic[2] ='\0';
	if(strcmp(magic,"Zv") != 0){
		printf("ERROR\n");
		printf("wrong magic");
		exit(5);	
	}
	int16_t marime;
	lseek(fd, -4, SEEK_END);
	read(fd, &marime, 2);
	marime = marime * (-1);	

	int16_t vers;
	lseek(fd, marime, SEEK_END);
	read(fd, &vers, 2);
	if((26 > vers) || (vers > 101)){
		printf("ERROR\n");
		printf("wrong version");
		exit(6);
	}
	int8_t nr_sections;
	lseek(fd, marime + 2, SEEK_END);
	read(fd, &nr_sections, 1);
	if((8 > nr_sections) || (nr_sections >15)){
		printf("ERROR\n");
		printf("wrong sect_nr");
		exit(7);
	}
	for(int8_t i=1; i <= nr_sections; i++){
		char snume[14];			
		read(fd, snume, 14);
		int8_t tip;
		read(fd, &tip, 1);
		if ((tip != 10) && (tip != 28) && (tip != 13) && (tip != 34))
		{
			printf("ERROR\n");
			printf("wrong sect_types");
			exit(8);
		}
		int32_t adr;
		read(fd, &adr, 4);
		int32_t siz;
		read(fd, &siz, 4);
	}
	lseek(fd, marime + 3, SEEK_END);
	printf("SUCCESS\n");
	printf("version=%d\n",vers);
	printf("nr_sections=%d\n",nr_sections);
	for(int8_t i=1; i <= nr_sections; i++){
		printf("section%d: " ,i);
		char snume[14];			
		read(fd, snume, 14);
		printf("%s ", snume);
		int8_t tip;
		read(fd, &tip, 1);
		printf("%d ", tip);
		int32_t adr;
		read(fd, &adr, 4);
		int32_t siz;
		read(fd, &siz, 4);
		printf("%d\n",siz);
	}
	close(fd);
}

void extr(char *path, int sct, int ln){
	int fd;
	if((fd = open(path, O_RDONLY)) < 0){
		printf("ERROR/n");
		printf("invalid file adr\n");
		exit(4);	
	}
	char magic[3];
	lseek(fd, -2, SEEK_END);
	read(fd, magic, 2);
	magic[2] = '\0';
	if(strcmp(magic,"Zv") != 0){
		printf("ERROR\n");
		printf("invalid file mag");
		exit(5);	
	}
	int16_t marime;
	lseek(fd, -4, SEEK_END);
	read(fd, &marime, 2);
	marime = marime * (-1);	

	int16_t vers;
	lseek(fd, marime, SEEK_END);
	read(fd, &vers, 2);
	if((26 > vers) || (vers > 101)){
		printf("ERROR\n");
		printf("invalid file ve");
		exit(6);
	}
	int8_t nr_sections;
	lseek(fd, marime + 2, SEEK_END);
	read(fd, &nr_sections, 1);
	if((8 > nr_sections) || (nr_sections >15)){
		printf("ERROR\n");
		printf("invalid file sec");
		exit(7);
	}
	if(nr_sections < sct){
		printf("ERROR\n");
		printf("invalid section");
		exit(8);		
	}
	for(int8_t i=1; i <= nr_sections; i++){
		char snume[14];
		read(fd, snume, 14);
		int8_t tip;
		read(fd, &tip, 1);
		if ((tip != 10) && (tip != 28) && (tip != 13) && (tip != 34))
		{
			printf("ERROR\n");
			printf("invalid file tip");
			exit(9);
		}	
		int32_t adr;
		read(fd, &adr, 4);
		int32_t siz;
		read(fd, &siz, 4);
	}
	lseek(fd, marime + 3, SEEK_END);
	lseek(fd, (sct-1) * 23, SEEK_CUR);
	lseek(fd, 15, SEEK_CUR);
	int32_t adr;
	read(fd, &adr, 4);
	int32_t siz;
	read(fd, &siz, 4);
	lseek(fd, adr, SEEK_SET);
	int32_t cap = 0;
	int16_t countLines = 1;
	while(countLines < ln){
		if(cap > siz){
			printf("ERROR\n");
			printf("invalid line");
			exit(10);
		}
		char c;
		read(fd, &c, 1);
		cap++;
		if (c == '\r')
			countLines++;
	}
	char c = '1';
	read(fd, &c, 1);
	printf("SUCCESS\n");
	if ((c != '\r') && (c != '\n'))
		printf("%c",c);
	while (c != '\r'){
		read(fd, &c, 1);
		printf("%c",c);
	}
	close(fd);
}

char* getPath(char* str){
	char s[150]; 
	strcpy(s, str);
	char *ptr = strtok(s, "=");
	ptr = strtok(NULL, "=");
	return ptr;
}

char* getNameCond(char* str){
	char s[50]; 
	strcpy(s, str);
	char *ptr = strtok(s, "=");
	ptr = strtok(NULL, "=");
	return ptr;
}

char* getSizeCond(char* str){
	char s[30]; 
	strcpy(s, str);
	char *ptr = strtok(s, "=");
	ptr = strtok(NULL, "=");
	return ptr;
}

void listDir(char* dir_name, int siz) {
	DIR* dir;
	struct dirent *dirEntry;
	struct stat inode;
	char name[1024];

	dir = opendir(dir_name);
	if (dir == 0) {
		ERR_MSG("ERROR");
		exit(3);
	}
	while ((dirEntry = readdir(dir)) != NULL) {
		snprintf(name, MAX, "%s/%s", dir_name, dirEntry->d_name);
		lstat(name, &inode);
		if(inode.st_size >= siz)
			if((strcmp(dirEntry->d_name,".") != 0) && (strcmp(dirEntry->d_name,"..") != 0))
			printf(" %s\n", name);
	}
	closedir(dir);
}

void rec_listDir(char* dir_name, int siz) {
	DIR* dir;
	struct stat inode;
	struct dirent *dirEntry;
	char name[1024];

	dir = opendir(dir_name);
	if (dir == 0) {
		ERR_MSG("ERROR");
		exit(3);
	}
	while ((dirEntry = readdir(dir)) != NULL) {
		snprintf(name, MAX, "%s/%s", dir_name, dirEntry->d_name);
		lstat(name, &inode);		
		if (S_ISDIR(inode.st_mode) && (strcmp(dirEntry->d_name,".") != 0) && (strcmp(dirEntry->d_name,"..") != 0))
			rec_listDir(name,siz);
		if(inode.st_size >= siz)
			if((strcmp(dirEntry->d_name,".") != 0) && (strcmp(dirEntry->d_name,"..") != 0))
			printf(" %s\n", name);
	}
	closedir(dir);
}

void listDirName(char* dir_name, char nme[]) {
	DIR* dir;
	struct dirent *dirEntry;
	char name[1024];

	dir = opendir(dir_name);
	if (dir == 0) {
		ERR_MSG("ERROR");
		exit(3);
	}
	while ((dirEntry = readdir(dir)) != NULL) {
		snprintf(name, MAX, "%s/%s", dir_name, dirEntry->d_name);
		if(comp_str(dirEntry->d_name, nme) == true)
			if((strcmp(dirEntry->d_name,".") != 0) && (strcmp(dirEntry->d_name,"..") != 0))
			printf(" %s\n", name);
	}
	closedir(dir);
}

void rec_listDirName(char* dir_name, char nme[]) {
	DIR* dir;
	struct stat inode;
	struct dirent *dirEntry;
	char name[1024];
	dir = opendir(dir_name);
	if (dir == 0) {
		ERR_MSG("ERROR");
		exit(3);
	}
	while ((dirEntry = readdir(dir)) != NULL) {
		snprintf(name, MAX, "%s/%s", dir_name, dirEntry->d_name);
		lstat(name, &inode);
		if (S_ISDIR(inode.st_mode) && (strcmp(dirEntry->d_name,".") != 0) && (strcmp(dirEntry->d_name,"..") != 0))
			rec_listDirName(name, nme);
		if(comp_str(dirEntry->d_name, nme) == true)
			if((strcmp(dirEntry->d_name,".") != 0) && (strcmp(dirEntry->d_name,"..") != 0))
				printf(" %s\n", name);
	}
	closedir(dir);
}

int main(int argc, char** argv) {
	if (argc == 0){
		printf("no arguments");		
		exit(1);
	}
	if (0 == strcmp(argv[1], "variant")) {
		printf("27620\n");
		return 0;
	}
	if (0 == strcmp(argv[1], "list")) {
		if(argc > 3){
			char path[150];
			bool rec = false; bool condNr = false; bool condNume = false; 
			int siz; char num[30];
			if (strcmp(argv[2],"recursive") == 0){
				if(strstr(argv[3],"size_greater=") != NULL){
					condNr = true;
					char nr[10];
					strcpy(nr,getSizeCond(argv[3]));
					siz = atoi(nr);
				}else
					if(strstr(argv[3],"name_starts_with=") != NULL){
						condNume = true;
						strcpy(num,getNameCond(argv[3]));
					}
			}
			if (strcmp(argv[2],"recursive") != 0){	
					if(strstr(argv[2],"size_greater") != NULL){
						condNr = true;
						char nr[10];
						strcpy(nr,getSizeCond(argv[2]));
						siz = atoi(nr);
					}else
					   if(strstr(argv[2],"name_starts_with=") != NULL){
						condNume = true;
						strcpy(num,getNameCond(argv[2]));
						}
			}	
			if((strcmp(argv[2],"recursive") == 0) || (strcmp(argv[3],"recursive") == 0)){
				if((condNr == true) || (condNume == true))
					strcpy(path,getPath(argv[4]));
				else
					strcpy(path,getPath(argv[3]));
				rec = true;
			}else
				if((condNr == true) || (condNume == true))
					strcpy(path,getPath(argv[3]));
				else
					strcpy(path,getPath(argv[2]));
			struct stat fileMetadata;
			if (stat(path, &fileMetadata) < 0) {
				ERR_MSG("ERROR");
				exit(2);
			}	
			if (S_ISDIR(fileMetadata.st_mode)) {
					printf("SUCCESS\n");
					if(condNume == true){
						if( rec == false)
							listDirName(path, num);
						else
							rec_listDirName(path, num);
					}else
						if (condNr == false){
							if( rec == false)
								listDir(path, 0);
							else
								rec_listDir(path, 0);
						}
						else{
							if( rec == false)
								listDir(path, siz);
							else
								rec_listDir(path, siz);
						}
			}
		}else{
			char path[150];
			strcpy(path,getPath(argv[2]));
			struct stat fileMetadata;
			if (stat(path, &fileMetadata) < 0) {
				ERR_MSG("ERROR");
				exit(2);
			}	
			if (S_ISDIR(fileMetadata.st_mode)) {
					printf("SUCCESS\n");
					listDir(path, 0);
			}
		}
	}	

	if(strcmp(argv[1],"parse") == 0){
		char path[150] ="";
		strcpy(path,getPath(argv[2]));
		parse_fil(path);		
	}
	if(strcmp(argv[1],"extract") == 0){
		char path[150] ="";
		char line[3];
		char sect[2];
		strcpy(path,getPath(argv[2]));
		strcpy(sect,getPath(argv[3]));
		strcpy(line,getPath(argv[4]));
		int8_t sct = atoi(sect);
		int16_t ln = atoi(line);
		extr(path, sct, ln);
	}
}
