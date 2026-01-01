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
double process_width(double width);
namespace gui {
    cevar_s* branding = nullptr;
	void draw_branding() {
        if (!branding || !branding->base || !branding->base->integer)
            return;

        auto x = 2.f - (float)process_width(0) * 0.5f;
        auto y = 8.f;
        auto fontID = 1;
        const auto scale = 0.16f;
        float color[4] = { 1.f, 1.f, 1.f, 0.50f * 0.7f };
        float color_shadow[4] = { 0.f, 0.f, 0.f, 0.80f * 0.7f };
        auto text = "CODUOQoL r" BUILD_NUMBER_STR;
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

        }
    };

}
REGISTER_COMPONENT(gui::component);