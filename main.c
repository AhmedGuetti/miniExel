#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "lexer.h"



/*---------------------START---------------------
=================================================
|   get the data from a file into a c_string    |
=================================================
*/
char* fetchfile(const char* file_name, int* size){
    char* buffer = NULL;
	FILE* fptr = fopen(file_name,"rb");
    if(fptr == NULL) goto error;


    if(fseek(fptr,0,SEEK_END) < 0) goto error;


    int s = ftell(fptr);
    if(s < 0) goto error;
	*size = s;
    

    if(fseek(fptr,0,SEEK_SET) < 0) goto error;


    buffer = (char*)malloc(s*sizeof(char)+1);
    if(buffer == NULL) goto error;
    if(memset(buffer,'\0',s*sizeof(char)+1) == NULL) goto error;

    int read_size = fread(buffer,1,s,fptr);
    if(ferror(fptr)) goto error;

    fclose(fptr);
    return buffer;

    error:
    if(fptr){
        fclose(fptr);
    }
    if(buffer){
        free(buffer);
    }

}
/*---------------------END---------------------
=================================================
|   get the data from a file into a c_string    |
=================================================
*/







char* trim(char* in){
	int i = 0,j = 0 , n = 0;
	while(in[n] != '\0'){
		if(isspace(in[i])) i++;
		if(isspace(in[strlen(in) - j - 1])) j++;
			++n;
	}
	char* res = (char*)calloc(n-j-i + 1,sizeof(char));
	strncpy(res, in+i,n-j-i);
	
	return res;
}

char** str_split(char* a_str, const char a_delim, int* n)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;
	

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = trim(strdup(token));
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
		*n = idx;
    }

    return result;
}




char*** table_size(char* in, int* out_rows,  int* out_cols){
    int cols = 0, rows = 0, tmp;
	int n = strlen(in);
	char** lines = str_split(in, '\n', &rows);
	char*** token = (char***) malloc(rows * sizeof(char**));
	for(int  i = 0; i < rows ; ++i){
		token[i] = str_split(lines[i], '|', &tmp);
		if(tmp > cols) cols = tmp;
	}

		
    if(out_cols) *out_cols = cols;
    if(out_rows) *out_rows = rows;

	return token;
}


char*** calculate_table(char*** table, int row, int col){
	for(int i = 0; i < row ;++i){
		for(int j=0; j < col;++j){
			if(table[i][j][0] == '='){
				//char* expr = malloc(strlen(table[i][j]) * 2 * sizeof(char));
				int r = 1, start = 1;
				while(table[i][j][r] != '\0'){
					int n,m;				
					if(table[i][j][r] > 'A' && table[i][j][r] < 'Z' ){
						m = table[i][j][r] % 26;
					} else if(table[i][j][r] > '1' && table[i][j][r] < '9'){
						n = atoi(table[i][j]+r);
					} 
					++r;
				}
			}
		}
	}
}

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr,"argument error \n");
        exit(1);
    }
    
    const char* file_path = argv[1];
	int count;
    char* content = fetchfile(file_path,&count);
    if(content == NULL){
        fprintf(stderr,"Error : int making the file into string \n");
        exit(1);
    }
    fwrite(content, 1,count, stdout);
	int row,col;
	char*** token = table_size(content,&row,&col);


	printf("\n\n%d : %d\n",row, col);


    return 0;
}