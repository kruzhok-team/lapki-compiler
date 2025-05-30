#include "Settings.h"
#include "gd_lib.h"
#include "shell.h"
#include "kl_fs_utils.h"

Settings settings;

void Settings::Load() {
    for(ValueBase* const pval : values_arr) {
        int32_t v;
        retv r = ini::Read(kSettingsFilename, pval->section, pval->name, &v);
        if(r == retv::NotFound) {
            SetAllToDefault();
            return;
        }
        else if(r.NotOk() or pval->CheckAndSetIfOk(v).NotOk()) {
            pval->SetToDefault();
            Printf("Bad %S %S. Default value set\r\n", pval->section, pval->name);
        }
    }
    Printf("Settings loaded\r\n");
}

retv Settings::Save() {
    const char* curr_grp = nullptr;
    if(TryOpenFileRewrite(kSettingsFilename, &common_file) != retv::Ok) return retv::Fail;
    // Description
    f_printf(&common_file, "# IR PCB v3.2 Settings.ini\r\n\r\n");
    // Values
    for(ValueBase* const pval : values_arr) {
        if(curr_grp != pval->section) {
            curr_grp = pval->section;
            f_printf(&common_file, "[%S]\r\n", curr_grp);
        }
        pval->Save(&common_file);
    }
    CloseFile(&common_file);
    return retv::Ok;
}
