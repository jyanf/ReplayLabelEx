//Initial Author: 他人の空似
//
#include <array>
#include <vector>
#include <string>
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
//#include <SokuLib.hpp>
#include "../SokuLib/src/SokuLib.hpp"
//#include "../SokuLib/src/DeprecatedElements.hpp"
#define FILEVS_LIMIT 0x104
#define isCL (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
#define isNV (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT)

namespace Keys {
	static bool p_order()
	{
		const auto mode = SokuLib::mainMode;
		if (mode == SokuLib::BATTLE_MODE_VSPLAYER || mode== SokuLib::BATTLE_MODE_VSCOM)
		{
			return strcmp(SokuLib::profile1.name, SokuLib::profile2.name) <= 0;
		}
		else if (mode == SokuLib::BATTLE_MODE_VSWATCH)
		{
			const auto& net = SokuLib::getNetObject();
			return strcmp(net.profile1name, net.profile2name) <= 0;
		}
		return true;
	}
	static const std::array<std::wstring, 5> BattleMode = {
		L"vs_com", L"vs_loc", L"vs_net", L"spectate", L"other"
	};
	static const std::wstring& GetBattleMode()
	{
		switch (SokuLib::mainMode)
		{
		case SokuLib::BATTLE_MODE_VSCOM:	return BattleMode[0];
		case SokuLib::BATTLE_MODE_VSPLAYER: return BattleMode[1];
		case SokuLib::BATTLE_MODE_VSCLIENT:
		case SokuLib::BATTLE_MODE_VSSERVER: return BattleMode[2];
		case SokuLib::BATTLE_MODE_VSWATCH:	return BattleMode[3];
		case SokuLib::BATTLE_MODE_STORY:
		case SokuLib::BATTLE_MODE_ARCADE:
		case SokuLib::BATTLE_MODE_TIME_TRIAL:
		case SokuLib::BATTLE_MODE_PRACTICE:
		default:							return BattleMode[4];
		}
	}
	static std::array<char, 2> GetScores()
	{
		const SokuLib::BattleManager& battleMgr = SokuLib::getBattleMgr();
		return { battleMgr.leftCharacterManager.score, battleMgr.rightCharacterManager.score };
	}
	static const std::array<std::wstring, 4> BattleResult = {
		L"win", L"lose", L"noed", L"draw", 
	};
	static const std::array<std::wstring, 2> GetBattleResult()
	{
		const auto scores = GetScores();
		if (scores[0] == 2) {
			if (scores[1] == 2) return {BattleResult[3], BattleResult[3]};
			return { BattleResult[0], BattleResult[1] };
		}
		if (scores[1] == 2) {
			return { BattleResult[1], BattleResult[0] };
		}
		return { BattleResult[2], BattleResult[2] };
	}
	static const std::wstring GetSelfProfile()
	{
		if (!isNV) return p_order() ? L"%p1" : L"%p2";
		return isCL ? L"%p2" : L"%p1";
	}
	static const std::wstring GetOppoProfile()
	{
		if (!isNV) return !p_order() ? L"%p1" : L"%p2";
		return isCL ? L"%p1" : L"%p2";
	}
	static const std::wstring GetSelfChar()
	{
		if (!isNV) return p_order() ? L"%p1" : L"%p2";
		return isCL ? L"%c2" : L"%c1";
	}
	static const std::wstring GetOppoChar()
	{
		if (!isNV) return !p_order() ? L"%c1" : L"%c2";
		return isCL ? L"%c1" : L"%c2";
	}
};

static std::array<std::wstring, 2> Inis = { L"ReplayLabelEx.ini", L"CharacterNames.ini" };

static bool GetChName(const std::vector<SokuLib::Character>& ChSerials, std::vector<std::wstring>& names)
{//按角色编号取角色名（缩写）
	auto iniPath = Inis[1].c_str();
	//检查ini return false
	if (!PathFileExistsW(iniPath))
	{
#ifdef _DEBUG
		wprintf(L"%s not exist\n", iniPath);
#endif		
		return false;
	}
	const DWORD maxsize = 20;
	LPWSTR const tempw = new wchar_t[maxsize];
	for (auto serial : ChSerials)
	{
		GetPrivateProfileStringW(L"Character", (L"ch" + std::to_wstring(serial)).c_str(), L"UNKNOWN", tempw, maxsize, iniPath);
		names.push_back(tempw);
#ifdef _DEBUG
		wprintf(L"Get ch%2d, name %s in %d\n", serial, tempw, ChSerials.size());
#endif // _DEBUG

	}

	delete[] tempw;
	return true;
}

static bool GetSubs(std::wstring& path, std::vector<std::wstring>& labels, std::vector<std::wstring>& subs)
{//模块换标签，关键字换实值
	auto iniPath = Inis[0].c_str();
	//检查ini return false
	if (!PathFileExistsW(iniPath))
	{
#ifdef _DEBUG
		wprintf(L"%s not exist\n", iniPath);
#endif
		return false;
	}

	const DWORD maxsize = FILEVS_LIMIT;
	LPWSTR const tempw = new wchar_t[maxsize];
	//LPSTR const tempc = new char[maxsize];
	const DWORD resultVs = GetPrivateProfileStringW(L"PathFormat", L"file_vs", NULL, tempw, maxsize, iniPath);
	path = tempw;
	//wprintf(L"%s as %s", tempw, path.c_str());

	//按模式展开
	GetPrivateProfileStringW(L"BattleMode",		L"label",	L"%md",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	GetPrivateProfileStringW(L"BattleMode", Keys::GetBattleMode().c_str(), NULL, tempw, maxsize, iniPath);
	subs.push_back(tempw);

	//对战结果
	auto results = Keys::GetBattleResult();
	GetPrivateProfileStringW(L"BattleResult", results[0].c_str(), NULL, tempw, maxsize, iniPath);
	results[0] = tempw;
	GetPrivateProfileStringW(L"BattleResult", results[1].c_str(), NULL, tempw, maxsize, iniPath);
	results[1] = tempw;
	GetPrivateProfileStringW(L"BattleResult",	L"label1",	L"%e1",		tempw, maxsize, iniPath);
	labels.push_back(tempw); subs.push_back(results[0]);
	GetPrivateProfileStringW(L"BattleResult",	L"label2",	L"%e2",		tempw, maxsize, iniPath);
	labels.push_back(tempw); subs.push_back(results[1]);
	GetPrivateProfileStringW(L"BattleResult",	L"labels",	L"%es",		tempw, maxsize, iniPath);
	labels.push_back(tempw); subs.push_back(!isNV ? (Keys::p_order() ? results[0] : results[1]) : (isCL ? results[1] : results[0]));
	GetPrivateProfileStringW(L"BattleResult",	L"labelo",	L"%eo",		tempw, maxsize, iniPath);
	labels.push_back(tempw); subs.push_back(!isNV ? (Keys::p_order() ? results[0] : results[1]) : (isCL ? results[0] : results[1]));

	//1P、2P比分
	const auto scores = Keys::GetScores();
	GetPrivateProfileStringW(L"BattleScore",	L"label1",	L"%s1",		tempw, maxsize, iniPath);
	labels.push_back(tempw); subs.push_back(std::to_wstring(scores[0]));
	GetPrivateProfileStringW(L"BattleScore",	L"label2",	L"%s2",		tempw, maxsize, iniPath);
	labels.push_back(tempw); subs.push_back(std::to_wstring(scores[1]));
	
	//自机机签
	GetPrivateProfileStringW(L"ProfileName",	L"labels",	L"%ps",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetSelfProfile());
	//对手机签
	GetPrivateProfileStringW(L"ProfileName",	L"labelo",	L"%po",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetOppoProfile());
	//自机角色
	GetPrivateProfileStringW(L"CharacterName",	L"labels",	L"%cs",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetSelfChar());
	//对手角色
	GetPrivateProfileStringW(L"CharacterName",	L"labelo",	L"%co",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetOppoChar());

	//扩展原版角色名
	std::vector<std::wstring> CharNamesW;
	std::vector<SokuLib::Character> Serials = { SokuLib::leftChar, SokuLib::rightChar };
	if (!GetChName(Serials, CharNamesW)) CharNamesW = {L"%c1", L"%c2"};
	GetPrivateProfileStringW(L"CharacterNameW",	L"label1",	L"%C1",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(CharNamesW[0]);
	GetPrivateProfileStringW(L"CharacterNameW",	L"label2",	L"%C2",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(CharNamesW[1]);
	GetPrivateProfileStringW(L"CharacterNameW",	L"labels",	L"%Cs",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(!isNV ? (Keys::p_order() ? CharNamesW[0] : CharNamesW[1]) : (isCL ? CharNamesW[1] : CharNamesW[0]));
	GetPrivateProfileStringW(L"CharacterNameW", L"labelo",	L"%Co",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(!isNV ? (!Keys::p_order() ? CharNamesW[0] : CharNamesW[1]) : (isCL ? CharNamesW[0] : CharNamesW[1]));

	//角色名列表
	//GetPrivateProfileStringW(L"Character", L"ch01", NULL, , maxsize, iniPath);

	delete[] tempw;
	
	return true;
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
#ifdef _DEBUG
	puts("entering! ");
#endif
	
	DWORD result = GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
	if (std::string("replay") != lpAppName || std::string("file_vs") != lpKeyName)
	{
		return result;
	}

	std::vector<std::wstring> labels;
	std::vector<std::wstring> subs;
	std::wstring path;
	if (!GetSubs(path, labels, subs)) return result;
	for (unsigned i = 0; i < labels.size(); ++i)
	{
		unsigned int pos;
		while (pos = path.find(labels[i]), pos != std::wstring::npos)
		{
#ifdef _DEBUG
			wprintf(L"%s replacing %s\n", subs[i].c_str(), labels[i].c_str());
#endif

			path.replace(path.begin() + pos, path.begin() + pos + labels[i].size(), subs[i]);
		}
	}
	//if (WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, lpReturnedString, 0, NULL, NULL) > nSize) return result;
	const char defaultc = 'x';//防止rep路径带?导致丢失
	result = WideCharToMultiByte(CP_THREAD_ACP, 0, path.c_str(), -1, lpReturnedString, nSize, &defaultc, NULL);
	//strcpy_s(lpReturnedString, nSize, path.c_str());
	
#ifdef _DEBUG
	printf("replaced & saved as \n%s\n", lpReturnedString);
#endif
	return result;
}

typedef DWORD(__stdcall*  pGPPSA)(LPCSTR, LPCSTR, LPCSTR, LPSTR, DWORD, LPCSTR);
extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	wchar_t iniPath[MAX_PATH];
	for (auto& ini : Inis)
	{
		GetModuleFileNameW(hMyModule, iniPath, _countof(iniPath));
		PathRemoveFileSpecW(iniPath);
		PathAppendW(iniPath, ini.c_str());
		ini = iniPath;
	}
#ifdef _DEBUG
	FILE* _;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif
	
	DWORD oldProtect;
	//auto target = &SokuLib::DLL::kernel32.GetPrivateProfileStringA; //wrong
	auto target = reinterpret_cast<pGPPSA*>(0x0085712C);
	VirtualProtect(target, 4, PAGE_READWRITE, &oldProtect);
	*target = DummyGetPrivateProfileStringA;
	VirtualProtect(target, 4, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
#ifdef _DEBUG
	printf("function at %#x hooked!\n", target);
#endif // _DEBUG
	return true;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return true;
}

extern "C" __declspec(dllexport) int getPriority()
{
	return -1;
}