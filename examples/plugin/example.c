// Example plugin for the Manifold test app. It reports where it found
// libmftestapp, to show that a plugin resolves the app's library.

#define _GNU_SOURCE
#include <mftestapp.h>

#include <stdio.h>

static char *report(void)
{
    char *text = NULL;
    if (asprintf(&text, "libmftestapp API %d, loaded from %s",
                 mftest_api_version(), mftest_library_path()) < 0)
        return NULL;
    return text;
}

static const MfTestPlugin plugin = {
    .api_version = MFTEST_PLUGIN_API_VERSION,
    .name = "Example",
    .description = "Reports where it found libmftestapp",
    .report = report,
};

const MfTestPlugin *mftest_plugin_info(void)
{
    return &plugin;
}
