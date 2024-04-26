/*
* Sniffer is prrogramm that can collect data about incoming udp packages
* Copyright (C) 2024  Vladimir Mimikin
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef HLPR
#define HLPR

#define ERROR_EXIT(message) do { \
    printf(message); \
    exit(EXIT_FAILURE); \
} while(0)

#define ERROR_RETURN(message) do { \
    perror(message); \
    return 0; \
} while(0)

#define THREAD_ERROR_RETURN(message) do { \
    perror(message); \
    return (void*) 0; \
} while(0)

#define WRONG_OPT_RETURN(message) do { \
    printf(message); \
    return 0; \
} while(0)

#define NOTVALIDSOCKET(s) ((s) < 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int 
#define MAX_ADDR_SZ 16

#endif