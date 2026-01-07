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
bool GetGameScreenRes(vector2& res);
double process_width(double width);
double process_widths(double width); 
namespace gui {
    cevar_s* branding = nullptr;
    cevar_s* cg_ammo_overwrite_size_enabled = nullptr;
    cevar_s* cg_ammo_overwrite_size = nullptr;
	void draw_branding() {
        if (!branding || !branding->base || !branding->base->integer)
            return;

        auto x = 2.f - (float)process_width(0) * 0.5f;
        auto y = 8.f;
        auto fontID = 1;
        const auto scale = 0.16f;
        float color[4] = { 1.f, 1.f, 1.f, 0.50f * 0.7f };
        float color_shadow[4] = { 0.f, 0.f, 0.f, 0.80f * 0.7f };
        auto text = MOD_NAME " r" BUILD_NUMBER_STR;
        if (branding->base->integer != 2) {
            game::SCR_DrawString(x + 1, y + 1, fontID, scale, color_shadow, text, NULL, NULL, NULL);
        }
        game::SCR_DrawString(x, y, fontID, scale, color, text, NULL, NULL, NULL);
	}
    SafetyHookInline RE_EndFrameD;
    int __cdecl RE_EndFrame_hook(DWORD* a1, DWORD* a2) {
        draw_branding();
        return RE_EndFrameD.unsafe_ccall<int>(a1, a2);
    }

    void* VM_call_og;

    void __stdcall DrawBlack() {
        float black[] = {0.f,0.f,0.f,1.f};
        game::SetColor(black);
        vector2 res;
        GetGameScreenRes(res);
        game::drawStretchPic(-process_widths(0), 0.f, res.x + process_widths(0), res.y, 0.f, 0.f, 0.f, 0.f,*game::whiteShader);
    }
    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            branding = Cevar_Get("branding", 1, CVAR_ARCHIVE, 0, 2);
            auto pattern = hook::pattern("A1 ? ? ? ? 57 33 FF 3B C7 0F 84 ? ? ? ? A1");
            if (!pattern.empty()) {
                RE_EndFrameD = safetyhook::create_inline(pattern.get_first(), RE_EndFrame_hook);
            }
            cg_ammo_overwrite_size = Cevar_Get("cg_ammo_overwrite_size", 0.325f,CVAR_ARCHIVE);
            cg_ammo_overwrite_size_enabled = Cevar_Get("cg_ammo_overwrite_size_enabled", 1, CVAR_ARCHIVE);
            pattern = hook::pattern("FF 15 ? ? ? ? 6A 00 FF 15 ? ? ? ? 83 C4 ? A1");
            if (!pattern.empty()) {
                Memory::VP::Nop(pattern.get_first(), 6);
            }

        }

        void post_cgame() override
        {
            HMODULE cg = (HMODULE)cg_game_offset;

            auto pattern = hook::pattern(cg, "52 50 8D 74 24 ? E8 ? ? ? ? 83 C4 ? 5F 5E 5B 83 C4 ? C3 8B 4C 24 ? 8B 54 24 ? 8B 44 24 ? 51");

            if (!pattern.empty()) {
                CreateMidHook(pattern.get_first(), [](SafetyHookContext& ctx) {

                    if (cg_ammo_overwrite_size_enabled->base->integer && cg_ammo_overwrite_size->base->value) {

                        float& scale = *(float*)&ctx.eax;
                        scale = cg_ammo_overwrite_size->base->value;


                    }


                    });
            }

        }

    };

}
REGISTER_COMPONENT(gui::component);