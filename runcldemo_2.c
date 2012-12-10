#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdlib.h> 
#include <regex.h>

int main()
{
	int worksize=4000;
	int depth=7;
	int sphere_num=9;
	while(worksize<5000)
	{
		char string1[20];
	  	char string2[20];
		char string3[20];
		char string4[20];
		char string5[20];

		strcpy(string1, "./cldemo ");
		sprintf(string2,"%i",worksize);
		sprintf(string4,"%i",depth);
		sprintf(string5,"%i",sphere_num);
		strcpy(string3, " data.txt");
		strcat( string1, string2 );
		strcat( string1, string3 );
		printf("%s\n",string1);

		FILE *my_file;
		my_file=fopen("parameters.h","w");
		fprintf(my_file,"#ifndef MAX_DEPTH\n");
		fprintf(my_file,"#define MAX_DEPTH %s\n",string4);
		fprintf(my_file,"#endif\n");
		fprintf(my_file,"#ifndef num_spheres\n");
		fprintf(my_file,"#define num_spheres %s\n",string5);
		fprintf(my_file,"#endif\n");
		fclose(my_file);

		system(string1);
		worksize=worksize*2;		
	}
	
	
	

}
