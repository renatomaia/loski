/*
http://msdn.microsoft.com/en-us/library/aa302340.aspx#win32map_processandthreadfunctions

How to list all processes:
Win32:  http://support.microsoft.com/kb/187913
MacOSX: http://developer.apple.com/mac/library/qa/qa2001/qa1123.html
POSIX:  http://www.opengroup.org/onlinepubs/000095399/utilities/ps.html
*/

static void fill_env(lua_State *L,  char* buffer)
{
	int n = lua_getn(L, -1);	// n = size of "env" table
	int j = 0;

	for( int i = 1; i <= n ; i++)
	{
		lua_pushnumber(L, (double)i);
		lua_gettable(L, -2);
		const char* str = lua_tostring(L, -1);
		strcpy(&buffer[j],str);
		j += (strlen(str) +1 );
		lua_pop(L, 1);
	}
}

static WORD getShowMode(const char* str_mode)
{
	if (!strcmp("SW_SHOWMAXIMIZED", str_mode))
		return SW_SHOWMAXIMIZED;
	else if (!strcmp("SW_SHOWMINIMIZED", str_mode))
		return SW_SHOWMINIMIZED;
	else if (!strcmp("SW_SHOWMINNOACTIVE", str_mode))
		return SW_SHOWMINNOACTIVE;
	else if (!strcmp("SW_RESTORE", str_mode))
		return SW_RESTORE;
	else
		return SW_SHOWNORMAL;
}

static int _create_process (lua_State *L)
{
	const char* commandline;
	WORD show_mode = SW_SHOWNORMAL;
	WORD flags = NORMAL_PRIORITY_CLASS;
	const char* dir = NULL;
	char* env = NULL;
	static char buffer[1024];

	luaL_arg_check(L, lua_istable(L,1) || lua_isstring(L,1),1,"must be a table or string");
	if (lua_isstring(L,1))
		commandline = lua_tostring(L,1);
	else
	{
		lua_pushstring(L, "cmd");
		lua_gettable(L, 1);
		if (lua_isstring(L, -1))
		  commandline = lua_tostring(L, -1);
		else
			LuaCompat::error(L, "The cmd field must be provided");

		lua_pushstring(L, "console");
		lua_gettable(L, 1);
		if (!lua_isnil(L, -1))
			flags |= CREATE_NEW_CONSOLE;

		lua_pushstring(L, "show_mode");
		lua_gettable(L, 1);
		if (lua_isstring(L, -1))
			show_mode = getShowMode(lua_tostring(L, -1));

		lua_pushstring(L, "dir");
		lua_gettable(L, 1);
		if (lua_isstring(L, -1))
			dir = lua_tostring(L, -1);

		lua_pushstring(L, "env");
		lua_gettable(L, 1);
		if (lua_istable(L, -1))
		{
			fill_env(L, buffer);
			env = buffer;
		}
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = show_mode;
	BOOL pr = CreateProcess(NULL,(char*)commandline,NULL,NULL,FALSE,
									flags,env,dir,&si,&pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	if (!pr)
		lua_pushnil(L);
	else
		lua_pushnumber(L,pi.dwProcessId);
	return 1;
}

static int _kill_process(lua_State *L)
{
	DWORD pid = (DWORD)luaL_check_number(L,1);
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,0,pid);

	if (hProcess != NULL)
	{
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	return 0;
}

static int _sleep(lua_State* L)
{
	double dtime = luaL_check_number(L,1);
	DWORD time = (DWORD) (dtime*1000);
	Sleep(time);
	return 0;
}

static void init_proc_utils(lua_State *L)
{
	lua_register(L, "create_process",_create_process);
	lua_register(L, "kill_process",_kill_process);
	lua_register(L, "sleep",_sleep);
}


















int process_init()
{
	return 0;
}

int process_end()
{
	return 0;
}

int process_create(Process *proc,
                   const char *binpath,
                   const char *runpath,
                   char *const argvals[],
                   char *const envlist[],
                   FILE *stdin,
                   FILE *stdout,
                   FILE *stderr)
{
	const char* commandline;
	WORD show_mode = SW_SHOWNORMAL;
	WORD flags = NORMAL_PRIORITY_CLASS;
	const char* dir = NULL;
	char* env = NULL;
	static char buffer[1024];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = show_mode;
	BOOL pr = CreateProcess(NULL,(char*)commandline,NULL,NULL,FALSE,
									flags,env,dir,&si,&pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

int process_status(Process *proc, int *status)
{
}

int process_exitval(Process *proc, int *code)
{
}

int process_kill(Process *proc)
{
}

int process_discard(Process *proc)
{
}
