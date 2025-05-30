#pragma once

#include "ff.h"
#include "gd_lib.h"
#include "shell.h"
#include "color.h"

// Constants
#define MAX_NAME_LEN        128UL

// Variables
extern FILINFO file_info;
extern DIR dir;
extern FIL common_file;

retv TryOpenFileRead(const char *filename, FIL *pfile);
retv TryOpenFileRewrite(const char *filename, FIL *pfile);
void CloseFile(FIL *pfile);
retv CheckFileNotEmpty(FIL *pfile);
retv TryRead(FIL *pfile, void *ptr, uint32_t sz);
static inline bool FileIsOpen(FIL *pfile) { return (pfile->obj.fs != 0); }

template <typename T>
retv TryRead(FIL *pfile, T *ptr) {
    uint32_t read_sz=0;
    uint8_t r = f_read(pfile, ptr, sizeof(T), &read_sz);
    return (r == FR_OK and read_sz == sizeof(T))? retv::Ok : retv::Fail;
}

retv ReadLine(FIL *pfile, char* S, uint32_t max_len);

bool DirExists(const char* dir_name);
bool DirExistsAndContains(const char* dir_name, const char* extension);
retv CountFilesInDir(const char* dir_name, const char* extension, uint32_t *pcnt);
retv CountDirsStartingWith(const char* path, const char* dir_name_start, uint32_t *pcnt);

#if 1 // ========================= GetRandom from dir ==========================
struct DirRandData {
    char name[MAX_NAME_LEN];
    uint32_t last_n;
    uint32_t file_cnt = 0;
};

#define DIR_CNT   9

class DirList {
private:
    DirRandData dirs[DIR_CNT];
    int32_t dir_cnt = 0;
    int32_t curr_indx = 0;
    retv FindDirInList(const char* dir_name) {
        curr_indx = 0;
        for(int32_t i=0; i<dir_cnt; i++) {
            if(strcmp(dir_name, dirs[i].name) == 0) {
                curr_indx = i;
                return retv::Ok;
            }
        }
        return retv::NotFound;
    }
    void AddDir(const char* dir_name) {
        if(dir_cnt >= DIR_CNT) dir_cnt = 0;
        curr_indx = dir_cnt;
        dir_cnt++;
        strcpy(dirs[curr_indx].name, dir_name);
    }
    void CountFiles(const char* ext) {
        CountFilesInDir(dirs[curr_indx].name, ext, &dirs[curr_indx].file_cnt);
    }
public:
    void Reset() {
        dir_cnt = 0;
        curr_indx = 0;
        dirs[0].file_cnt = 0;
        dirs[0].last_n = 0;
    }

    retv GetRandomFnameFromDir(const char* dir_name, char* afname) {
        // Check if dir in list
        if(FindDirInList(dir_name) != retv::Ok) { // No such dir
    //        Printf("No Dir %S in list\r" , dir_name);
            AddDir(dir_name);
            CountFiles("wav");  // Count files in dir
        }
        uint32_t cnt = dirs[curr_indx].file_cnt;
        if(cnt == 0) return retv::Fail; // Get out if nothing to play
        // Select number of file
        uint32_t n = 0;
        uint32_t last_n = dirs[curr_indx].last_n;
        if(cnt > 1) { // Get random number if count > 1
            do {
                n = Random::Generate(0, cnt-1); // [0; cnt-1]
            } while(n == last_n);   // skip same as previous
        }
        dirs[curr_indx].last_n = n;
    //    Printf("Dir %S: n=%u\r", dir_name, n);
        // Iterate files in dir until success
        cnt = 0;
        uint8_t rslt = f_opendir(&dir, dir_name);
        if(rslt != FR_OK) return retv::Fail;
        while(true) {
            rslt = f_readdir(&dir, &file_info);
            if(rslt != FR_OK) return retv::Fail;
            if((file_info.fname[0] == 0)
#if _USE_LFN
                    and (file_info.altname[0] == 0)
#endif
            ) return retv::Fail;  // somehow no files left
            else { // filename ok, check if not dir
                if(!(file_info.fattrib & AM_DIR)) {
                    // Check if wav or mp3
#if _USE_LFN
                    char *fname = (file_info.fname[0] == 0)? file_info.altname : file_info.fname;
#else
                    char *fname = file_info.fname;
#endif
                    uint32_t len = strlen(fname);
                    if(len > 4) {
                        if(strcasecmp(&fname[len-3], "wav") == 0) {
                            if(n == cnt) {
                                // Build full filename with path
                                // Check if root dir. Empty string allowed, too
                                int len = strlen(dir_name);
                                if((len > 1) or (len == 1 and *dir_name != '/' and *dir_name != '\\')) {
                                    strcpy(afname, dir_name);
                                    afname[len] = '/';
                                }
                                strcpy(&afname[len+1], fname);
                                return retv::Ok;
                            }
                            else cnt++;
                        }
                    } // if len>4
                } // if not dir
            } // filename ok
        } // while true
    }
};
#endif

#define SD_STRING_SZ    256 // for operations with strings
namespace ini { // =================== ini file operations =====================
/*
 * ini file has the following structure:
 *
 * # This is Comment: comment uses either '#' or ';' symbol
 * ; This is Comment too
 *
 * [Section]    ; This is name of section
 * Count=6      ; This is key with value of int32
 * Volume=-1    ; int32
 * SoundFileName=phrase01.wav   ; string
 *
 * [Section2]
 * Key1=1
 * ...
 */

retv ReadString(const char *afile_name, const char *asection, const char *akey, char **ppoutput);

retv ReadStringTo(const char *afile_name, const char *asection, const char *akey, char *poutput, uint32_t max_len);

template <typename T>
static retv Read(const char *afile_name, const char *asection, const char *akey, T *poutput) {
    char *S = nullptr;
    retv r = ReadString(afile_name, asection, akey, &S);
    if(r.IsOk()) {
        int32_t tmp = kl_strtol(S, NULL, 0);
        *poutput = (T)tmp;
        return retv::Ok;
    }
    else return r;
}

retv ReadColor (const char *afile_name, const char *asection, const char *akey, Color_t *poutput);

void WriteSection(FIL *pfile, const char *asection);
void WriteString(FIL *pfile, const char *akey, char *pvalue);
void WriteInt32(FIL *pfile, const char *akey, const int32_t pvalue);
void WriteNewline(FIL *pfile);
void WriteComment(FIL *pfile, const char* S);

} // namespace

namespace csv { // =================== csv file operations =====================
/*
 * csv file has the following structure:
 *
 * # this is comment
 * 14, 0x38, "DirName1"
 * name = "Mr. First"
 * ...
 */

retv OpenFile(const char *afile_name);
void CloseFile();
void RewindFile();
retv ReadNextLine();
retv GetNextCellString(char* poutput);
retv GetNextToken(char** poutput);

// Finds first cell with given name and puts pointer to next cell
retv FindFirstCell(const char* name);

template <typename T>
static retv GetNextCell(T *poutput) {
    char *token;
    if(GetNextToken(&token) == retv::Ok) {
//        Printf("token: %S\r", token);
        char *p;
        *poutput = (T)strtoul(token, &p, 0);
        if(*p == '\0') return retv::Ok;
        else return retv::NotANumber;
    }
    else return retv::Empty;
}

retv GetNextCell(float *poutput);

template <typename T>
static retv TryLoadParam(char* token, const char* name, T *ptr) {
    if(strcasecmp(token, name) == 0) {
        if(csv::GetNextCell<T>(ptr) == retv::Ok) return retv::Ok;
        else Printf("%S load fail\r", name);
    }
    return retv::Fail;
}

retv TryLoadParam(char* token, const char* name, float *ptr);

retv TryLoadString(char* token, const char* name, char *Dst, uint32_t max_len);

} // namespace
