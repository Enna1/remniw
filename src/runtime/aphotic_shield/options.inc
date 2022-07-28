#ifndef APHOTIC_SHIELD_OPTION
    #error "Define APHOTIC_SHIELD_OPTION prior to including this file!"
#endif

APHOTIC_SHIELD_OPTION(bool, Enabled, true, "Is APHOTIC-SHIELD enabled? Defaults to true.")

APHOTIC_SHIELD_OPTION(int, MaxSimultaneousAllocations, 4096,
                      "Number of simultaneously-guarded allocations available in the "
                      "pool. Defaults to 4096.")

APHOTIC_SHIELD_OPTION(bool, help, false, "Print a summary of the available options.")
