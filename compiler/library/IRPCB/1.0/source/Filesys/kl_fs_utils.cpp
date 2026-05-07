/*
 * kl_fs_common.cpp
 *
 *  Created on: 2016
 *      Author: Kreyl
 */

#include "kl_fs_utils.h"
#include "shell.h"

// Variables
FILINFO file_info;
DIR dir;
FIL common_file;
static char istr[SD_STRING_SZ];

#if 1 // ============================== Common =================================
retv TryOpenFileRead(const char *filename, FIL *pfile) {
//    Printf("%S: %S; %X\r", __FUNCTION__, filename, pfile);
    FRESULT rslt = f_open(pfile, filename, FA_READ);
    if(rslt == FR_OK) {
        // Check if zero file
        if(f_size(pfile) == 0) {
            f_close(pfile);
            Printf("Empty file %S\r", filename);
            return retv::Empty;
        }
        return retv::Ok;
    }
    else {
        if (rslt == FR_NO_FILE) Printf("%S: not found\r", filename);
        else Printf("OpenFile error: %u\r", rslt);
        return retv::Fail;
    }
}

retv TryOpenFileRewrite(const char *filename, FIL *pfile) {
    FRESULT rslt = f_open(pfile, filename, FA_WRITE+FA_CREATE_ALWAYS);
    if(rslt == FR_OK) return retv::Ok;
    else {
        Printf("%S open error: %u\r", filename, rslt);
        return retv::Fail;
    }
}

void CloseFile(FIL *pfile) {
    f_close(pfile);
}

retv CheckFileNotEmpty(FIL *pfile) {
    if(f_size(pfile) == 0) {
        Printf("Empty file\r");
        return retv::Empty;
    }
    else return retv::Ok;
}

retv TryRead(FIL *pfile, void *ptr, uint32_t sz) {
    uint32_t read_sz=0;
    uint8_t r = f_read(pfile, ptr, sz, &read_sz);
    return (r == FR_OK and read_sz == sz)? retv::Ok : retv::Fail;
}

retv ReadLine(FIL *pfile, char* S, uint32_t max_len) {
    uint32_t len = 0, rcvd_len;
    char c, str[2];
    while(len < max_len-1) {
        if(f_read(pfile, str, 1, &rcvd_len) != FR_OK) return retv::Fail;
        if(rcvd_len != 1) return retv::EndOfFile;
        c = str[0];
        if(c == '\r' or c == '\n') {    // End of line
            *S = '\0';
            return retv::Ok;
        }
        else {
            *S++ = c;
            len++;
        }
    } // while
    *S = '\0';
    return retv::Ok;
}

bool DirExists(const char* dir_name) {
    FRESULT rslt = f_opendir(&dir, dir_name);
    return (rslt == FR_OK);
}

bool DirExistsAndContains(const char* dir_name, const char* extension) {
    if(f_opendir(&dir, dir_name) == FR_OK) {
        while(true) {
            // Empty names before reading
            *file_info.fname = 0;
#if _USE_LFN
            *file_info.altname = 0;
#endif
            if(f_readdir(&dir, &file_info) != FR_OK) return false;
            if((file_info.fname[0] == 0)
#if _USE_LFN
                    and (file_info.altname[0] == 0)
#endif
            ) return false;   // No files left
            else { // filename ok, check if not dir
                if(!(file_info.fattrib & AM_DIR)) {
                    // Check the ext
#if _USE_LFN
                    char *fname = (file_info.fname[0] == 0)? file_info.altname : file_info.fname;
#else
                    char *fname = file_info.fname;
#endif
                    uint32_t len = strlen(fname);
                    if(len > 4) {
                        if(strncasecmp(&fname[len-3], extension, 3) == 0) return true;
                    } // if len>4
                } // if not dir
            } // filename ok
        } // while
    }
    else return false;
}

retv CountFilesInDir(const char* dir_name, const char* extension, uint32_t *pcnt) {
    *pcnt = 0;
    FRESULT rslt = f_opendir(&dir, dir_name);
//    Printf("f_opendir %S: %u\r", dir_name, rslt);
    if(rslt != FR_OK) return retv::Fail;
    while(true) {
        // Empty names before reading
        *file_info.fname = 0;
#if _USE_LFN
        *file_info.altname = 0;
#endif

        rslt = f_readdir(&dir, &file_info);
        if(rslt != FR_OK) return retv::Fail;
        if((file_info.fname[0] == 0)
#if _USE_LFN
                    and (file_info.altname[0] == 0)
#endif
        ) return retv::Ok;   // No files left
        else { // filename ok, check if not dir
            if(!(file_info.fattrib & AM_DIR)) {
                // Check Ext
#if _USE_LFN
                char *fname = (file_info.fname[0] == 0)? file_info.altname : file_info.fname;
#else
                char *fname = file_info.fname;
#endif
//                Printf("%S\r", fname);
                uint32_t len = strlen(fname);
                if(len > 4) {
                    if(strncasecmp(&fname[len-3], extension, 3) == 0) (*pcnt)++;
                } // if len>4
            } // if not dir
        } // filename ok
    }
    return retv::Ok;
}

retv CountDirsStartingWith(const char* path, const char* dir_name_start, uint32_t *pcnt) {
    FRESULT rslt = f_opendir(&dir, path);
    if(rslt != FR_OK) return retv::Fail;
    *pcnt = 0;
    int len = strlen(dir_name_start);
    while(true) {
        // Empty names before reading
        *file_info.fname = 0;
#if _USE_LFN
        *file_info.altname = 0;
#endif
        rslt = f_readdir(&dir, &file_info);
        if(rslt != FR_OK) return retv::Fail;
        if((file_info.fname[0] == 0)
#if _USE_LFN
                    and (file_info.altname[0] == 0)
#endif
        ) return retv::Ok;   // Nothing left
        else { // filename ok, check if dir
            if(file_info.fattrib & AM_DIR) {
                // Check if starts with dir_name_start
#if _USE_LFN
                char *fname = (file_info.fname[0] == 0)? file_info.altname : file_info.fname;
#else
                char *fname = file_info.fname;
#endif
//                Printf("%S\r", fname);
                if(strncasecmp(fname, dir_name_start, len) == 0) (*pcnt)++;
            } // if dir
        } // filename ok
    }
    return retv::Ok;
}
#endif

namespace ini { // =================== ini file operations =====================
void WriteSection(FIL *pfile, const char *psection) {
    f_printf(pfile, "[%S]\r\n", psection);
}
void WriteString(FIL *pfile, const char *akey, char *pvalue) {
    f_printf(pfile, "%S=%S\r\n", akey, pvalue);
}
void WriteInt32(FIL *pfile, const char *akey, const int32_t pvalue) {
    f_printf(pfile, "%S=%D\r\n", akey, pvalue);
}
void WriteNewline(FIL *pfile) {
    f_putc('\r', pfile);
    f_putc('\n', pfile);
}
void WriteComment(FIL *pfile, const char* S) {
    f_printf(pfile, "# %S\r\n", S);
}


static inline char* skipleading(char *S) {
    while (*S != '\0' && *S <= ' ') S++;
    return S;
}
static inline char* skiptrailing(char *S, const char *base) {
    while ((S > base) && (*(S-1) <= ' ')) S--;
    return S;
}
static inline char* striptrailing(char *S) {
    char *ptr = skiptrailing(strchr(S, '\0'), S);
    *ptr='\0';
    return S;
}

retv ReadString(const char *afilename, const char *psection, const char *akey, char **ppoutput) {
    FRESULT rslt;
//    Printf("%S %S %S %S\r", __FUNCTION__, afilename, psection, akey);
    // Open file
    rslt = f_open(&common_file, afilename, FA_READ+FA_OPEN_EXISTING);
    if(rslt != FR_OK) {
        if (rslt == FR_NO_FILE) {
            Printf("%S: not found\r", afilename);
            return retv::NotFound;
        }
        else Printf("%S: openFile error: %u\r", afilename, rslt);
        return retv::Fail;
    }
    // Check if zero file
    if(f_size(&common_file) == 0) {
        f_close(&common_file);
        Printf("Empty file\r");
        return retv::Empty;
    }
    // Move through file one line at a time until a section is matched or EOF.
    char *pstart, *pend = nullptr;
    int32_t len = strlen(psection);
    do {
        if(f_gets(istr, SD_STRING_SZ, &common_file) == nullptr) {
            Printf("iniNoSection %S\r", psection);
            f_close(&common_file);
            return retv::Fail;
        }
        pstart = skipleading(istr);
        if((*pstart != '[') or (*pstart == ';') or (*pstart == '#')) continue;
        pend = strchr(pstart, ']');
        if((pend == NULL) or ((int32_t)(pend-pstart-1) != len)) continue;
    } while (strncmp(pstart+1, psection, len) != 0);

    // Section found, find the key
    len = strlen(akey);
    do {
        if(!f_gets(istr, SD_STRING_SZ, &common_file) or *(pstart = skipleading(istr)) == '[') {
            Printf("iniNoKey %S\r", akey);
            f_close(&common_file);
            return retv::Fail;
        }
        pstart = skipleading(istr);
        if((*pstart == ';') or (*pstart == '#')) continue;
        pend = strchr(pstart, '=');
        if(pend == NULL) continue;
    } while(((int32_t)(skiptrailing(pend, pstart)-pstart) != len or strncmp(pstart, akey, len) != 0));
    f_close(&common_file);

    // Process Key's value
    pstart = skipleading(pend + 1);
    // Remove a trailing comment
    uint8_t is_string = 0;
    for(pend = pstart; (*pend != '\0') and (((*pend != ';') and (*pend != '#')) or is_string) and ((uint32_t)(pend - pstart) < SD_STRING_SZ); pend++) {
        if (*pend == '"') {
            if (*(pend + 1) == '"') pend++;     // skip "" (both quotes)
            else is_string = !is_string; // single quote, toggle is_string
        }
        else if (*pend == '\\' && *(pend + 1) == '"') pend++; // skip \" (both quotes)
    } // for
    *pend = '\0';   // Terminate at a comment
    striptrailing(pstart);
    *ppoutput = pstart;
    return retv::Ok;
}

retv ReadStringTo(const char *afilename, const char *psection, const char *akey, char *poutput, uint32_t max_len) {
    char *S;
    if(ReadString(afilename, psection, akey, &S) == retv::Ok) {
        // Copy what was read
        if(strlen(S) > (max_len-1)) {
            strncpy(poutput, S, (max_len-1));
            poutput[max_len-1] = 0;  // terminate string
        }
        else strcpy(poutput, S);
        return retv::Ok;
    }
    else return retv::Fail;
}

retv HexToUint(char *S, uint8_t amax_len, uint32_t *poutput) {
    *poutput = 0;
    char c;
    uint8_t b=0;
    for(uint8_t i=0; i<amax_len; i++) {
        c = *S++;
        if (c == 0) return retv::Ok;    // end of string
        // Shift result
        *poutput <<= 4;
        // Get next digit
        if     ((c >= '0') && (c <= '9')) b = c-'0';
        else if((c >= 'A') && (c <= 'F')) b = c-'A'+10;
        else if((c >= 'a') && (c <= 'f')) b = c-'a'+10;
        else return retv::Fail;  // not a hex digit
        *poutput += b;
    }
    return retv::Ok;
}

retv ReadColor (const char *afilename, const char *psection, const char *akey, Color_t *poutput) {
    char *S;
    if(ReadString(afilename, psection, akey, &S) == retv::Ok) {
        if(strlen(S) != 6) return retv::BadValue;
        uint32_t N=0;
        if(HexToUint(&S[0], 2, &N) != retv::Ok) return retv::Fail;
        poutput->R = N;
        if(HexToUint(&S[2], 2, &N) != retv::Ok) return retv::Fail;
        poutput->G = N;
        if(HexToUint(&S[4], 2, &N) != retv::Ok) return retv::Fail;
        poutput->B = N;
        return retv::Ok;
    }
    else return retv::Fail;
}

} // Namespace

namespace csv { // =================== csv file operations =====================
#define CSV_DELIMITERS  " ,;={}\t\r\n"
__unused static char *csv_curr_token;

retv OpenFile(const char *afilename) {
    return TryOpenFileRead(afilename, &common_file);
}
void RewindFile() {
    f_lseek(&common_file, 0);
}
void CloseFile() {
    f_close(&common_file);
}

/* Skips empty and comment lines (starting with #).
 * Returns retv::Ok if not-a-comment line read, retvEndOfFile if eof
 */
retv ReadNextLine() {
    // Move through file until comments end
    while(true) {
        if(f_eof(&common_file) or f_gets(istr, SD_STRING_SZ, &common_file) == nullptr) {
//            Printf("csvNoMoreData\r");
            return retv::EndOfFile;
        }
        csv_curr_token = strtok(istr, CSV_DELIMITERS);
        if(*csv_curr_token == '#' or *csv_curr_token == 0) continue; // Skip comments and empty lines
        else return retv::Ok;
    }
}

retv GetNextToken(char** poutput) {
    // First time, return csv_curr_token, as it was set in ReadNextLine
    if(csv_curr_token != nullptr) {
        *poutput = csv_curr_token;
        csv_curr_token = nullptr;
    }
    else *poutput = strtok(NULL, CSV_DELIMITERS);
    return (**poutput == '\0')? retv::Empty : retv::Ok;
}

retv GetNextCellString(char* poutput) {
    char *token;
    if(GetNextToken(&token) == retv::Ok) {
        // Skip leading quotes
        char *pstart = token;
        while(*pstart == '"' and *pstart != '\0') pstart++;
        char *pend = token + strlen(token) - 1;
        while(*pend == '"' and pend > token) pend--;
        int32_t len = pend - pstart + 1;
        if(len > 0) strncpy(poutput, pstart, len);
        else *poutput = '\0';
        return retv::Ok;
    }
    else return retv::Fail;
}

retv FindFirstCell(const char* name) {
    while(true) {
        if(ReadNextLine() != retv::Ok) break;
//        Printf("token: %S\r", csv_curr_token);
        if(strcasecmp(csv_curr_token, name) == 0) {
            csv_curr_token = strtok(NULL, CSV_DELIMITERS);
            return retv::Ok;
        }
    }
    return retv::NotFound;
}

retv GetNextCell(float *poutput) {
    char *token;
    if(GetNextToken(&token) == retv::Ok) {
        char *p;
        *poutput = strtof(token, &p);
        if(*p == '\0') return retv::Ok;
        else return retv::NotANumber;
    }
    else return retv::Empty;
}

retv TryLoadParam(char* token, const char* name, float *ptr) {
    if(strcasecmp(token, name) == 0) {
        if(csv::GetNextCell(ptr) == retv::Ok){
//            Printf("  %S: %f\r", name, *ptr);
            return retv::Ok;
        }
        else Printf("%S load fail\r", name);
    }
    return retv::Fail;
}

retv TryLoadString(char* token, const char* name, char *Dst, uint32_t max_len) {
    if(strcasecmp(token, name) == 0) {
        *Dst = 0;   // Empty Dst
        char *Cell;
        if(GetNextToken(&Cell) == retv::Ok) {
            uint32_t len = strlen(Cell);
            if(len < max_len) strcpy(Dst, Cell);
            else {
                strncpy(Dst, Cell, max_len-1);
                Dst[max_len-1] = 0;
            }
        }
        return retv::Ok;
    }
    return retv::Fail;
}

} // namespace
