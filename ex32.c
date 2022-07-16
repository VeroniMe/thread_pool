// Veronika Merkulova 336249362

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define EXCELLENT 1
#define WRONG 2
#define SIMILAR 3
#define NO_C_FILE 4
#define COMPILATION_ERROR 5
#define TIMEOUT 6
#define OTHER 7

/*Check if path is directory*/
int is_dir(const char* path) {
    struct stat path_stat;
    if( stat(path, &path_stat) == -1) {
        if(write(1,"Error in: stat\n",strlen("Error in: stat\n")) < 0)
		{
			exit(-1);
		}
    };
    return S_ISDIR(path_stat.st_mode);
}

int execute_c(char input[3][150], char* path_of_comp_program) { 
    
    char returned = 0;
    char buf[150];
    char path_to_compare[150];
    time_t start_t, end_t;
    double diff_t;

    getcwd(buf,150);
    strcpy(path_to_compare, buf);
    strcat(buf, "/");
    strcat(buf, "a.out");
    strcat(path_to_compare, "/");
    strcat(path_to_compare, "out.txt");

    if(!access(buf, F_OK)) { //a.out exist - if not -> compile error occurs        
        /*Open out.txt file for output of user programm + output rediraction*/
        int fdin;
        fdin = open("out.txt", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if(fdin < 0) {
            if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		        {
			        exit(-1);
		        }
        }
        dup2(fdin, 1);

        char in[150];
        strcpy(in, input[1]);
        char* args[4] = { "./a.out\0", "<\0", input[1], NULL};
        /*child process for execute file of user*/
        pid_t child1 = fork();
        if(child1 == -1) {
            if(write(1,"Error in: fork\n",strlen("Error in: fork\n")) < 0)
		    {
			    exit(-1);
		    }
        } else if(child1 == 0) { //child
            int sts = execvp(args[0], args);
                if(sts < 0) {
                if(write(1,"Error in: execvp\n",strlen("Error in: execvp\n")) < 0)
		            {
			             exit(-1);
		            }
                }
        } else {
         //parent
            if (waitpid(child1, NULL, 0) != child1) {
			    if(write(1,"Error in: waitpid\n",strlen("Error in: waitpid\n")) < 0)
		        {
			        exit(-1);
		        }
            }
        }
        dup2(0, fdin);
        /*child process for execute files to compare 2 files*/
        pid_t child2 = fork();
        strcat(path_of_comp_program, "/");
        strcat(path_of_comp_program, "comp.out");
        char* args2[5] = { path_of_comp_program, input[2], path_to_compare, NULL};
        if(child2 == -1) {
            if(write(1,"Error in: fork\n",strlen("Error in: fork\n")) < 0)
		    {
			    exit(-1);
		    }
        } else if(child2 == 0) { //child
            int sts = execvp(args2[0], args2);
            if(sts < 0) {
                if(write(1,"Error in: execvp\n",strlen("Error in: execvp\n")) < 0)
		        {
			        exit(-1);
		        }
            }
        } else {
         //parent
            int status=0;
            time(&start_t);
            if (waitpid(child2, &status, 0) != child2) {
			     if(write(1,"Error in: wait\n",strlen("Error in: wait\n")) < 0)
		        {
			        exit(-1);
		        }
            }
            time(&end_t);
            diff_t = difftime(end_t, start_t);       
            if (WIFEXITED(status)){
                returned = (char)WEXITSTATUS(status);
            }
            if( diff_t > 5) {
                returned = TIMEOUT;
            }
        }
        close(fdin);
        if(remove("out.txt") != 0) {
            if(write(1,"Error in: remove\n",strlen("Error in: remove\n")) < 0)
		    {
			    exit(-1);
		    }
        }
        if(remove("a.out") != 0) {
            if(write(1,"Error in: remove\n",strlen("Error in: remove\n")) < 0)
		    {
			    exit(-1);
		    }
        }
    } else {
        returned = COMPILATION_ERROR;
    }    
    return returned;
}

int compile_c(char* file, char conf[3][150], char* path){

    int ret = 0;
    char buf[150];
    char buf2[150];
    int ch = 0;
    int sts;
    char* args[] = {"gcc", file, NULL};
    /*child process for compile user .c file*/
    pid_t child = fork();
    if( child == -1) {
        if(write(1,"Error in: fork\n",strlen("Error in: fork\n")) < 0)
		{
			exit(-1);
		}
    } else if(child == 0) { //child
        sts = execvp("gcc", args);
        if(sts < 0) {
            if(write(1,"Error in: execvp\n",strlen("Error in: execvp\n")) < 0)
		    {
			    exit(-1);
		    }
        }
    } else {
        //parent
        if (waitpid(child, NULL, 0) != child) {
			 if(write(1,"Error in: execvp\n",strlen("Error in: execvp\n")) < 0)
		    {
			    exit(-1);
		    }
        }
    }
    ret = execute_c(conf, path);
    return ret;
}



int find_c(const char* p, char conf[3][150], char* path) {

    int returned = 0;
    DIR* di;
    struct dirent* dir;
    int found = 0;
    char ptr1[100];
    char ptr2[100];
    char ptr3[100];
    char temp_p[150];
    strcpy(temp_p, p);

    if ( (di = opendir(temp_p)) == NULL) {
         if(write(1,"Error in: opendir\n",strlen("Error in: opendir\n")) < 0)
		{
			exit(-1);
		}
    }
    while( (dir = readdir(di)) != NULL ) {       

        strcat(temp_p, "/");
        strcat(temp_p, dir->d_name);

        if( strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 ) {
        if(!is_dir(temp_p)) { //its file
            size_t len = strlen(dir->d_name);
            char name_of_file[len];
            strcpy(name_of_file, dir->d_name);

            if(name_of_file[len-1] == 'c' && name_of_file[len-2] == '.') {
                //its c-file
                found = 1;
                int status = chdir(p);
                if (status == -1) {
                    if(write(1,"Error in: chdir\n",strlen("Error in: chdir\n")) < 0)
		            {
			            exit(-1);
		            }
                };
                returned = compile_c(name_of_file, conf, path);
                /*RETURN BACK*/
                int st = 0;
                st = chdir("..");
                if (st == -1) {
                    if(write(1,"Error in: chdir\n",strlen("Error in: chdir\n")) < 0)
		            {
			            exit(-1);
		            }
                    
                };
                char b[150];
            }            
        }
        }
        strcpy(temp_p, p);
    }
    if ( closedir(di) == -1) {
        if(write(1,"Error in: closedir\n",strlen("Error in: closedir\n")) < 0)
		{
			exit(-1);
		}
    }
    if(found == 0) {
        returned = NO_C_FILE;
    }
    return returned;
}

int main(int argc, char *argv[]) {

    int fdin1;
    size_t SIZE = 1;    
    char config[3][150];
    char buf1 [SIZE+1];
    char s[150];    
    ssize_t char1;
    s[0] = '\0';

    fdin1 = open(argv[1], O_RDONLY);
    if (fdin1 < 0) /* means file open did not take place */ 
	{                
		if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		{
			exit(-1);
		} 
	}
    int i = 0;
    while ((char1 = read(fdin1, buf1, SIZE)) > 0 )
    {
        buf1[1] = '\0';
        while (strcmp(buf1, "\n") != 0)
        {
            buf1[1] = '\0';
            strcat(s, buf1);    
            char1 = read(fdin1, buf1, SIZE);
            if (char1 == -1) {
                if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		        {
			        exit(-1);
		        }
            }      
        }
        strcpy(config[i], s);
        i++;        
        s[0] = '\0';
        char1 = read(fdin1, buf1, SIZE);
        if (char1 == -1) {
            if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		    {
			    exit(-1);
		    }
        }        
        strcat(s, buf1); 
    }
    if (char1 == -1) {
        if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		{
			exit(-1);
		}
    }
	close(fdin1);

    char buf[150];
    char path[150];
    char temp[150];
    strcpy( path, getcwd(buf,150) );
    strcpy( temp, getcwd(buf,150) );
    int j = 0;

    for( j; j<3;j++){
        if( config[j][0] != '/') {
           strcat( temp, "/");
           strcat(temp, config[j]);
           strcpy(config[j], temp);
           strcpy( temp, path );
        }
    }

    /**** I/O Redirection ****/
    //INPUT:
    int fdin;
    fdin = open(config[1], O_RDONLY);
    if(fdin < 0) {
        if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		{
			exit(-1);
		}
    }
    dup2(fdin, 0);
    close(fdin);
    //ERRORS:
    int fd_errors;
    fd_errors = open("errors.txt", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd_errors < 0) {
        if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		{
			exit(-1);
		}
    }
    dup2(fd_errors, 2);
    close(fd_errors);
    /*******************************/ 
    /**** OPEN FILE FOR RESULTS ****/
    int fd_result;
    fd_result = open("result.csv", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd_result < 0) {
        if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		{
			exit(-1);
		}
    }
    /*******************************/

    DIR* dip;
    struct dirent* dit;    
    if ( (dip = opendir(config[0])) == NULL) {
        if(write(1,"Error in: opedir\n",strlen("Error in: opendir\n")) < 0)
		{
			exit(-1);
		}
    }
    while ( (dit = readdir(dip)) != NULL)
    {
        char temp_path[150];
        strcpy(temp_path, config[0]);
        strcat(temp_path, "/");
        strcat(temp_path, dit->d_name);

        if(strcmp(dit->d_name, ".") != 0 && strcmp(dit->d_name, "..") != 0 ) {
        if(is_dir(temp_path)) {  //if its directory (to user folder with file .c)
            //pass it to function for find and executing c
            int i = find_c(temp_path, config, path);

            if(write(fd_result,dit->d_name,strlen(dit->d_name)) < 0)
		    {
			    exit(-1);
		    }
            if(write(fd_result,",",1) < 0)
		    {
			    exit(-1);
		    }
            switch (i)
            {
            case 1:
                if(write(fd_result,"100,EXCELLENT,\n",strlen("100,EXCELLENT,\n")) < 0)
		        {
			        exit(-1);
		        }
                break;
            case 2:
                if(write(fd_result,"50,WRONG,\n",strlen("50,WRONG,\n")) < 0)
		        {
			        exit(-1);
		        }
                break;
            case 3:
                if(write(fd_result,"75,SIMILAR,\n",strlen("75,SIMILAR,\n")) < 0)
		        {
			        exit(-1);
		        }
                break;
            case 4:
                if(write(fd_result,"0,NO_C_FILE,\n",strlen("0,NO_C_FILE,\n")) < 0)
		        {
			        exit(-1);
		        }
                break;
            case 5:
                if(write(fd_result,"10,COMPILATION_ERROR,\n",strlen("10,COMPILATION_ERROR,\n")) < 0)
		        {
			        exit(-1);
		        }
                break;
            case 6:
                if(write(fd_result,"20,TIMEOUT,\n",strlen("20,TIMEOUT,\n")) < 0)
		        {
			        exit(-1);
		        }
                break;            
            default:
                break;
            }
        }
        }
        strcpy(temp_path, config[0]);
    }    
    if ( closedir(dip) == -1){
        if(write(1,"Error in: closedir\n",strlen("Error in: closedir\n")) < 0)
		{
			exit(-1);
		}
    }
    close(fd_result);
}