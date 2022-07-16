// Veronika Merkulova 336249362

#include <unistd.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define OFFSET 32 // for checking uper and lower case
#define NOT_EQUAL 2
#define EQUAL 1
#define PROMOTE1 5
#define PROMOTE2 4
#define SIMILAR 3

int compare(char* b1, char* b2)
{
    if( (strcmp(b1, b2) == 0) || ( abs(b1[0] - b2[0]) == OFFSET) )
    {
        return EQUAL;
    } else if ( (strcmp(b1, " ") == 0) || (strcmp(b1, "\n") == 0) ) {
        return PROMOTE1;
    } else if ( (strcmp(b2, " ") == 0) || (strcmp(b2, "\n") == 0) ) {
        return PROMOTE2;
    } else {
        return NOT_EQUAL;
    }    
}
int main(int argc, char *argv[]) {

    int fdin1;
    int fdin2;
    size_t SIZE = 1;
    char buf1 [SIZE+1];    
    char buf2 [SIZE+1];
    ssize_t char1, num_of_bytes1;
    ssize_t char2, num_of_bytes2;
    
    // identity: compare this variable to num of bytes that were red:
    // if files equal => i-ty == num_of bytes1 == num_of bytes1    
    ssize_t identity;                                   
    int result = 0;

    fdin1 = open(argv[1], O_RDONLY);
    if (fdin1 < 0) /* means file open did not take place */ 
	{                
		if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		{
			exit(-1);
		} 
	}
    fdin2 = open(argv[2], O_RDONLY);
    if (fdin2 < 0) /* means file open did not take place */ 
	{                
		if(write(1,"Error in: open\n",strlen("Error in: open\n")) < 0)
		{
			exit(-1);
		}  
	}    
    //printf("after open\n");
    char1 = read(fdin1, buf1, SIZE);
    if (char1 == -1) {
        if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		{
			exit(-1);
		} 
    }
    char2 = read(fdin2, buf2, SIZE);  
    if (char2 == -1) {
        if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		{
			exit(-1);
		}
    }
    ++num_of_bytes1;
    ++num_of_bytes2;  
    ++identity; 
    while ( char1>0 && char2>0 )
    {
        buf1[1] = '\0';
        buf2[1] = '\0';
        int i = compare(buf1, buf2);
        switch (i)
        {
            case EQUAL:         //chars are equal
                char1 = read(fdin1, buf1, SIZE);
                if (char1 == -1) {
                    if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		            {
			            exit(-1);
		            }
                }
                char2 = read(fdin2, buf2, SIZE);  
                if (char2 == -1) {
                    if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		            {
			            exit(-1);
		            }
                }
                ++num_of_bytes1;
                ++num_of_bytes2;   
                ++identity;           
                break;
            case PROMOTE1://promote file1
                char1 = read(fdin1, buf1, SIZE);
                if (char1 == -1) {
                    if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		            {
			            exit(-1);
		            }
                }
                ++num_of_bytes1;
                break;
            case PROMOTE2://promote file2
                char2 = read(fdin2, buf2, SIZE);
                if (char2 == -1) {
                    if(write(1,"Error in: read\n",strlen("Error in: read\n")) < 0)
		            {
			            exit(-1);
		            }
                }
                ++num_of_bytes2;
                break;  
            default:
                result = NOT_EQUAL;
                char1 = char2 = 0;
                break;
        }                  
    }
    if (result != 2) {
        if ( (char1 == 0 && char2 != 0 ) || (char1 != 0 && char2 == 0 ) ) {
            int test = SIMILAR;
            if(char1 == 0) {
                int test2 = SIMILAR;
                while (char2 > 0) {
                    if(strcmp(buf2, " ") == 0 || strcmp(buf2, "\n") == 0) {
                       char2 = read(fdin2, buf2, SIZE);                      
                    } else {
                        test2 = NOT_EQUAL;
                    }
                }
                test = test2;
            } else if (char2 == 0) {
                int test3 = SIMILAR;
                while (char1 > 0) {
                    if(strcmp(buf1, " ") == 0 || strcmp(buf1, "\n") == 0) {
                       char1 = read(fdin1, buf1, SIZE);                      
                    } else {
                        test3 = NOT_EQUAL;
                    }
                }
                test = test3;
            }
            result = test;            
        } else if ( num_of_bytes1 == identity && num_of_bytes2 == identity) {
            result = EQUAL;
        } else {
            result = SIMILAR;
        }
    }
    return result;
}