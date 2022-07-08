/*---------------------END---------------------
=================================================
|   A naif aproache to make a miniexel          |
=================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>





/*---------------------START---------------------
=================================================
|   Definition of structure, globale variable   |
|   and definitions
=================================================
*/

typedef struct {
    int count;
    const char* data;
}g_string;



typedef enum {
    EXPR_TYPE_NUMBER = 0,
    EXPR_TYPE_CELL,
    EXPR_TYPE_BINARY_OP,
} Expr_Type;


typedef struct {
    Expr_Type type;
}Expr;


typedef enum {
    CELL_TYPE_TEXT = 0,
    CELL_TYPE_NUMBER,
    CELL_TYPE_EXPR ,
} Cell_Type ;

typedef union {
    g_string text;
    double number;
    Expr expr;
} CellAS;

typedef struct{
    Cell_Type type;
    CellAS as;
} Cell;
typedef struct {
    Cell* cells;
    int rows;
    int cols;
}Table;

Table allocTable(int rows, int cols){
        Table table = {0};
        table.rows = rows;
        table.cols = cols;
        table.cells = malloc(sizeof(Cell) * rows * cols);
    if(table.cells == NULL){
        fprintf(stderr,"malloc(): failed can't be done ./n");
    }
    memset(table.cells, 0, sizeof(Cell)*rows*cols);
}


Cell* table_cell_at(Table* table, int row, int col){
    assert(row < table->rows);
    assert(col < table->cols);
    return &table->cells[row * table->cols + col];
}


/*---------------------END---------------------
=================================================
|   Definition of structure, globale variable   |
|   and definitions
=================================================
*/


/*---------------------START---------------------
=================================================
|   mini string function                        |
=================================================
*/

    g_string trim(g_string in){
        int i = 0,j = 0 , n = 0;
        while(n < in.count){
            if(isspace(in.data[i])) i++;
            if(isspace(in.data[in.count - j - 1])) j++;
                ++n;
        }
        g_string res ={
            .count = in.count - j - i,
            .data = in.data  + i
            
        };

        return res;
    }

    g_string chop_by_delim(g_string* in, char delim){
        int i = 0;
        while(i < in->count && in->data[i] != delim) ++i;

        g_string res;
            res.count = i;
            res.data = in->data;

        if(i < in->count){
            in->count -= i+1;
            in->data += i+1; 
        }else{
            in->count -= i;
            in->data += i; 
        
        }
        return res;

    }

    void g_string_print(g_string in){
        for(int  i = 0; i < in.count ; ++i){
            printf("%c",in.data[i]);
        }
    }

/*---------------------END---------------------
=================================================
|   mini string function                        |
=================================================
*/



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

/*---------------------START---------------------
=================================================
|   Parse the input file into token getting     |
|   the of the table.                           |
=================================================
*/

void parse_table(Table* table, g_string content)
{
    for(int row = 0 ;content.count > 0; ++row){
        g_string line = chop_by_delim(&content , '\n');
        for(int col = 0;line.count > 0; ++col)
        {   
            g_string cell = trim(chop_by_delim(&line, '|'));
            table_cell_at(table, row, col)->as.text = cell;
        }
    }
}


void table_size(g_string in, int* out_rows,  int* out_cols){
    int cols = 0, rows = 0;
    for(; in.count > 0; ++rows){
        g_string line = chop_by_delim(&in, '\n');
        const char* start = line.data;
        int col = 0;
        for(; line.count > 0 ; ++col)
            trim(chop_by_delim(&line,'|'));
        // The biggest rows has the cols number 
        if(cols < col) cols = col;
    }
    if(out_cols) *out_cols = cols;
    if(out_rows) *out_rows = rows;
}




/*---------------------END---------------------
=================================================
|   Parse the input file into token getting     |
|   the of the table.                           |
=================================================
*/

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr,"argument error \n");
        exit(1);
    }
    
    const char* file_path = argv[1];
    int content_size = 0;
    char* content = fetchfile(file_path,&content_size);
    if(content  == NULL){
        fprintf(stderr,"Error : int making the file into string \n");
        exit(1);
    }

    g_string input = {
        .count = content_size ,
        .data = content

    };
    int rows, cols;
    table_size(input,&rows,&cols);
    //Table table = allocTable(rows, cols);
    Table table = {0};
    table.rows = rows;
        table.cols = cols;
        table.cells = malloc(sizeof(Cell) * rows * cols);
    if(table.cells == NULL){
        fprintf(stderr,"malloc(): failed can't be done ./n");
    }
    memset(table.cells, 0, sizeof(Cell)*rows*cols);


    parse_table(&table, input);

    for(int row = 0 ;row < table.rows; ++row){
        for(int col = 0;col < table.cols; ++col) 
            printf("%s |",table_cell_at(&table, row, col)->as.text);
        printf("\n");
    }

    return 0;
} 