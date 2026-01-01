#pragma once
#include <helper.hpp>
#include "..\structs.h"
extern uintptr_t cg_game_offset;
extern uintptr_t ui_offset;
extern uintptr_t game_offset;

constexpr auto BASE_GAME = 0x30000000;
constexpr auto BASE_CGAME = 0x30000000;
constexpr auto BASE_UI = 0x40000000;



namespace game
{


	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t mp_address, const ptrdiff_t offset = 0) :
			mp_object(reinterpret_cast<T*>(mp_address)),
			offset(offset)
		{
		}

		T* get() const
		{
			T* ptr = mp_object;
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
		T* mp_object;
		ptrdiff_t offset;
	};

}

uintptr_t cg(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

uintptr_t ui(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

uintptr_t g(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);


uintptr_t exe(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);

uintptr_t sp_mp(uintptr_t CODUOSP, uintptr_t CODUOMP = 0);
typedef int(__cdecl* Com_PrintfT)(const char* format, ...);
extern Com_PrintfT Com_Printf;
extern uint32_t* player_flags;

namespace game {
	void __cdecl Cmd_AddCommand(const char* command_name, void* function);

	extern int FS_FOpenFileByMode(const char* qpath, fileHandle_t* f, fsMode_t mode);

	extern int FS_FCloseFile(fileHandle_t* f);
	extern int FS_GetFileList(const char* path, const char* extension, char* listbuf, int bufsize);
	extern void SCR_DrawStringExt(int x, int y, float size, const char* string, float* setColor, qboolean forceColor);
	extern void SCR_DrawString(float x, float y, int fontID, float scale, float* color, const char* text, float spaceBetweenChars, int maxChars, int arg9);
}