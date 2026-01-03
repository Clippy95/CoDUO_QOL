#include "framework.h"
#include "shellapi.h"
std::wstring rootPath;
bool GameIsLargeAddressAware()
{
    static PBYTE module_base = reinterpret_cast<PBYTE>(GetModuleHandle(nullptr));

    PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);
    PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + dos_header->e_lfanew);

    return (nt_headers->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) == IMAGE_FILE_LARGE_ADDRESS_AWARE;
}

//https://programmerall.com/article/8899839695/
uint32_t calc_checksum(uint32_t checksum, void* data, int length) {
    if (length && data != nullptr) {
        uint32_t sum = 0;
        do {
            sum = *(uint16_t*)data + checksum;
            checksum = (uint16_t)sum + (sum >> 16);
            data = (char*)data + 2;
        } while (--length);
    }

    return checksum + (checksum >> 16);
}

void LAACheck()
{
    static bool LAAChecked = false;
    if (LAAChecked)
        return; // user was prompted/EXE checked already, exit out

    std::wstring dontAskAgainPath = rootPath + L"dontaskagain.LAA";
    if (GetFileAttributesW(dontAskAgainPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        return; // "dontaskagain.LAA" file exists, skipping check
    }

    LAAChecked = true; // prevent us from checking more than once

    if (GameIsLargeAddressAware())
        return; // Game is already 4GB/LAA patched, exit out

    if (MessageBoxA(NULL,
        "Your game executable is missing the 4GB / LAA patch.\n\nDo you want CoDUO_QoL to patch the game's .exe for you? (Recommended)",
        "4GB / Large Address Aware patch missing!",
        MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        char module_path_array[4096];
        GetModuleFileNameA(GetModuleHandle(nullptr), module_path_array, 4096);

        std::string module_path = module_path_array;
        std::string module_path_new = module_path + ".new";
        std::string module_path_bak = module_path + ".bak";

        if (GetFileAttributesA(module_path_new.c_str()) != 0xFFFFFFFF)
            DeleteFileA(module_path_new.c_str());

        if (GetFileAttributesA(module_path_bak.c_str()) != 0xFFFFFFFF)
            DeleteFileA(module_path_bak.c_str());

        BOOL result_CopyFileA = CopyFileA(module_path.c_str(), module_path_new.c_str(), false);

        FILE* file;
        int LAA_ErrorNum = fopen_s(&file, module_path_new.c_str(), "rb+");
        if (LAA_ErrorNum == 0)
        {
            fseek(file, 0, SEEK_END);
            std::vector<uint8_t> exe_data(ftell(file));
            fseek(file, 0, SEEK_SET);

            if (fread(exe_data.data(), 1, exe_data.size(), file) != exe_data.size())
            {
                fclose(file);
                LAA_ErrorNum = 1;
            }
            else
            {
                PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(exe_data.data());
                PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(exe_data.data() + dos_header->e_lfanew);

                // Set LAA flag in PE headers
                nt_headers->FileHeader.Characteristics |= IMAGE_FILE_LARGE_ADDRESS_AWARE;

                // Fix up PE checksum
                uint32_t header_size = (uintptr_t)nt_headers - (uintptr_t)exe_data.data() +
                    ((uintptr_t)&nt_headers->OptionalHeader.CheckSum - (uintptr_t)nt_headers);

                // Skip over CheckSum field
                uint32_t remain_size = (exe_data.size() - header_size - 4) / sizeof(uint16_t);
                void* remain = &nt_headers->OptionalHeader.Subsystem;

                uint32_t header_checksum = calc_checksum(0, exe_data.data(), header_size / sizeof(uint16_t));
                uint32_t file_checksum = calc_checksum(header_checksum, remain, remain_size);
                if (exe_data.size() & 1)
                    file_checksum += *((char*)exe_data.data() + exe_data.size() - 1);

                nt_headers->OptionalHeader.CheckSum = file_checksum + exe_data.size();

                fseek(file, dos_header->e_lfanew, SEEK_SET);
                auto wrote = fwrite(nt_headers, sizeof(IMAGE_NT_HEADERS), 1, file);
                fclose(file);

                if (wrote != 1)
                {
                    LAA_ErrorNum = 2;
                }
                else
                {
                    BOOL result_moveFile1 = MoveFileExA(module_path.c_str(), module_path_bak.c_str(), MOVEFILE_REPLACE_EXISTING);
                    BOOL result_moveFile2 = MoveFileA(module_path_new.c_str(), module_path.c_str());
                    if (!result_moveFile1)
                        LAA_ErrorNum = 3;
                    else if (!result_moveFile2)
                        LAA_ErrorNum = 4;

                    if (result_moveFile1 && !result_moveFile2)
                    {
                        // Try restoring the original EXE if replacement failed
                        MoveFileA(module_path_bak.c_str(), module_path.c_str());
                    }
                }
            }
        }

        if (LAA_ErrorNum == 0)
        {
            MessageBoxA(NULL, "CoDUO_QoL has successfully patched the game's .exe and also created a backup copy.\n\n",
                "4GB / Large Address Aware patch successful!", 0);
        }
        else
        {
            char errText[256];
            sprintf_s(errText, "CoDUO_QoL failed to patch the game's .exe (error %d)\n\nYou can manually patch it yourself by using the \"NTCore 4GB Patch\" tool - an internet search should help find it!", LAA_ErrorNum);
            MessageBoxA(NULL, errText, "4GB / Large Address Aware patch failed...", MB_ICONEXCLAMATION);
        }
    }
    else {
        // New dialog to ask if the user doesn't want to be prompted again
        if (MessageBoxA(NULL,
            "CoDUO_QoL has not patched the game's .exe, would you like to stop future reminders?",
            "Stop Patch Notifications",
            MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
        {
            // Create a file to remember the user's choice
            HANDLE file = CreateFileW(dontAskAgainPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file != INVALID_HANDLE_VALUE) {
                CloseHandle(file);  // Just need to create the file, close it immediately
            }
        }
    }
}
