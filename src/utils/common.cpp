#include "..\framework.h"
// FROM UAL
std::wstring GetModulePath(HMODULE hModule)
{
    static constexpr auto INITIAL_BUFFER_SIZE = MAX_PATH;
    static constexpr auto MAX_ITERATIONS = 7;
    std::wstring ret;
    auto bufferSize = INITIAL_BUFFER_SIZE;
    for (size_t iterations = 0; iterations < MAX_ITERATIONS; ++iterations)
    {
        ret.resize(bufferSize);
        size_t charsReturned = 0;
        charsReturned = GetModuleFileNameW(hModule, ret.data(), bufferSize);
        if (charsReturned < ret.length())
        {
            ret.resize(charsReturned);
            return ret;
        }
        else
        {
            bufferSize *= 2;
        }
    }
    return std::wstring();
}
// FROM UAL
std::wstring GetExeModulePath()
{
    std::wstring r = GetModulePath(NULL);
    r = r.substr(0, r.find_last_of(L"/\\") + 1);
    return r;
}

std::wstring GetCurrentModuleName()
{
    HMODULE hModule = NULL;


    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)&GetCurrentModuleName,
        &hModule
    );

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);

    return std::wstring(path);
}

std::wstring thisModuleFileName()
{
    std::wstring fullPath = GetCurrentModuleName();
    size_t lastSlash = fullPath.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos)
        return fullPath.substr(lastSlash + 1);
    return fullPath;
}