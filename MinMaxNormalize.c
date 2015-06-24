/*************************************************************************************
File: MinMaxNormalize.c
Purpose: For a given arff data file, this program performs the min-max normalization. 
Date: 09/13/2012

Usage:
“$ Normalize InputFile  –c classattribute –attribute1 newminVal1 newmaxVal1 –attribute2 newminVal2 newmaxVal2 … “. This line indicates that attribute classattribute will be skipped for normalization.

eg.1 $MinMaxNormalize mmtest.arff -c class -x 10 20
eg.2 $MinMaxNormalize mmtest.arff -c class -x 10 20 -y 5 10

The attributes can be provided in any order. Normalization for class attribute is not done. If new ranges are not provided for an attribute, the original data will be populated for that attribute. 

**************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXCOLS 100
#define MAXROWS 10000
#define MAXLINE 4096
#define MAXTOKENSIZE 100

const char *DELIMITER_STR = " ";
const char *ATTRIB_STR = "@attribute";
const char *DATA_STR = "@data";

//Function declaration
char *replace(char *st, char *orig, char *repl);
char **tokenize(char *line);
void readData(char * line);
void readAttributes(char * line);
int getColIndex(char *attrib_param);
float getMaxFromColumn(int colindex);
float getMinFromColumn(int colindex);
int createArffFile(char *fname, char *attrib[MAXCOLS], int attribcount, char *dtype[MAXCOLS], float data_array[MAXROWS][MAXCOLS], int rowscount);

char *attrib_name_list[MAXCOLS];
char *attrib_dtype_list[MAXCOLS];
float orig_data_array[MAXROWS][MAXCOLS];
float new_data_array[MAXROWS][MAXCOLS];

int attribCount = 0;
int objectCount = 0;

//Main function
int main(int argc, char *argv[])
{
    FILE *fp;
     
    int data_read_start = 0;
    int i=0,j=0;
    
    float minmax_dataarray[2][MAXCOLS];
    float norm_dataarray[MAXROWS][MAXCOLS];
    char line[MAXLINE];
    
    char outputFile[100];
    char inputFile[100];
    char attrib_param[100];

    int no_attrib_params = 0;
    int colindex=0;
    float new_min, new_max, old_min, old_max;

    if ((argc-1) % 3 != 0) {
	printf("Invalid number of arguments\n");
        printf("Usage:\n");
	printf("$ms0023Normalize InputFile  –c classattribute –attribute1 newminVal1 newmaxVal1 –attribute2 newminVal2 newmaxVal2 …\n");
	return;
    }

    strcpy(inputFile, argv[1]);
    fp = fopen(inputFile,"r");
    
    if (fp == NULL){
       printf("Error opening file");
       return 1;
    }
    
    printf("Reading from file %s..........\n", inputFile);
    while(fgets(line,MAXLINE,fp)!= NULL){
         if(data_read_start == 1){
             readData(line);
             objectCount++;
         }
         else if (strstr(line,DATA_STR)!=NULL)	{
              data_read_start = 1;
	}
         else {
              if (strstr( line, ATTRIB_STR)!=NULL) {
                    attribCount++;
                    readAttributes(line);
              } 
         }
   }
   
    printf("Reading from file completed.\n");

    printf("Computing min max values for %d attributes.......\n",attribCount);

    // calculate the max and min values and write to MinMax output file
    for (i=0; i<attribCount; i++){
        minmax_dataarray[0][i]=getMinFromColumn(i);
        minmax_dataarray[1][i]=getMaxFromColumn(i);
    }

    strcat(outputFile, "MinMax");
    strcat(outputFile, inputFile);

    createArffFile(outputFile, attrib_name_list, attribCount-1, attrib_dtype_list, minmax_dataarray, 2);

    printf("Min max values dumped in the file %s\n",outputFile);
    
    // Code for normalization here
    for(i=0;i<objectCount;i++)
		for(j=0;j<attribCount;j++)
			new_data_array[i][j] = orig_data_array[i][j];
    
    no_attrib_params = (argc - 4)/3;

    printf("Processing normalization for %d attributes.........\n", no_attrib_params);

    for(i=1;i<=no_attrib_params; i++){
		//normalize this attribute
		strcpy(attrib_param, replace(argv[1+i*3],"-",""));
		new_min = atof (argv[2+i*3]);
		new_max = atof (argv[3+i*3]);
		colindex = getColIndex(attrib_param);
		old_min = minmax_dataarray[0][colindex];
		old_max = minmax_dataarray[1][colindex];

		for(j=0;j<objectCount;j++)
			new_data_array[j][colindex] = (orig_data_array[j][colindex] - old_min )*(new_max - new_min) / (old_max - old_min) + new_min;
    }

    strcat(outputFile, "Normalize");
    strcat(outputFile, inputFile);

    if (createArffFile(outputFile, attrib_name_list, attribCount-1, attrib_dtype_list, new_data_array, objectCount)==1){
		printf("Error saving file.");
		return;
    }

    printf("Normalized data dumped into file %s\n", outputFile);
    printf("Processing completed. Please check the output files for reveiw\n");

    fclose(fp);    
    return 0;
}

//This function reads the attribute declaration part of an arff file
void readAttributes(char * line){
	char **tokens = tokenize(line);
	attrib_name_list[attribCount-1]=tokens[1];
	attrib_dtype_list[attribCount-1]=tokens[2];
}

//This function is used to parse data portion of an arff file
void readData(char * line){ 
     int col=0;
     char **tokens = tokenize(line);
     
     for(col = 0; tokens[col] != NULL; col++) {
           orig_data_array[objectCount][col]= atof(tokens[col]);
           free(tokens[col]);
     }
}

//This function returns an array of string
char **tokenize(char *line) {
    char **tokens;
    char *tmpString;
    char *token;
    int i = 0;

    tokens  = (char **) malloc(sizeof(char *) * MAXTOKENSIZE);
    if(tokens == NULL) return NULL;
    tmpString = (char *) malloc(sizeof(char) * MAXLINE + 1);
    if(tmpString == NULL) return NULL;

    strcpy(tmpString, line);
    for(i = 0; i < MAXCOLS; i++) tokens[i] = NULL;
	    
    i = 0;
    token = strtok(tmpString, DELIMITER_STR);
    
    while((i < (MAXCOLS - 1)) && (token != NULL)) {
             tokens[i] = (char *) malloc(sizeof(char) * MAXTOKENSIZE + 1);
             if(tokens[i] != NULL) {
                          strcpy(tokens[i], token);
                          i++;
                          token = strtok(NULL, DELIMITER_STR);
             }
    }
    
    free(tmpString);
    return tokens;
}

//Function to create an arff file
int createArffFile(char *fname, char *attrib[MAXCOLS], int attribcount, char *dtype[MAXCOLS], float data_array[MAXROWS][MAXCOLS], int rowscount)
{
    int i=0,j=0;
    FILE *fp = fopen(fname,"w");
    
    if (fp == NULL){
       printf("Error opening file");
       return 1;
    }
    
    fprintf(fp, "@relation ");
    fprintf(fp, replace(fname,".arff",""));
    fprintf(fp, "\n\n");
    
    for(i=0;i<attribcount;i++)
          fprintf(fp, "@attribute %s %s",attrib[i], dtype[i]);
    
    fprintf(fp,"\n");
    fprintf(fp,"@data\n");
    
    for(i=0;i<rowscount;i++){
          for(j=0;j<attribcount;j++)
                fprintf(fp,"%f ", data_array[i][j]);
          fprintf(fp,"\n");
    }

    fclose(fp);
    return 0;
}

float getMaxFromColumn(int colindex){
      int i;
      float max = orig_data_array[0][colindex];
      for (i=1;i< objectCount; i++)
          if(orig_data_array[i][colindex] > max)
                 max=orig_data_array[i][colindex];
      return max;
}

float getMinFromColumn(int colindex){
      int i;
      float min = orig_data_array[0][colindex];
      for (i=1;i< objectCount; i++)
          if(orig_data_array[i][colindex] < min)
                 min=orig_data_array[i][colindex];
      return min;
}

//Replace string orig in st with repl
char *replace(char *st, char *orig, char *repl) {
  static char buffer[4096];
  char *ch;
  if (!(ch = strstr(st, orig)))
     return st;
    strncpy(buffer, st, ch-st);  
    buffer[ch-st] = 0;
    sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
    return buffer;
}

//This function returns the column index given an attribute name
int getColIndex(char *attrib_param){
	int count=0;
	while(1){
		if ( strcmp(attrib_param,attrib_name_list[count])==0 )
			return count;
		count++;
	}
	return -1;
}
