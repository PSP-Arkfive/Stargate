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
#include <stdio.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_private.h>

#include "loadmodule_patch.h"
#include "nodrm_patch.h"

PSP_MODULE_INFO("Stargate", 0x1007, 1, 5);
PSP_MAIN_THREAD_ATTR(0);

// Previous Module Start Handler
STMOD_HANDLER previous;

extern void patch_ioDevCtl();
extern void patch_IsoDrivers();
extern void patch_sceMesgLed();
extern void applyFixesByGameId();
extern void applyFixesByModule(SceModule* mod);
extern void hide_cfw_folder(SceModule * mod);

// Module Start Handler
int stargateSyspatchModuleOnStart(SceModule * mod)
{
    static int booted = 0;

    if (booted){
        // Patch CFW dirs
        hide_cfw_folder(mod);
        // Patch game-specific modules
        applyFixesByModule(mod);
    }

    // Boot Complete Action not done yet
    if (strcmp(mod->modname, "sceKernelLibrary") == 0)
    {
        booted = 1;
        applyFixesByGameId();
    }

    // Call Previous Module Start Handler
    if (previous) previous(mod);
    
    // Patch LoadModule Function
    patchLoadModuleFuncs(mod);

    return 0;
    
}

// Boot Time Module Start Handler
int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;
int stargateStartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt)
{
    // Fetch Module
    SceModule * mod = (SceModule *)sceKernelFindModuleByUID(modid);
    
    // Module not found
    if(mod == NULL) return -1;
    
    // Fix Prometheus Patch #1
    SceLibraryStubTable * import = sctrlFindImportLib(mod, "Kernel_LibrarZ");
    if(import != NULL)
    {
        strcpy((char *)import->libname, "Kernel_Library");
    }
    
    // Fix Prometheus Patch #2
    import = sctrlFindImportLib(mod, "Kernel_Librar0");
    if(import != NULL)
    {
        strcpy((char* )import->libname, "Kernel_Library");
    }
    
    // Fix Prometheus Patch #3
    import = sctrlFindImportLib(mod, "sceUtilitO");
    if(import != NULL)
    {
        strcpy((char*)import->libname, "sceUtility");
    }
    
    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}

// Idol Master Fix
static void patchLoadExec(void)
{
    // Fix Load Execute CFW Detection
    SceModule* mod = (SceModule*)sceKernelFindModuleByName("sceLoadExec");
    for (u32 addr=mod->text_addr; ;addr+=4){
        if (_lw(addr) == 0x00250821){
            _sw(NOP, addr+4);
            break;
        }
    }
}

// Entry Point
int module_start(SceSize args, void * argp)
{

    #ifdef DEBUG
    // Hello Message
    printk("stargate started: compiled at %s %s\r\n", __DATE__, __TIME__);
    #endif

    int apitype = sceKernelInitApitype();

    if (apitype == 0x141 || apitype == 0x152)
        return 0; // don't do anything in homebrew

    patch_sceMesgLed();

    // Fix Idol Master
    patchLoadExec();

    // Fix non-Latin1 characters in ISO name
    patch_IsoDrivers();
    
    // Fetch sceUtility Load Module Functions
    getLoadModuleFuncs();
    
    // Initialize NPDRM
    nodrmInit();

    // Patch free mem size
    patch_ioDevCtl();
    
    // Register Start Module Handler
    previous = sctrlHENSetStartModuleHandler(stargateSyspatchModuleOnStart);
    
    // Register Boot Start Module Handler
    prev_start = sctrlSetStartModuleExtra(stargateStartModuleHandler);
    
    // Flush Cache
    sctrlFlushCache();
    
    // Module Start Success
    return 0;
}
