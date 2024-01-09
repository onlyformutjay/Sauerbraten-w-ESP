#include "mem.h"
#include "pch.h"
void mem::PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess)
{
	DWORD oldProtect;
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	WriteProcessMemory(hProcess, dst, src, size, nullptr);
	VirtualProtectEx(hProcess, dst, size, oldProtect, &oldProtect);

}
void mem::NopEx(BYTE* dst, unsigned int size, HANDLE hProcess)
{
	BYTE* nopArray = new BYTE[size];
	memset(nopArray, 0x90, size);
	PatchEx(dst, nopArray, size, hProcess);
	delete[] nopArray;
}

uintptr_t mem::FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

//internal
void mem::Patch(BYTE* dst, BYTE* src, unsigned int size)
{
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldProtect, &oldProtect);

}
void mem::Nop(BYTE* dst, unsigned int size)
{
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset(dst, 0x90, size);
	VirtualProtect(dst, size, oldProtect, &oldProtect);
}

uintptr_t mem::IntFindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}

bool mem::Detour32(BYTE* src, BYTE* dst, const uintptr_t len)
{
	if (len < 5)
		return false;

	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	uintptr_t relativeAddy = dst - src - 5;
	*src = 0xE9;

	*(uintptr_t*)(src + 1) = relativeAddy;
	VirtualProtect(src, len, curProtection, &curProtection);
	return true;
}

BYTE* mem::TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len)
{
	if (len < 5)
		return 0;
	
	//create gateway
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//write stolen bytes to gateway
	memcpy_s(gateway, len, src, len);

	//get gateway to destination address
	uintptr_t gatewayRelativeAddy = src - gateway - 5;

	//add the jmp opcodes to end of gateway
	*(gateway + len) = 0xE9;

	//write addy of gateway to jump
	*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddy;

	//perform the detour
	Detour32(src, dst, len);

	return gateway;
}