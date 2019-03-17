#include <string.h>
#include <kernel/memory/paging.h>

int test_strsplit(const char *str,
                  char delim,
                  const char *expected[],
                  unsigned int count)
{
    struct strarray *splitted = strsplit(str, delim);

    if (splitted->size != count) {
        return 0;
    }

    for (unsigned int i = 0; i < splitted->size; i++) {
        if (strcmp(expected[i], splitted->array[i]) > 0) {
            return 0;
        }
    }

    return 1;
}

int test_string()
{
    const char *strs[] = {
        "/toto/foo/bar",
        "bonjour",
        "//abcde",
        "/////toto//abc///foo//////bar///////"
    };

    const char *expected[10][30] = {
        { "toto", "foo", "bar" },
        { "bonjour" },
        { "abcde" },
        { "toto", "abc", "foo", "bar" }
    };

    const int count[] = {
        3,
        1,
        1,
        4
    };

    for (int i = 0; i < sizeof(count) / sizeof(int); i++) {
        if (!test_strsplit(strs[i], '/', expected[i], count[i])) {
            printf("[TEST] test_strsplit %d failed\n", i);
        }
    }

    return 1;
}
