/* LOCALE stuff - contributed code. */

#include <string.h>
#include <locale.h>
#include <ctype.h>

int strnicoll(char *s1, char *s2, int maxlen) {

int len1, len2, len, i, c;
unsigned char t1[2],t2[2];

len1=strlen(s1);
len2=strlen(s2);
len=len1;
if (len2 < len1) len=len2;
if (maxlen < len) len=maxlen;
t1[1]='\0';
t2[1]='\0';

for (i=0; i<len; i++) {
  t1[0] = tolower(s1[i]);
  t2[0] = tolower(s2[i]);
  if ((c=strcoll(t1, t2)) != 0) return (c);
}
if (maxlen <= len1 && maxlen <= len2) return (0);
if (len1 < len2) return (-1);
if (len1 > len2) return ( 1);
return (0);
}

int stricoll(char *s1, char *s2) {

int len1, len2, len, i, c;
unsigned char t1[2],t2[2];

/* printf (">> %s %s\n",s1,s2); */
len1=strlen(s1);
len2=strlen(s2);
len=len1;
if (len2 < len1) len=len2;
t1[1]='\0';
t2[1]='\0';

for (i=0; i<len; i++) {
  t1[0] = tolower(s1[i]);
  t2[0] = tolower(s2[i]);
  if ((c=strcoll(t1, t2)) != 0) return (c);
}
if (len1 < len2) return (-1);
if (len1 > len2) return ( 1);
return (0);
}
