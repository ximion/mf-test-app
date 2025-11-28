#define _GNU_SOURCE
#include "mftestapp.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>

int mftest_api_version(void)
{
    return MFTEST_PLUGIN_API_VERSION;
}

const char *mftest_library_path(void)
{
    Dl_info info;
    if (dladdr((void *) mftest_library_path, &info) == 0 || info.dli_fname == NULL)
        return "(unknown)";
    return info.dli_fname;
}

static void set_error(char **error, const char *format, ...)
{
    if (error == NULL)
        return;
    va_list args;
    va_start(args, format);
    if (vasprintf(error, format, args) < 0)
        *error = NULL;
    va_end(args);
}

const MfTestPlugin *mftest_plugin_load(const char *path, char **error)
{
    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        set_error(error, "%s", dlerror());
        return NULL;
    }

    const MfTestPlugin *(*info)(void) = (const MfTestPlugin *(*)(void)) dlsym(handle, "mftest_plugin_info");
    if (info == NULL) {
        set_error(error, "not a plugin: no mftest_plugin_info()");
        dlclose(handle);
        return NULL;
    }

    const MfTestPlugin *plugin = info();
    if (plugin == NULL || plugin->api_version != MFTEST_PLUGIN_API_VERSION) {
        set_error(error, "plugin API version %d, expected %d",
                  plugin != NULL ? plugin->api_version : -1, MFTEST_PLUGIN_API_VERSION);
        dlclose(handle);
        return NULL;
    }
    return plugin;
}
