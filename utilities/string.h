#ifndef STRING_H
#define STRING_H

class string{
	int length;
	char *str;
	
public:
		string(char *);

		const char *get();

		string& operator=(char *);
};

#endif
