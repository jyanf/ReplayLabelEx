//Initial Author: ���ˤο���
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
#define isCL (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT)

namespace Keys {
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
	static const std::array<std::wstring, 4> BattleResult = {
		L"win", L"lose", L"noed", L"draw"
	};
	static const std::wstring& GetBattleResult()
	{
		SokuLib::BattleMode comMode = SokuLib::mainMode;
		SokuLib::BattleManager battleMgr = SokuLib::getBattleMgr();
		const char lWin = battleMgr.leftCharacterManager.score;
		const char rWin = battleMgr.rightCharacterManager.score;
		const char& myWin = (comMode == SokuLib::BATTLE_MODE_VSCLIENT) ? rWin : lWin;
		const char& otherWin = (comMode == SokuLib::BATTLE_MODE_VSCLIENT) ? lWin : rWin;

		if (myWin == 2) {
			if (otherWin == 2) return BattleResult[3];
			return BattleResult[0];
		}
		if (otherWin == 2) {
			return BattleResult[1];
		}
		return BattleResult[2];
	}
	static const std::wstring GetSelfProfile()
	{
		//SokuLib::player1Profile;
		return isCL ? L"%p2" : L"%p1";
	}
	static const std::wstring GetOppoProfile()
	{
		return isCL ? L"%p1" : L"%p2";
	}
	static const std::wstring GetSelfChar()
	{
		return isCL ? L"%c2" : L"%c1";
	}
	static const std::wstring GetOppoChar()
	{
		return isCL ? L"%c1" : L"%c2";
	}
	
};

static std::array<std::wstring, 2> Inis = { L"ReplayLabelEx.ini", L"CharacterNames.ini" };

static bool GetChName(const std::vector<SokuLib::Character>& ChSerials, std::vector<std::wstring>& names)
{//����ɫ���ȡ��ɫ������д��
	auto iniPath = Inis[1].c_str();
	//���ini return false
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
{//ģ�黻��ǩ���ؼ��ֻ�ʵֵ
	auto iniPath = Inis[0].c_str();
	//���ini return false
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
	const DWORD resultVs = GetPrivateProfileStringW(L"PathFormat", L"file_vs", L"err_invalid_path", tempw, maxsize, iniPath);
	path = tempw;
	//wprintf(L"%s as %s", tempw, path.c_str());


	//��ģʽչ��
	GetPrivateProfileStringW(L"BattleMode",		L"label",	L"%md",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	GetPrivateProfileStringW(L"BattleMode", Keys::GetBattleMode().c_str(), NULL, tempw, maxsize, iniPath);
	subs.push_back(tempw);
	//GetPrivateProfileStringW(L"BattleMode",		L"vs_com", L"local", tempw, maxsize, iniPath);
	//GetPrivateProfileStringW(L"BattleMode",		L"vs_loc",	L"local\\%p1 ��%p2", tempw, maxsize, iniPath);
	//GetPrivateProfileStringW(L"BattleMode",		L"vs_net",	L"online\\%po",tempw, maxsize, iniPath);
	//GetPrivateProfileStringW(L"BattleMode",		L"spectate",L"watch\\%p1",tempw, maxsize, iniPath);

	//��ս���
	GetPrivateProfileStringW(L"BattleResult",	L"label",	L"%ed",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
#ifdef _DEBUG
	wprintf(L"mode:%d\n", SokuLib::mainMode);
#endif
	GetPrivateProfileStringW(L"BattleResult", Keys::GetBattleResult().c_str(), NULL, tempw, maxsize, iniPath);
	subs.push_back(tempw);
	//GetPrivateProfileStringW(L"BattleResult",	L"win",		L"win",		tempw, maxsize, iniPath);
	//GetPrivateProfileStringW(L"BattleResult",	L"lose",	L"lose",	tempw, maxsize, iniPath);
	//GetPrivateProfileStringW(L"BattleResult",	L"draw",	L"draw",	tempw, maxsize, iniPath);
	//GetPrivateProfileStringW(L"BattleResult",	L"no_game", L"no_game", tempw, maxsize, iniPath);
	
	//�Ի���ǩ
	GetPrivateProfileStringW(L"SelfProfileName",L"label",	L"%ps",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetSelfProfile());
	//���ֻ�ǩ
	GetPrivateProfileStringW(L"OpponentProfileName", L"label", L"%po",	tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetOppoProfile());
	//�Ի���ɫ
	GetPrivateProfileStringW(L"SelfCharacter",	L"label",	L"%cs",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetSelfChar());
	//���ֽ�ɫ
	GetPrivateProfileStringW(L"OpponentCharacter", L"label",L"%co",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(Keys::GetOppoChar());

	//��չԭ���ɫ��
	std::vector<std::wstring> CharNamesW;
	std::vector<SokuLib::Character> Serials = { SokuLib::leftChar, SokuLib::rightChar };
	if (!GetChName(Serials, CharNamesW)) CharNamesW = {L"%c1", L"%c2"};
	GetPrivateProfileStringW(L"P1CharacterW",	L"label",	L"%C1",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(CharNamesW[0]);
	GetPrivateProfileStringW(L"P2CharacterW",	L"label",	L"%C2",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(CharNamesW[1]);
	GetPrivateProfileStringW(L"SelfCharacterW",	L"label",	L"%Cs",		tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(isCL ? CharNamesW[1] : CharNamesW[0]);
	GetPrivateProfileStringW(L"OpponentCharacterW", L"label",L"%Co",	tempw, maxsize, iniPath);
	labels.push_back(tempw);
	subs.push_back(isCL ? CharNamesW[0] : CharNamesW[1]);

	//��ɫ���б�
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
	/* {
		L"BattleMode",
		L"BattleResult",
		L"SelfProfileName",
		L"OpponentProfileName",
		L"SelfCharacter",
		L"OpponentCharacter",
		L"SelfCharacterW",
		L"OppoCharacterW",
		L"P1CharacterW",
		L"P2CharacterW"
	};*/
	std::vector<std::wstring> subs;
	/*{
		Keys::GetBattleMode(),
		Keys::GetBattleResult(),
		Keys::GetSelfProfile(),
		Keys::GetOppoProfile(),
		Keys::GetSelfChar(),
		Keys::GetOppoChar()
	};*/

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
	//if (WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, lpReturnedString, 0, NULL, NULL) > FILEVS_LIMIT) return result;
	result = WideCharToMultiByte(CP_THREAD_ACP, 0, path.c_str(), -1, lpReturnedString, FILEVS_LIMIT, NULL, NULL);
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