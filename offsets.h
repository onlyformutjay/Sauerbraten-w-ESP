#pragma once
#include "Vec3.h"
#include <Windows.h>
#include <iostream>

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

class ent
{
public:
	//type name offset
	DEFINE_MEMBER_N(int, health, 0x340);
	DEFINE_MEMBER_N(int, armor,  0x348);
	DEFINE_MEMBER_N(Vec3, body, 0x394);
};