#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

between_character(char buffer[], char strout[], char strin[] ,char *out, int choose)  //we get the character that between strout and strin,the out is the character that return main
{  
 	char *str1, *str2, *token, *subtoken;  
    char *saveptr1, *saveptr2;  
    int j;      
    for (j = 1, str1 = buffer;  ; j++, str1 = NULL) 
    {  
		token = strtok_r(str1, strout, &saveptr1);  
      	if (token == NULL)  
        	break;  
          	printf("%d: %s\n", j, token);   
      		if(j == choose)
      		{
            	for (str2 = token; ; str2 = NULL) 
            	{  
            		subtoken = strtok_r(str2, strin, &saveptr2);  
                	if (subtoken == NULL)  
                		break;  
               		printf(" --> %s\n", subtoken); 
                	memcpy(out,subtoken,strlen(subtoken));
             	}  
         	}
     }  
    // return subtoken;
}  


add_checksum(char buffer[] , int len) //the last character is the checksum,get the checksum
{
	int i;
	char checksum = 0x00;
	for(i=0;i<len-1;i++)
	{
		checksum += buffer[i];
		buffer[len-1] = checksum;
		buffer[len] = '\0';
	}
}


int checksum_check(char command[], int n) //confirm the checksum
{
	char checksum = 0x00;
	int i;
	char m;
	if(n >= 5){		//命令最小长度为5;  （起始字 命令字 长度 数据 校验和）
		m = command[2];     //命令长度;
		for( i=0;i<m-1;i++ )
			checksum += command[i];
		if( checksum == command[m-1] )
			return 0;
		else {
			return -1;
		}
	}
	else
		return -1;
}