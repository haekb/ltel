#include "shared.h"

LTELString* shared_CreateString(char* pString)
{
	return new LTELString(pString);
}
