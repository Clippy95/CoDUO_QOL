#include "framework.h"
#include "shared.h"
#include "structs.h"
#include "cevar.h"
#include "safetyhook.hpp"


// ============================================================================
// CEVAR - EXTENDED CVAR SYSTEM WITH OVERLOADED CREATION
// ============================================================================
typedef cvar_t* (__cdecl* Cvar_GetT)(const char* var_name, const char* var_value, int flags);
extern Cvar_GetT Cvar_Get;
extern SafetyHookInline Cvar_Set_og;




std::unordered_map<cvar_t*, cevar_t*> g_cevars;
// ============================================================================
// INTERNAL HELPER - Creates cevar structure
// ============================================================================


cevar_t* Cevar_Internal_Create(const char* var_name, const char* var_value, int flags,
    CvarCallback callback, const cevar_limits& limits) {
    cvar_t* base = Cvar_Get(var_name, var_value, flags);
    if (!base) return nullptr;

    auto it = g_cevars.find(base);  // Use pointer directly
    if (it != g_cevars.end()) {
        if (callback) it->second->callback = callback;
        it->second->limits = limits;
        return it->second;
    }

    cevar_t* cevar = new cevar_t();
    cevar->base = base;
    cevar->callback = callback;
    cevar->limits = limits;

    g_cevars[base] = cevar;  // Use pointer as key
    return cevar;
}

// ============================================================================
// STRING OVERLOADS
// ============================================================================

// String, no limits (same as before)
extern cevar_t* Cevar_Get(const char* var_name, const char* var_value, int flags,
    CvarCallback callback) {
    cevar_limits limits = {};
    limits.has_limits = false;
    return Cevar_Internal_Create(var_name, var_value, flags, callback, limits);
}

// ============================================================================
// FLOAT OVERLOADS
// ============================================================================

// Float, no limits
cevar_t* Cevar_Get(const char* var_name, float var_value, int flags,
    CvarCallback callback) {
    char buffer[32];
    sprintf(buffer, "%f", var_value);

    cevar_limits limits = {};
    limits.has_limits = false;
    return Cevar_Internal_Create(var_name, buffer, flags, callback, limits);
}

// Float with limits (min < max indicates limits are enabled)
cevar_t* Cevar_Get(const char* var_name, float var_value, int flags,
    float min, float max, CvarCallback callback) {
    char buffer[32];
    float clamped = std::clamp(var_value, min, max);
    sprintf(buffer, "%f", clamped);

    cevar_limits limits = {};
    limits.has_limits = (min < max);
    limits.is_float = true;
    limits.f.min = min;
    limits.f.max = max;

    return Cevar_Internal_Create(var_name, buffer, flags, callback, limits);
}

// ============================================================================
// INT OVERLOADS
// ============================================================================

// Int, no limits
cevar_t* Cevar_Get(const char* var_name, int var_value, int flags,
    CvarCallback callback) {
    char buffer[32];
    sprintf(buffer, "%d", var_value);

    cevar_limits limits = {};
    limits.has_limits = false;
    return Cevar_Internal_Create(var_name, buffer, flags, callback, limits);
}

// Int with limits
cevar_t* Cevar_Get(const char* var_name, int var_value, int flags,
    int min, int max, CvarCallback callback) {
    char buffer[32];
    int clamped = std::clamp(var_value, min, max);
    sprintf(buffer, "%d", clamped);

    cevar_limits limits = {};
    limits.has_limits = (min < max);
    limits.is_float = false;
    limits.i.min = min;
    limits.i.max = max;

    return Cevar_Internal_Create(var_name, buffer, flags, callback, limits);
}

// ============================================================================
// CEVAR SETTER IMPLEMENTATIONS
// ============================================================================

// Set from string
bool Cevar_Set(cevar_t* cevar, const char* value) {
    if (!cevar || !cevar->base || !value) return false;

    const char* oldValue = cevar->base->string;

    // If has limits, parse and clamp
    if (cevar->limits.has_limits) {
        if (cevar->limits.is_float) {
            float val = (float)atof(value);
            val = std::clamp(val, cevar->limits.f.min, cevar->limits.f.max);
            char buffer[32];
            sprintf(buffer, "%f", val);
            Cvar_Set_og.ccall<cvar_s*>(cevar->base->name, buffer, true);
        }
        else {
            int val = atoi(value);
            val = std::clamp(val, cevar->limits.i.min, cevar->limits.i.max);
            char buffer[32];
            sprintf(buffer, "%d", val);
            Cvar_Set_og.ccall<cvar_s*>(cevar->base->name, buffer, true);
        }
    }
    else {
        // No limits, set directly
        Cvar_Set_og.ccall<cvar_s*>(cevar->base->name, value, true);
    }

    // Trigger callback if value changed
    if (cevar->callback && strcmp(oldValue, cevar->base->string) != 0) {
        cevar->callback(cevar->base, oldValue);
    }

    return true;
}

// Set from float
bool Cevar_Set(cevar_t* cevar, float value) {
    if (!cevar || !cevar->base) return false;

    const char* oldValue = cevar->base->string;

    // Apply limits if they exist
    if (cevar->limits.has_limits && cevar->limits.is_float) {
        value = std::clamp(value, cevar->limits.f.min, cevar->limits.f.max);
    }

    char buffer[32];
    sprintf(buffer, "%f", value);
    Cvar_Set_og.ccall<cvar_s*>(cevar->base->name, buffer, true);

    // Trigger callback if value changed
    if (cevar->callback && strcmp(oldValue, cevar->base->string) != 0) {
        cevar->callback(cevar->base, oldValue);
    }

    return true;
}

// Set from int
bool Cevar_Set(cevar_t* cevar, int value) {
    if (!cevar || !cevar->base) return false;

    const char* oldValue = cevar->base->string;

    // Apply limits if they exist
    if (cevar->limits.has_limits && !cevar->limits.is_float) {
        value = std::clamp(value, cevar->limits.i.min, cevar->limits.i.max);
    }

    char buffer[32];
    sprintf(buffer, "%d", value);
    Cvar_Set_og.ccall<cvar_s*>(cevar->base->name, buffer, true);

    // Trigger callback if value changed
    if (cevar->callback && strcmp(oldValue, cevar->base->string) != 0) {
        cevar->callback(cevar->base, oldValue);
    }

    return true;
}