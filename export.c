//==========================================================================
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================

#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include "export.h"

HMODULE MEM_REMOTE_HANDLE(DWORD Process_ID, const TCHAR *modName)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;

	HANDLE snapshot = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Process_ID);
	if (!snapshot)
		return NULL;
	if (EnumProcessModulesEx(snapshot, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL))
	{
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];
			if (GetModuleBaseName(snapshot, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				if (_tcscmp(szModName, modName) == 0)
				{
					CloseHandle(snapshot);
					return hMods[i];
				}
			}
		}
	}
	CloseHandle(snapshot);
	return NULL;
}

FARPROC MEM_REMOTE_ADDRESS(HANDLE snapshot, HMODULE hMod, const char *procName)
{
	BYTE *base = (BYTE *)hMod;
	IMAGE_DOS_HEADER dosHeader;
	IMAGE_NT_HEADERS ntHeaders;
	IMAGE_EXPORT_DIRECTORY expDir;

	if (!ReadProcessMemory(snapshot, base, &dosHeader, sizeof(dosHeader), NULL))
		return NULL; // Reads DOS header of the module
	if (!ReadProcessMemory(snapshot, base + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders), NULL))
		return NULL; // Reads the PE header
	if (!ReadProcessMemory(snapshot, base + ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, &expDir, sizeof(expDir), NULL))
		return NULL; // Reads the exported directory

	DWORD *funcs = (DWORD *)malloc(expDir.NumberOfFunctions * sizeof(DWORD));
	DWORD *names = (DWORD *)malloc(expDir.NumberOfNames * sizeof(DWORD));
	WORD *ordinals = (WORD *)malloc(expDir.NumberOfNames * sizeof(WORD));
	if (!ReadProcessMemory(snapshot, base + expDir.AddressOfFunctions, funcs, expDir.NumberOfFunctions * sizeof(DWORD), NULL))
		return NULL;
	if (!ReadProcessMemory(snapshot, base + expDir.AddressOfNames, names, expDir.NumberOfNames * sizeof(DWORD), NULL))
		return NULL;
	if (!ReadProcessMemory(snapshot, base + expDir.AddressOfNameOrdinals, ordinals, expDir.NumberOfNames * sizeof(WORD), NULL))
		return NULL;

	FARPROC addr = NULL;
	for (DWORD i = 0; i < expDir.NumberOfNames; i++)
	{
		char funcName[256];
		if (ReadProcessMemory(snapshot, base + names[i], funcName, sizeof(funcName), NULL) && strcmp(funcName, procName) == 0)
		{
			addr = (FARPROC)(base + funcs[ordinals[i]]);
			break;
		}
	}

	free(funcs);
	free(names);
	free(ordinals);

	return addr;
}