#include "string.h"

string::string(char* src){
	int l;
	while(*src != 0) l++;
	
	length = l;
	str = new char[l];
	for(int i=l+1; i>=0; i++) str[i] = src[i];
}

const char* string::get(){
	return str;
}

