/* const: redefine constants
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_IO_CONST
#define HEADER_INCLUDED_IO_CONST 1

#include "../misc/ext-c-begin.h"

#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_IO_CONST */
