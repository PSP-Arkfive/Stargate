/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>
#include <pspsdk.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>
#include <module2.h>
#include <systemctrl.h>

static char *g_blacklist[] = {
    "iso",
    "seplugins",
    "isocache.bin",
    "irshell",
};

static inline int is_in_blacklist(const char *dname)
{
    // lower string
    char temp[255]; memset(temp, 0, sizeof(temp));
    strncpy(temp, dname, sizeof(temp));

    extern int lowerString(char*, char*, int);
    lowerString(temp, temp, strlen(temp)+1);

    for (int i=0; i<NELEMS(g_blacklist); ++i) {
        if (strstr(temp, g_blacklist[i])) {
        	return 1;
        }
    }

    return 0;
}

int hideIoDread(SceUID fd, SceIoDirent * dir)
{
    int k1 = pspSdkSetK1(0);
    int result = sceIoDread(fd, dir);

    if(result > 0 && is_in_blacklist(dir->d_name)) {
        result = sceIoDread(fd, dir);
    }

    pspSdkSetK1(k1);
    return result;
}

// hide cfw folders, this avoids crashing the weird dj max portable 3 savegame algorithm
void hide_cfw_folder(SceModule2 * mod)
{
    // hide dread
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, &hideIoDread);
}
