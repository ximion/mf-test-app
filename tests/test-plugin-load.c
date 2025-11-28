// Loads the example plugin (argv[1]) and checks that something that is no
// plugin (argv[2]) is rejected.

#include <mftestapp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;

    char *error = NULL;
    const MfTestPlugin *plugin = mftest_plugin_load(argv[1], &error);
    if (plugin == NULL) {
        fprintf(stderr, "loading %s failed: %s\n", argv[1], error);
        return 1;
    }
    char *text = plugin->report();
    printf("%s: %s\n", plugin->name, text);
    if (strcmp(plugin->name, "Example") != 0 || text == NULL || strstr(text, "libmftestapp") == NULL)
        return 1;
    free(text);

    if (mftest_plugin_load(argv[2], &error) != NULL) {
        fprintf(stderr, "%s was accepted as a plugin\n", argv[2]);
        return 1;
    }
    printf("rejected %s: %s\n", argv[2], error);
    free(error);
    return 0;
}
