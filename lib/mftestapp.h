// Plugin API of the Manifold test app.
//
// A plugin is a shared object that exports mftest_plugin_info(). The app
// loads every *.so it finds below its plugin directories and shows what
// each plugin's report() returns.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Bumped on every incompatible change; the loader rejects other versions
#define MFTEST_PLUGIN_API_VERSION 1

typedef struct MfTestPlugin {
    int api_version; // MFTEST_PLUGIN_API_VERSION
    const char *name;
    const char *description;
    // Text for the app to show, malloc()ed; the app free()s it. May be NULL.
    char *(*report)(void);
} MfTestPlugin;

// Implemented by each plugin, not by the library
const MfTestPlugin *mftest_plugin_info(void);

// MFTEST_PLUGIN_API_VERSION of the library loaded at runtime
int mftest_api_version(void);

// File the library was loaded from
const char *mftest_library_path(void);

// Loads the plugin at path. Returns NULL and sets *error (malloc()ed, may
// be NULL) if it can't be loaded, isn't a plugin or has another API
// version. A loaded plugin stays loaded until the process exits.
const MfTestPlugin *mftest_plugin_load(const char *path, char **error);

#ifdef __cplusplus
}
#endif
