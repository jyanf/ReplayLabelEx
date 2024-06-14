//Original Author: À˚»À§Œø’À∆
//

#include <string>
#include <windows.h>
#include <Shlwapi.h>
//#include <SokuLib.hpp>
#include "../SokuLib/src/SokuLib.hpp"

static bool init = false;

enum ComMode {
	COM_MODE_STORY = 0,
	COM_MODE_ARCADE = 1,
	COM_MODE_COM = 2,
	COM_MODE_PLAYER = 3,
	COM_MODE_SERVER = 4,
	COM_MODE_CLIENT = 5,
	COM_MODE_WATCHER = 6,
};
enum BattleResult {
	BATTLE_RESULT_WIN,
	BATTLE_RESULT_LOSW,
	BATTLE_RESULT_DRAW,
	BATTLE_RESULT_NO_GAME
};

static BattleResult GetWinLose()
{
	SokuLib::BattleMode comMode = SokuLib::mainMode;
	SokuLib::BattleManager battleMgr = SokuLib::getBattleMgr();
	const char lWin = battleMgr.leftCharacterManager.score;
	const char rWin = battleMgr.rightCharacterManager.score;
	const char& myWin = (comMode == SokuLib::BATTLE_MODE_VSCLIENT) ? rWin : lWin;
	const char& otherWin = (comMode == SokuLib::BATTLE_MODE_VSCLIENT) ? lWin : rWin;

	if (myWin == 2) {
		if (otherWin == 2) {
			return BATTLE_RESULT_DRAW;
		}
		return BATTLE_RESULT_WIN;
	}
	if (otherWin == 2) {
		return BATTLE_RESULT_LOSW;
	}
	return BATTLE_RESULT_NO_GAME;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}

static DWORD WINAPI DummyGetPrivateProfileStringA(
	__in_opt LPCSTR lpAppName,
	__in_opt LPCSTR lpKeyName,
	__in_opt LPCSTR lpDefault,
	__out_ecount_part_opt(nSize, return +1) LPSTR lpReturnedString,
	__in_opt DWORD nSize,
	__in_opt LPCSTR lpFileName
)
{
	const DWORD result = ::GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
	if (std::string("replay") != lpAppName || std::string("file_vs") != lpKeyName) {
		return result;
	}

	std::string path(lpReturnedString);
	const std::string label = "%winlose";
	const std::string winlose[4] = { "win", "lose", "draw", "no_game" };
	const unsigned int pos = path.find(label);
	if (pos == std::string::npos) {
		return result;
	}
	path.replace(path.begin() + pos, path.begin() + pos + label.size(), winlose[GetWinLose()]);
	::strcpy_s(lpReturnedString, nSize, path.c_str());

	printf("replaced & saved %s\n", path.c_str());
	return result;
}


extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	//DWORD old;

#ifdef _DEBUG
	FILE* _;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	DWORD oldProtect;
	::VirtualProtect(&SokuLib::DLL::kernel32.GetPrivateProfileStringA, 4, PAGE_READWRITE, &oldProtect);
	SokuLib::DLL::kernel32.GetPrivateProfileStringA = DummyGetPrivateProfileStringA;
	::VirtualProtect(&SokuLib::DLL::kernel32.GetPrivateProfileStringA, 4, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	puts("Hook, success!");
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return true;
}


extern "C" __declspec(dllexport) int getPriority()
{
	return 0;
}
