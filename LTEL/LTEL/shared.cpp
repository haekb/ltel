#include "shared.h"

LTELString* shared_CreateString(char* pString)
{
	return new LTELString(pString);
}

char* shared_GetStringData(HSTRING hString)
{
	LTELString* pString = (LTELString*)hString;

	if (!pString)
	{
		return nullptr;
	}

	return (char*)pString->sData.c_str();
}