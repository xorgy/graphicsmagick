//
//  Little cms
//  Copyright (C) 1998-2003 Marti Maria
//
// Permission is hereby granted, free of charge, to any person obtaining 
// a copy of this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software 
// is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


// Named color support

#include "lcms.h"


LPcmsNAMEDCOLORLIST  cdecl cmsAllocNamedColorList(void);
void                 cdecl cmsFreeNamedColorList(LPcmsNAMEDCOLORLIST List);
BOOL                 cdecl cmsAppendNamedColor(cmsHTRANSFORM xform, const char* Name, WORD PCS[3], WORD Colorant[MAXCHANNELS]);


// ---------------------------------------------------------------------------------

static
BOOL GrowNamedColorList(LPcmsNAMEDCOLORLIST v, int ByElements)
{
    LPcmsNAMEDCOLOR NewList;
    int NewElements;
    
    if (ByElements > v ->Allocated) {

        if (v ->Allocated == 0)
            NewElements = 64;   // Initial guess
        else
            NewElements = v ->Allocated;

        while (ByElements > NewElements)
                NewElements *= 2;
        
        NewList = (LPcmsNAMEDCOLOR) malloc(sizeof(cmsNAMEDCOLOR) * NewElements);

        if (NewList == NULL) {
            cmsSignalError(LCMS_ERRC_ABORTED, "Out of memory reallocating named color list");
            return FALSE;
        }
        else {

            if (v -> List) {
                CopyMemory(NewList, v -> List, v ->nColors* sizeof(cmsNAMEDCOLOR));
                free(v -> List);                
            }

            v -> List = NewList;
            v -> Allocated = NewElements;
            return TRUE;
        }
    }

    return TRUE;
}


LPcmsNAMEDCOLORLIST cmsAllocNamedColorList(void)
{
    LPcmsNAMEDCOLORLIST v = (LPcmsNAMEDCOLORLIST) malloc(sizeof(cmsNAMEDCOLORLIST));

    if (v == NULL) {
        cmsSignalError(LCMS_ERRC_ABORTED, "Out of memory creating named color list");
        return NULL;
    }

    ZeroMemory(v, sizeof(cmsNAMEDCOLORLIST));

    v ->nColors   = 0;
    v ->Allocated = 0;  
    v ->Prefix[0] = 0;
    v ->Suffix[0] = 0;  
    v -> List     = NULL; 
    
    
    return v;
}

void cmsFreeNamedColorList(LPcmsNAMEDCOLORLIST v)
{
    if (v == NULL) {
        cmsSignalError(LCMS_ERRC_RECOVERABLE, "Couldn't free a NULL named color list");
        return;
    }
                
    if (v -> List) free(v->List);
    free(v);
}   




BOOL cmsAppendNamedColor(cmsHTRANSFORM xform, const char* Name, WORD PCS[3], WORD Colorant[MAXCHANNELS])
{
    _LPcmsTRANSFORM v = (_LPcmsTRANSFORM) xform;
    LPcmsNAMEDCOLORLIST List = v ->NamedColorList;
    int i;

    if (List == NULL) return FALSE;
    if (!GrowNamedColorList(List, List ->nColors + 1)) return FALSE;
    
    for (i=0; i < MAXCHANNELS; i++)
        List ->List[List ->nColors].DeviceColorant[i] = Colorant[i];

    for (i=0; i < 3; i++)
        List ->List[List ->nColors].PCS[i] = PCS[i];

    strncpy(List ->List[List ->nColors].Name, Name, MAX_PATH-1);

    List ->nColors++;
    return TRUE;
}


// Returns named color count 

int LCMSEXPORT cmsNamedColorCount(cmsHTRANSFORM xform)
{
     _LPcmsTRANSFORM v = (_LPcmsTRANSFORM) xform;

     if (v ->NamedColorList == NULL) return 0;
     return v ->NamedColorList ->nColors;
}


BOOL LCMSEXPORT cmsNamedColorInfo(cmsHTRANSFORM xform, int nColor, char* Name, char* Prefix, char* Suffix)
{
    _LPcmsTRANSFORM v = (_LPcmsTRANSFORM) xform;


     if (v ->NamedColorList == NULL) return FALSE;

     if (nColor < 0 || nColor >= cmsNamedColorCount(xform)) return FALSE;

     if (Name) strncpy(Name, v ->NamedColorList->List[nColor].Name, 31);
     if (Prefix) strncpy(Name, v ->NamedColorList->Prefix, 31);
     if (Suffix) strncpy(Name, v ->NamedColorList->Suffix, 31);

     return TRUE;
}


int  LCMSEXPORT cmsNamedColorIndex(cmsHTRANSFORM xform, const char* Name)
{
    _LPcmsTRANSFORM v = (_LPcmsTRANSFORM) xform;    
    int i, n;

         if (v ->NamedColorList == NULL) return -1;

        n = cmsNamedColorCount(xform);
        for (i=0; i < n; i++) {
            if (stricmp(Name,  v ->NamedColorList->List[i].Name) == 0)
                    return i;
        }

        return -1;
}
