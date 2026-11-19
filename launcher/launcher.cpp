// DDNet E-450 Custom launcher
// Builds a temporary config from a profile, picks a random DDNet server
// and runs DDNet.exe. After exit it connects to the next server ("endless levels").
#include <windows.h>

#include <ctime>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

static std::string BaseDir()
{
	char aPath[MAX_PATH] = {};
	DWORD Len = GetModuleFileNameA(nullptr, aPath, MAX_PATH);
	std::string Path(aPath, Len);
	size_t Pos = Path.find_last_of("\\/");
	return Pos == std::string::npos ? Path : Path.substr(0, Pos);
}

static std::string Join(const std::string &Dir, const std::string &Name)
{
	return Dir + "\\" + Name;
}

static std::string ReadText(const std::string &Path)
{
	std::ifstream File(Path, std::ios::binary);
	std::stringstream Buf;
	Buf << File.rdbuf();
	return Buf.str();
}

static bool WriteText(const std::string &Path, const std::string &Text)
{
	std::ofstream File(Path, std::ios::binary | std::ios::trunc);
	if(!File)
		return false;
	File << Text;
	return File.good();
}

static std::string PickServer(const std::string &Path)
{
	std::ifstream File(Path);
	std::vector<std::string> vAddrs;
	std::string Line;
	while(std::getline(File, Line))
	{
		if(!Line.empty() && Line.back() == '\r')
			Line.pop_back();
		if(Line.empty() || Line[0] == '#')
			continue;
		size_t Bar = Line.find('|');
		std::string Addr = Bar == std::string::npos ? Line : Line.substr(0, Bar);
		if(Addr.find(':') != std::string::npos)
			vAddrs.push_back(Addr);
	}
	if(vAddrs.empty())
		return "";

	std::mt19937 Gen((unsigned)GetCurrentProcessId() ^ (unsigned)std::time(nullptr) ^ GetTickCount());
	std::uniform_int_distribution<size_t> Dist(0, vAddrs.size() - 1);
	return vAddrs[Dist(Gen)];
}

static void ShowError(const char *pMessage)
{
	MessageBoxA(nullptr, pMessage, "DDNet Custom", MB_ICONERROR | MB_OK);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR pCmdLine, int)
{
	const std::string Base = BaseDir();

	const std::string CmdLine = pCmdLine == nullptr ? "" : std::string(pCmdLine);
	const bool Beauty = CmdLine.find("--beauty") != std::string::npos;

	const std::string ProfilePath = Join(Base, Beauty ? "profiles\\beauty1000.cfg" : "profiles\\fast.cfg");
	const std::string Profile = ReadText(ProfilePath);
	if(Profile.empty())
	{
		ShowError("Missing profiles\\fast.cfg / profiles\\beauty1000.cfg");
		return 1;
	}

	const std::string LevelsPath = Join(Base, "infinite-levels.txt");
	const std::string ExePath = Join(Base, "DDNet.exe");
	const std::string LaunchCfg = Join(Base, "launch_rotation.cfg");
	const std::string StopFlag = Join(Base, "stop.flag");

	if(GetFileAttributesA(ExePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		ShowError("DDNet.exe not found next to the launcher");
		return 1;
	}

	while(true)
	{
		const std::string Addr = PickServer(LevelsPath);
		if(Addr.empty())
		{
			ShowError("infinite-levels.txt has no valid server addresses");
			return 1;
		}

		WriteText(LaunchCfg, Profile + "\nconnect " + Addr + "\n");

		std::string CmdLine = "\"" + ExePath + "\" -f \"" + LaunchCfg + "\"";
		std::vector<char> Cmd(CmdLine.begin(), CmdLine.end());
		Cmd.push_back('\0');

		STARTUPINFOA Startup = {};
		Startup.cb = sizeof(Startup);
		PROCESS_INFORMATION Process = {};
		if(!CreateProcessA(
			   ExePath.c_str(),
			   Cmd.data(),
			   nullptr,
			   nullptr,
			   FALSE,
			   ABOVE_NORMAL_PRIORITY_CLASS,
			   nullptr,
			   Base.c_str(),
			   &Startup,
			   &Process))
		{
			ShowError("Failed to start DDNet.exe");
			return 1;
		}

		WaitForSingleObject(Process.hProcess, INFINITE);
		CloseHandle(Process.hThread);
		CloseHandle(Process.hProcess);

		if(GetFileAttributesA(StopFlag.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			DeleteFileA(StopFlag.c_str());
			break;
		}
		Sleep(1500);
	}
	return 0;
}
