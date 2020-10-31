#include "client.h"
// Here be our accessible functions

HCONSOLEVAR impl_GetConsoleVar(char* pName)
{
	return nullptr;
}

void LTELClient::InitFunctionPointers()
{
	GetConsoleVar = impl_GetConsoleVar;
}