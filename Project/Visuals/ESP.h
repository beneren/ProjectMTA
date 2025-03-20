#pragma once
#include "..\Main\main.h"
#include "CClientEntity.h"
extern CClientEntity* AimPlayer;
class MTAInfo {
public:
	char pad_0x00[0xC];
	int max;
	int temp2;
};
enum eBone {
    BONE_PELVIS1 = 1,
    BONE_PELVIS,
    BONE_SPINE1,
    BONE_UPPERTORSO,
    BONE_NECK,
    BONE_HEAD2,
    BONE_HEAD1,
    BONE_HEAD,
    BONE_RIGHTUPPERTORSO = 21,
    BONE_RIGHTSHOULDER,
    BONE_RIGHTELBOW,
    BONE_RIGHTWRIST,
    BONE_RIGHTHAND,
    BONE_RIGHTTHUMB,
    BONE_LEFTUPPERTORSO = 31,
    BONE_LEFTSHOULDER,
    BONE_LEFTELBOW,
    BONE_LEFTWRIST,
    BONE_LEFTHAND,
    BONE_LEFTTHUMB,
    BONE_LEFTHIP = 41,
    BONE_LEFTKNEE,
    BONE_LEFTANKLE,
    BONE_LEFTFOOT,
    BONE_RIGHTHIP = 51,
    BONE_RIGHTKNEE,
    BONE_RIGHTANKLE,
    BONE_RIGHTFOOT
};
enum eWeaponSlot
{
    WEAPONSLOT_TYPE_UNARMED = 0,
    WEAPONSLOT_TYPE_MELEE,
    WEAPONSLOT_TYPE_HANDGUN,
    WEAPONSLOT_TYPE_SHOTGUN,
    WEAPONSLOT_TYPE_SMG,            // 4
    WEAPONSLOT_TYPE_MG,
    WEAPONSLOT_TYPE_RIFLE,
    WEAPONSLOT_TYPE_HEAVY,
    WEAPONSLOT_TYPE_THROWN,
    WEAPONSLOT_TYPE_SPECIAL,              // 9
    WEAPONSLOT_TYPE_GIFT,                 // 10
    WEAPONSLOT_TYPE_PARACHUTE,            // 11
    WEAPONSLOT_TYPE_DETONATOR,            // 12

    WEAPONSLOT_MAX
};
template <class T, int SIZE>
struct SFixedArray
{
    T& operator[](uint uiIndex)
    {
        assert(uiIndex < SIZE);
        return data[uiIndex];
    }

    const T& operator[](uint uiIndex) const
    {
        assert(uiIndex < SIZE);
        return data[uiIndex];
    }

    T data[SIZE];
};
static char* ReadString(DWORD addr, int size)
{
    char* str = new char[size];
    for (int i = 0; i < size; ++i)
    {
        str[i] = *(char*)(addr + i);
    }
    return str;
}
static char* ReadName(DWORD addr)
{
    int name_len = *(int*)(addr + 0x10);

    char* nName;
    if (name_len <= 15)
    {
        nName = (ReadString(addr, name_len + 1));
    }
    else {
        nName = (ReadString(*(int*)(addr), name_len + 1));
    }
    return nName;
}

void Player_ESP();
