/*
 * Author: Erik Grootendorst
 * Class: CS570
 * Prof: Carroll
 * Date: 09/4/18
 * Description: parses a string by char and returns words.
 * 				Works as a unix lexial analyzer to filter 
 * 				key symbols based on placement.
 */
 
#include <stdio.h>
#include "getword.h"
#include <stdlib.h>

int getword(char *w){
	extern int EOL_REACHED;
	extern int backSlash;
	int iochar = getchar();
	int charCount = 0;
	int bsBoolean = 0; //backslash boolean '\'
	int lsBoolean = 0; //less than boolean '<'
	int specCase = 0;// account for strings with leading $
	char *path = getenv("HOME");
    int pathEnd = strlen(path)-1;
	*w = '\0';
	while(iochar == 32) //remove preceding spaces
                iochar = getchar();

	if(iochar == 126){//tilde
		while(pathEnd != -1)
			ungetc(*(path+(pathEnd--)),stdin);//reverse into stream
		iochar = getchar();
	}
	if(iochar == 36){
		specCase = 1;
		iochar = getchar();
		if (iochar == 10 || iochar == 59 || iochar == EOF){
			ungetc(iochar,stdin);// specific case to isolate $	
			return charCount;
		}
	}		
	
	while(iochar != EOF){
		if(charCount == 254){
			ungetc(iochar,stdin);
			return charCount;
		}	
		if(iochar == 10){ //newline	
			if(bsBoolean == 1){
				iochar = 32;
				////////////////global for p2.c
				backSlash = 1;
				//////////////
			}
			break;
		}//if
		if((iochar == 32 || iochar == 59 || iochar == 38 || iochar == 62 || iochar == 124) && bsBoolean == 0)//space, amper,semicol
			break;
		////////////////////////////////a global for p2!!!!!
		if(iochar == 38 && bsBoolean == 1)
			backSlash = 1;
		////////////////////////////
		if(iochar == 60){	//lessthan
			if(bsBoolean == 0 && charCount != 0 && lsBoolean == 0){
				ungetc(iochar,stdin);
				break;
			}
			if(bsBoolean == 0)
				lsBoolean = 1;
		}
		if(iochar == 92 && bsBoolean == 0){//backslash
			bsBoolean = 1;
			if(lsBoolean == 1){
				ungetc(iochar,stdin);
				break;
			}
			iochar = getchar();
			continue;
		}
		if(iochar != 60 && lsBoolean == 1){
			ungetc(iochar,stdin);
			break;
		}
	
		*(w++)=iochar;
		*w = '\0';
		charCount++;
		bsBoolean = 0;	
		iochar = getchar();
	}//while

	if(iochar == 38 || iochar == 62 || iochar == 124){
		if(charCount != 0)
                	ungetc(iochar,stdin);
		else{
			*w = iochar;
			*(w+1) = '\0';
			charCount++;
		}
	}
	if((iochar == 59 || iochar == EOF || iochar == 10) && charCount != 0)
		ungetc(iochar,stdin);

	if(iochar == 10 && charCount == 0)//to account for strings ending with newline
		*w = '\0';

	if(iochar == EOF && charCount == 0){
		*w = '\0';
		return 0-STORAGE;
	}	
	//signals to CLI to process current args 
	if(iochar == 10 || iochar == 59)
		EOL_REACHED = 1;
	
	if(specCase == 1)
		return 0-charCount;
	return charCount;
}