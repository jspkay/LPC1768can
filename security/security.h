#ifndef SECURITY_H
#define SECURITY_H

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include "sha256.h"

#define ENCRYPT 1
#define DECRYPT 0

void DES3(unsigned char* data, unsigned char key[3][8], int mode);
void digest(unsigned char *data, unsigned char *key, unsigned char *final);
int verify_digest(unsigned char *data, unsigned char* key, unsigned char* dig);

#endif   // SECURITY_H