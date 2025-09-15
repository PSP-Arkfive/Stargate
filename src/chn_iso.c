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

#include <stdlib.h>
#include <string.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>

#include <ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_private.h>

int myIoOpen_kernel_chn(char *file, int flag, int mode)
{

    u32 (*GetCompiledSdkVersion)() = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0xFC114573);
    void (*SetCompiledSdkVersion)(u32) = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x7591C7DB);

    u32 sdk_ver = GetCompiledSdkVersion();
    SetCompiledSdkVersion(FW_660);

    int ret = sceIoOpen(file, flag, mode);

    SetCompiledSdkVersion(sdk_ver);

    return ret;
}

void patch_IsoDrivers(void)
{
    SceModule *mod = (SceModule*) sceKernelFindModuleByName("PRO_Inferno_Driver");
    if(mod != NULL) {
        sctrlHookImportByNID(mod, "IoFileMgrForKernel", 0x109F50BC, &myIoOpen_kernel_chn);
    }
}
