#include <ctype.h>

Tolower(c)
char c;
{
	return(isupper(c) ? tolower(c) : c);
}
