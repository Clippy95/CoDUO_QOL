#pragma once
#include <helper.hpp>
#include "..\structs.h"
#include "..\loader\component_loader.h"
#include <Hooking.Patterns.h>
#include "..\framework.h"
#define WEAK __declspec(selectany)

extern uintptr_t cg_game_offset;
extern uintptr_t ui_offset;
extern uintptr_t game_offset;

constexpr auto BASE_GAME = 0x20000000;
constexpr auto BASE_CGAME = 0x30000000;
constexpr auto BASE_UI = 0x40000000;

uintptr_t cg(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

uintptr_t ui(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

uintptr_t g(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);


uintptr_t exe(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

uintptr_t sp_mp(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

namespace game
{


	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t sp_address, const size_t mp_address = 0, const ptrdiff_t offset = 0) :
			sp_object(reinterpret_cast<T*>(sp_address)),
			mp_object(reinterpret_cast<T*>(mp_address)),
			offset(offset)
		{
		}

		T* get() const
		{
			T* ptr = exe(1, 0) ? sp_object : mp_object;
			uintptr_t base_address = 0;

			switch (offset)
			{
			case BASE_CGAME:
				base_address = cg_game_offset;
				break;
			case BASE_UI:
				base_address = ui_offset;
				break;
			default:
				return ptr;
			}

			return reinterpret_cast<T*>(base_address + (reinterpret_cast<uintptr_t>(ptr) - offset));
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}
	private:
		T* sp_object;
		T* mp_object;
		ptrdiff_t offset;
	};

	enum game_module {
		EXE,
		CGAME,
		UI,
		GAME,
		MAX_GAME_MODULE,
	};

    template <typename T>
    class symbolp
    {
    public:
        symbolp(std::initializer_list<std::string_view> patterns, const signed int offset_val = 0, game_module module = EXE, bool dereference = false)
            : patterns(patterns), offset_val(offset_val), module(module), dereference(dereference), object(nullptr)
        {
            register_as_component();
        }

        symbolp(std::string_view pattern_str, const signed int offset_val = 0, game_module module = EXE, bool dereference = false)
            : patterns({ pattern_str }), offset_val(offset_val), module(module), dereference(dereference), object(nullptr)
        {
            register_as_component();
        }

        T* get() const
        {
            return object;
        }

        operator T* () const
        {
            return this->get();
        }

        T* operator->() const
        {
            return this->get();
        }

        //bool operator==(const T& other) const {
        //    return (this->get() == other);
        //}

    private:
        void register_as_component()
        {
            class symbolp_resolver : public component_interface
            {
            public:
                symbolp_resolver(symbolp* owner, game_module module) : owner(owner), module(module) {}

                void post_start() override
                {
                    if (module == EXE) {
                        owner->resolve();
                    }
                }

                void post_cgame() override {
                    if (module == CGAME)
                        owner->resolve();
                }

                void post_ui() override {
                    if (module == UI)
                        owner->resolve();
                }

                void post_game_sp() override {
                
                    if (module == GAME)
                        owner->resolve();

                }

            private:
                symbolp* owner;
                game_module module;
            };

            auto resolver = std::make_unique<symbolp_resolver>(this, module);
            component_loader::register_component(std::move(resolver));
        }

        void resolve()
        {
            uintptr_t search_base = (uintptr_t)GetModuleHandle(NULL);

            switch (module)
            {
            case CGAME:
                search_base = cg_game_offset;
                break;
            case UI:
                search_base = ui_offset;
                break;
            case GAME:
                search_base = game_offset;
                break;
            }


            for (const auto& pattern_str : patterns)
            {
                hook::pattern pattern((HMODULE)search_base, pattern_str);

                if (!pattern.empty())
                {
                    uintptr_t address = reinterpret_cast<uintptr_t>(pattern.get_first()) + offset_val;

                    if (dereference)
                    {
                        object = *reinterpret_cast<T**>(address);
                    }
                    else
                    {
                        object = reinterpret_cast<T*>(address);
                    }
                    return;
                }
            }

            // All patterns failed - build error message
            object = nullptr;

            std::string error_msg = "symbolp: All patterns failed to resolve!\nPatterns tried:\n";
            for (size_t i = 0; i < patterns.size(); ++i)
            {
                error_msg += std::to_string(i + 1) + ": \"" + std::string(patterns[i]) + "\"\n";
            }

            MessageBoxA(NULL, error_msg.c_str(), MOD_NAME": Error", MB_ICONSTOP);
        }

        std::vector<std::string_view> patterns;
        signed int offset_val;
        game_module module;
        bool dereference;
        mutable T* object;
    };

}


typedef int(__cdecl* Com_PrintfT)(const char* format, ...);
extern Com_PrintfT Com_Printf;
extern uint32_t* player_flags;

namespace game {
	//void __cdecl Cmd_AddCommand(const char* command_name, void* function);

	extern int FS_FOpenFileByMode(const char* qpath, fileHandle_t* f, fsMode_t mode);

	extern int FS_FCloseFile(fileHandle_t* f);
	extern int FS_GetFileList(const char* path, const char* extension, char* listbuf, int bufsize);
	extern void SCR_DrawStringExt(int x, int y, float size, const char* string, float* setColor, qboolean forceColor);
	extern void SCR_DrawString(float x, float y, int fontID, float scale, float* color, const char* text, float spaceBetweenChars, int maxChars, int arg9);

	//WEAK symbol<qhandle_t> whiteShader{ 0x48422E8 };

    WEAK symbolp<qhandle_t> whiteShader{ "A1 ? ? ? ? 8B 15 ? ? ? ? ? ? ? 83 C4",1,game::EXE,true};
    WEAK symbolp<void(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader)> drawStretchPic{ "8B 0D ? ? ? ? 8B 81 ? ? ? ? 83 C0 ? 3D ? ? ? ? 0F 87 ? ? ? ? 56 8D B4 08 ? ? ? ? 85 F6 89 81 ? ? ? ? 74" };

    WEAK symbolp<int(const char* command_name, void* function)> Cmd_AddCommand{ "85 C0 53 55 8B 6C 24 ? 56 57 8B F8 74 ? 8B 77",-5 };

	//WEAK symbol<void(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader)> drawStretchPic{ 0x4DF240 };
	WEAK symbol<void(float *color)> SetColor{ 0x4DF1B0 };
    WEAK symbolp<qhandle_t> cstate{ "A1 ? ? ? ? 83 E8 00",1,game::EXE,true };
    WEAK symbolp<qhandle_t> keycatchers{ "F6 05 ? ? ? ? ? 74 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? DF E0",2,game::EXE,true };
}