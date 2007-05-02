#include "Locale.h"

char *my_locale;

void SetLocale(char *loc)
{
	my_locale = loc;
}

char *GetLocale(void)
{
	return my_locale;
}