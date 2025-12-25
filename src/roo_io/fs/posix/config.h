#pragma once

#ifndef ROO_IO_FS_SUPPORT_POSIX

#if (defined ESP_PLATFORM || defined ROO_TESTING || defined __linux__)
#define ROO_IO_FS_SUPPORT_POSIX 1
#else
#define ROO_IO_FS_SUPPORT_POSIX 0 
#endif

#endif