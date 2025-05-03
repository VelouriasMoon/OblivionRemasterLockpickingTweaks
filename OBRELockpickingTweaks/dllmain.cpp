// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Signature.h"
#include "SDK.hpp"
#include "magic_enum.hpp"
#include "OBSE/obse64/PluginAPI.h"

inline OBSEMessagingInterface* OBSE_MESSAGE = nullptr;
inline PluginHandle PLUGIN_HANDLE = kPluginHandle_Invalid;
inline HMODULE DLL_HANDLE = nullptr;


#pragma region Sigs

void* SIG_UVModernLockpickMenu_GetTumblersToReset = sigScan("\xB8\x1F\x85\xEB\x51\xF7\xEA\xC1\xFA\x03", "xxxxxxxxxx");

#pragma endregion

int GetSolvedTumblers(SDK::UWBP_ModernMenu_LockPick_C* Menu)
{
    int Sloved = 0;
    int Difficulty = magic_enum::enum_integer(Menu->CurrentDifficulty);
    if (Menu->Tumbler1->IsSolved && Difficulty >= 0) Sloved++;
    if (Menu->Tumbler2->IsSolved && Difficulty >= 1) Sloved++;
    if (Menu->Tumbler3->IsSolved && Difficulty >= 2) Sloved++;
    if (Menu->Tumbler4->IsSolved && Difficulty >= 3) Sloved++;
    if (Menu->Tumbler5->IsSolved && Difficulty >= 4) Sloved++;
    return Sloved;
}

#pragma region Hooks

HOOK(int, __stdcall, Hook_UVModernLockpickMenu_GetTumblersToReset, SIG_UVModernLockpickMenu_GetTumblersToReset, SDK::UVModernLockpickMenu* _this, int SecuritySkillLevel)
{
    //Try to check if function is coming from the widget, it always should be but you never know
    if (_this->IsA(SDK::UWBP_ModernMenu_LockPick_C::StaticClass()))
    {
        SDK::UWBP_ModernMenu_LockPick_C* menu = static_cast<SDK::UWBP_ModernMenu_LockPick_C*>(_this);
        int SolvedTumblers = GetSolvedTumblers(menu);
        int TumblersToFall = ceil(SolvedTumblers - SecuritySkillLevel / 25);
        printf("Solved Tumblers: %i, Tumblers to Fall %i\n", SolvedTumblers, TumblersToFall);
        return TumblersToFall;
    }
    else
    {
        //Original Calcluation
        return 4 - SecuritySkillLevel / 25;
    }
}

#pragma endregion

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (SIG_UVModernLockpickMenu_GetTumblersToReset)
        {
            printf("Found hook at %p\n", SIG_UVModernLockpickMenu_GetTumblersToReset);
            INSTALL_HOOK(Hook_UVModernLockpickMenu_GetTumblersToReset);
        }
        return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C"
{
    __declspec(dllexport) OBSEPluginVersionData OBSEPlugin_Version =
    {
         OBSEPluginVersionData::kVersion,
         1,
         "Lockpicking Tweaks",
         "Moonling",
         OBSEPluginVersionData::kAddressIndependence_Signatures,
         OBSEPluginVersionData::kStructureIndependence_NoStructs
    };

    __declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface* obse)
    {
        PLUGIN_HANDLE = obse->GetPluginHandle();
        OBSE_MESSAGE = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);

        return true;
    }
};

