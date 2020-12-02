#include "shared.h"

DDWORD shared_GetModelAnimation(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return -1;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer)
	{
		return -1;
	}

	return pExtraData->nCurrentAnimIndex;
}

DDWORD shared_GetModelPlaybackState(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return MS_PLAYDONE;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer || pExtraData->nCurrentAnimIndex == -1)
	{
		return MS_PLAYDONE;
	}

	if (pExtraData->pAnimationPlayer->is_playing())
	{
		return 0;
	}

	return MS_PLAYDONE;
}

HMODELANIM shared_GetAnimIndex(HOBJECT hObj, char* pAnimName)
{
	if (!hObj)
	{
		return -1;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer)
	{
		return -1;
	}

	auto sAnimList = pExtraData->pAnimationPlayer->get_animation_list();

	for (int i = 0; i < sAnimList.size(); i++)
	{
		if (sAnimList[i] == pAnimName)
		{
			return i;
		}
	}


	return -1;
}

DRESULT shared_SetObjectScale(HLOCALOBJ hObj, DVector* pScale)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	pObj->SetScale(*pScale);
	return DE_OK;
}

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

void shared_FreeString(HSTRING hString)
{
	LTELString* pString = (LTELString*)hString;

	if (!pString)
	{
		return;
	}

	delete pString;
}

int shared_Parse(char* pCommand, char** pNewCommandPos, char* argBuffer, char** argPointers, int* nArgs)
{
	const int nMaxBuffer = 5 * (PARSE_MAXTOKENSIZE + 1);
	std::string sCurrent = "";
	bool bMoreData = false;
	bool bIgnoreSpace = false;

	char* pCurrentArg = argBuffer;

	*nArgs = 0;

	while (true)
	{
		char szCurrent = *pCommand++;

		// End of string!
		if (szCurrent == 0)
		{
			char szArg[nMaxBuffer];
			strcpy_s(szArg, nMaxBuffer, sCurrent.c_str());
			szArg[nMaxBuffer - 1] = '\0';

			argPointers[*nArgs] = szArg;
			++(*nArgs);

			break;
		}

		// If we're in quotes, ignore any spaces!
		if (szCurrent == '"')
		{
			bIgnoreSpace = !bIgnoreSpace;
			continue;
		}

		// Seperator between key/value
		if (!bIgnoreSpace && szCurrent == ' ')
		{
			char szArg[nMaxBuffer];
			strcpy_s(szArg, nMaxBuffer, sCurrent.c_str());
			szArg[nMaxBuffer - 1] = '\0';

			argPointers[*nArgs] = szArg;
			++(*nArgs);

			sCurrent = "";
			continue;
		}

		// End of value + this functions lifespan
		if (szCurrent == ';')
		{
			char szArg[nMaxBuffer];
			strcpy_s(szArg, nMaxBuffer, sCurrent.c_str());
			szArg[nMaxBuffer - 1] = '\0';

			argPointers[*nArgs] = szArg;
			++(*nArgs);

			bMoreData = true;
			break;
		}

		// Append to our on-going string
		sCurrent += szCurrent;
	}

	*pNewCommandPos = pCommand;

	return bMoreData;
}
