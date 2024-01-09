#pragma once
#include <Windows.h>
#include <vector>
#include "pch.h"

namespace mem {
	void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess);
	void NopEx(BYTE* dst, unsigned int size, HANDLE hProcess);

	uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);

	//internal versions
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	void Nop(BYTE* dst, unsigned int size);

	uintptr_t IntFindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);

	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len);		//tramp hook
	BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);		//tramp hook
}	