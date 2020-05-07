#include "../core/sync.hpp"
#include "../core/utils.hpp"

extern int test_client_lib(int x) { return 1 * x * test_utils_lib(x) * test_sync_lib(x); }
