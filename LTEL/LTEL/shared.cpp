#include "shared.h"

// Godot
#include <Animation.hpp>
// End Godot

void shared_SetModelAnimation(HLOCALOBJ hObj, DDWORD iAnim)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer || iAnim == -1)
	{
		return;
	}

	auto sAnim = pExtraData->vAnimationList.at(iAnim);

	// Loops are set per animation
	auto pAnim = pExtraData->pAnimationPlayer->get_animation(sAnim.c_str());
	pAnim->set_loop(pExtraData->bLoop);

	pExtraData->pAnimationPlayer->play(sAnim.c_str());

	pExtraData->nCurrentAnimIndex = iAnim;

	return;
}

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

void shared_SetModelLooping(HLOCALOBJ hObj, DBOOL bLoop)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer || pExtraData->nCurrentAnimIndex == -1)
	{
		return;
	}

	auto sAnim = pExtraData->vAnimationList.at(pExtraData->nCurrentAnimIndex);

	// Loops are set per animation
	auto pAnim = pExtraData->pAnimationPlayer->get_animation(sAnim.c_str());
	pAnim->set_loop(bLoop);

	return;
}

DRESULT shared_SetObjectScale(HLOCALOBJ hObj, DVector* pScale)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	// Sprites don't have z scale param, so check if Y is > 0, but Z is not!
	if (pObj->IsType(OT_SPRITE) && pScale->y > 0.0f && pScale->z == 0.0f)
	{
		pScale->z = pScale->y;
	}

	pObj->SetScale(*pScale);
	return DE_OK;
}

void shared_GetObjectRotation(HLOCALOBJ hObj, DRotation* pRotation)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		*pRotation = DRotation(0, 0, 0, 0);
		return;
	}

	*pRotation = pObj->GetRotation();
}

LTELString* shared_CreateString(char* pString)
{
	return new LTELString(pString);
}

HSTRING shared_CopyString(HSTRING hString)
{
	auto szData = shared_GetStringData(hString);
	return (HSTRING)shared_CreateString(szData);
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
