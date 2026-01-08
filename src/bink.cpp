#include <helper.hpp>
#include <game.h>
#include "component_loader.h"
#include "cevar.h"
#include <Hooking.Patterns.h>
#include "utils/hooking.h"

#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

#include <optional>
#include "GMath.h"
#include <buildnumber.h>
#include "framework.h"
cvar_s* mss_volume;
FARPROC GetProcAddress(const char* modulename, const char* lpProcName) {
    auto module = GetModuleHandleA(modulename);
    if (module) {
        return GetProcAddress(module, lpProcName);
    }
    return NULL;
}
uintptr_t BinkSetVolumePtr;
namespace bink_game {
    SafetyHookInline BinkDoFrameog;
     void __stdcall BinkDoFrame_hook(void* bnk) {
         int volume = (int)(32768.0f * std::clamp(mss_volume->value,0.f,1.f));

         stdcall_call<void>(BinkSetVolumePtr, bnk, volume);

         BinkDoFrameog.unsafe_stdcall(bnk);
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            auto BinkDoFrameptr = GetProcAddress("binkw32.dll","_BinkDoFrame@4");
            BinkSetVolumePtr = (uintptr_t)GetProcAddress("binkw32.dll", "_BinkSetVolume@12");

            mss_volume = Cvar_Find("mss_volume");

            if (BinkDoFrameptr && BinkSetVolumePtr && mss_volume) {
                BinkDoFrameog = safetyhook::create_inline(BinkDoFrameptr, BinkDoFrame_hook);
            }



        }
    };
}
REGISTER_COMPONENT(bink_game::component);