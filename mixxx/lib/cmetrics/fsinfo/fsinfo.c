/**********************************************
 * Cmetrics.h - Case Metrics Interface
 *  Copyright 2007 John Sully, Phillip Mendonça-Vieira.
 *
 *  This file is part of Case Metrics.
 *
 *  Case Metrics is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  Case Metrics is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Case Metrics.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************/

//TODO: Update for win NT.  This will fail for non DOS namespace volumes

#ifdef LINUX
#include <sys/statfs.h>
#include <stdio.h>
#endif

#include "../globaldefs.h"
#include "../utf/utstr.h"
#include "fsinfo.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef LINUX
#include "paths.h"
#elif WIN32
#include <windows.h>
#endif

struct stzlist 
{
    struct stzlist *next;
    char *pstzPath;
    XCHAR *pstzStatfs;
} typedef _LIST;


// returns the size_t of the written XCHAR string successful (byte count), 
//-1 if unsuccesful and leaves errno to whatever the syscallstatfs 
// last set it to.
// this function allocates space and writes the resulting string to ppstz 
int getStatfs(char *pstzPath, XCHAR **ppstz)
{   
    
#ifdef LINUX
    struct statfs info;
    int strLen = 0;
    int strSize = (strlen(pstzPath) + (ULONG_HEX_SIZE * 4) + 4) 
        * sizeof(XCHAR); // 4 as a magic #: useful fsinfo attributes
    if ((statfs(pstzPath, &info)) == 0)
    {
        if ((*ppstz = malloc(strSize + 1)) == NULL) return -1;

        strLen = xsprintf(*ppstz, strSize, "%s|%lX|%lX|%lX|%lX", pstzPath, 
                info.f_type, info.f_blocks, info.f_bfree, info.f_bsize);
        return strLen; // I hate having to do unecessary strlens
        
    }
    else
        goto ERROR_HANDLER;
    //printf("get %d\n", ret);
#elif WIN32
	//Working Vars
	int bufSize = 0;
	int strLen = 0;
	//Volume Data
	char pvolumeName[MAX_PATH + 1];
	int volumeSerial;
	int maxComponentLength;
	int fsFlags;
	int driveType;
	ULARGE_INTEGER uliUserFreeBytes, uliTotalFreeBytes, uliTotalBytes;
	unsigned int64 userFreeBytes;
	unsigned int64 totalFreeBytes;
	unsigned int64 totalBytes;
	char pfsName[MAX_PATH + 1];

	if(! GetVolumeInformation(pstzPath,
					pvolumeName,
					(MAX_PATH + 1),
					&volumeSerial,
					&maxComponentLength,
					&fsFlags,
					pfsName,
					(MAX_PATH + 1)))
	{
#ifdef TEST
		printf("Error: %d\n", GetLastError());
#endif
		goto ERROR_HANDLER;
	}
	if(! GetDiskFreeSpaceEx(pstzPath,
						&uliUserFreeBytes,
						&uliTotalBytes,
						&uliTotalFreeBytes))
	{
#ifdef TEST
		printf("Error: %d\n", GetLastError());
#endif
		goto ERROR_HANDLER;
	}
	driveType = GetDriveType(pstzPath);

	userFreeBytes = uliUserFreeBytes.QuadPart;
	totalFreeBytes = uliTotalFreeBytes.QuadPart;
	totalBytes = uliTotalBytes.QuadPart;

	//Compute Buffer Length
	bufSize += strlen(pstzPath);
	bufSize += strlen(pvolumeName);
	bufSize += (UINT_HEX_SIZE) * 4;
	bufSize += (ULONG_HEX_SIZE) * 3;
	bufSize += strlen(pfsName);
	bufSize++;	//room for NULL

	*ppstz = malloc(bufSize * sizeof(XCHAR));

	//Make String
	//Format: (path)|(volume name)|(type)|(volume serial)|(fsname)|(max component length)|(flags)|(free bytes)|(total free bytes)|(total bytes)
	strLen = xsprintf(*ppstz, bufSize, "%s|%s|%X|%X|%s|%X|%X|%llX|%llX|%llX", pstzPath, pvolumeName, driveType, volumeSerial, pfsName, maxComponentLength, fsFlags, userFreeBytes, totalFreeBytes, totalBytes);
	return strLen;

#else //WIN32
#error "Invalid OS defined"
#endif
ERROR_HANDLER:
	*ppstz = NULL;
    return -1; 
}

// returns a pointer to an internal list struct that contains all mount
// points detected in (*nix) /etc/mtab, or if that fails, several common
// mount points. If it fails, somehow, it returns NULL. All items in the 
// linked list must be free()'d.
_LIST *getPaths()
{
	_LIST list, *plistcur;
#ifdef LINUX
    FILE *ret;
    char pstzPath[MAX_PATH_SIZE]; 
    char pstzDelim[] = " ";
    char *pstzToken;
    int tokenLen, i; // no need to recalc it all the time
    list.next = NULL; // init, head pointer
    plistcur = &list;
    
   // fprintf(stderr, "opening file\n");
    if (((ret  = (FILE *) fopen("/etc/mtab", "r")) == NULL))
    {

        for (i = 0; i < NUM_POSIX_COMMON_MOUNT_POINTS; i++)
        {
            plistcur->next = malloc(sizeof(_LIST));
            plistcur = plistcur->next;
            plistcur->next = NULL;

            tokenLen = strlen(POSIX_COMMON_MOUNT_POINTS[i]) + 1;
            plistcur->pstzPath = malloc(sizeof(char) * tokenLen);
            strncpy(plistcur->pstzPath, POSIX_COMMON_MOUNT_POINTS[i], tokenLen);
            plistcur->pstzPath[tokenLen - 1] = '\0';
        }
    }
    else 
    {
       // fprintf(stderr, "beginning parse\n");

        while (fgets(pstzPath, MAX_PATH_SIZE, ret) != NULL)
        {
            // mtab is space-separated, and entries are divided by \n's
            // mtab has to be semi valid in order for the system
            // to boot; so, skip comments and just deal with any errors
            // in the statfs function.
            pstzToken = strtok(pstzPath, pstzDelim);
            if (pstzToken[0] == '#') // if comment, skip line
                continue;
            pstzToken = strtok(NULL, pstzDelim); // get 2nd token

            plistcur->next = malloc(sizeof(_LIST));
            plistcur = plistcur->next;
            plistcur->next = NULL;

            tokenLen = strlen(pstzToken) + 1;
            plistcur->pstzPath = malloc(sizeof(char) * tokenLen);
            strncpy(plistcur->pstzPath, pstzToken, tokenLen);
            plistcur->pstzPath[tokenLen - 1] = '\0'; // strncpy doesn't always null strings, especially if it fails
        }
        //fprintf(stderr, "leaving while\n");
    }
    fclose(ret);
    //fprintf(stderr, "entering test loop\n");

    
#ifdef TEST
    plistcur = list.next;
    fprintf(stderr, "\nPrinting list struct being returned from mtab...\n");
    while (plistcur != NULL)
    {
        _LIST *temp;

        fprintf(stderr, "%s\n", plistcur->pstzPath);
        temp = plistcur;
        plistcur = plistcur->next;
        //free(temp->pstzPath); // struct used later in testing
        //free(temp);
    }
#endif  //end test
#elif WIN32 //end LINUX
	int idxPath = 0;
	int size;
	char pathBuffer[512];
	HANDLE hVolumeSearch;
	

	list.next = (_LIST*) malloc(sizeof(_LIST));
	list.next->pstzPath = malloc(MAX_PATH + 1);
	plistcur = &list;
	
	/* DOS/Pre-NT version */
	size = GetLogicalDriveStrings(511, pathBuffer);
	printf("Starting string: %s\n", pathBuffer);
	assert(size < 511);
	//Parse Strings
	do{
		plistcur = plistcur->next;
		strcpy_s(plistcur->pstzPath, (MAX_PATH + 1), (pathBuffer + idxPath));
#ifdef TEST
		printf("MSDOS Volume Found: %s\n", plistcur->pstzPath);
#endif

		plistcur->next = (_LIST*) malloc(sizeof(_LIST));
		plistcur->next->pstzPath = malloc(MAX_PATH + 1);
		while(pathBuffer[idxPath] != '\0' && idxPath < size)
			idxPath++;
		idxPath++;
	}while(idxPath < size);


#if 0 //WIN NT version
	hVolumeSearch = FindFirstVolumeA(plistcur->pstzPath, ARRAYSIZE(plistcur->pstzPath));
	
	while(FindNextVolumeA(hVolumeSearch, plistcur->next->pstzPath, (MAX_PATH + 1)))
	{
#ifdef TEST
		printf("Volume Found: %s\n", plistcur->next->pstzPath);
#endif
		plistcur = plistcur->next;
		plistcur->next = (_LIST*) malloc(sizeof(_LIST));
		plistcur->next->pstzPath = malloc(MAX_PATH + 1);
	}
#endif

	//Trailing _LIST not used so free it
	free(plistcur->next->pstzPath);
	free(plistcur->next);
	plistcur->next = NULL;


#else
#error "Unsupported OS"
#endif
    return list.next;  
}

// Composes an XCHAR in a fairly simple format to be sent along the wire.
// the XCHAR must be free()'d. Returns the empty string \0 on failure.
XCHAR *fsInfoStz()
{
    XCHAR *pstz;
    _LIST *plistTop;
    _LIST *plist;
    int bufSize = 0;
    int idx = 0;
    
    //Generate statfs strings and get total buffer length
    plistTop = plist = getPaths();
    if(plistTop == NULL)
    {
        bufSize = (strlen(MODULE_ERROR_CSTRING) * sizeof(XCHAR)) + 1;
        pstz = malloc(bufSize * sizeof(XCHAR));
        xsprintf(pstz, bufSize, "%s", MODULE_ERROR_CSTRING);
        return pstz;
    }
    
    while(plist != NULL)
    {
		//We do a clever trick here, if theres an error pstzStatfs will be NULL, and bufSize will be decreminted
		//The null will tell later stages to skip this, and the -1 will offset the room added for the deliminator
		//Net result, we don't affect the buffer size when we fail :)
        bufSize += getStatfs(plist->pstzPath, &plist->pstzStatfs);
        bufSize++;  //room for record deliminator
        plist = plist->next;
    }
    bufSize += 1;   //room for NULL

    pstz = malloc(sizeof(XCHAR) * bufSize);
    pstz[0] = '\0';

    //Concat everything to the output string
    plist = plistTop;
    while(plist != NULL)
    {
        _LIST *plistT;
		if(plist->pstzStatfs)
		{
			idx += xstrcat(pstz + idx, bufSize - idx, plist->pstzStatfs);

			pstz[idx++] = FSINFO_RECORD_DELIM;
			pstz[idx] = '\0';   //so we don't confuse xstrcat()
		}
        plistT = plist;
        plist = plist->next;
        free(plistT->pstzStatfs);
        free(plistT->pstzPath);
        free(plistT);
    }
    pstz[idx++] = '\0';
    assert(idx == bufSize);
    assert(pstz[idx - 1] == '\0');
    return pstz;
}

//supposed to feed it several paths and it returns the fstype, free, etc
#ifdef LINUX //REMOVE THIS WHEN WINDOWS SUPPORT IS ADDED
#ifdef TEST
#include <stdio.h>
int main()
{
//    int err;
//    char *path = "/home/phill";
    char buf[1024];
    XCHAR *pusr;
    XCHAR *phome;
    XCHAR *pvar;
    XCHAR *pstz;
    _LIST *plistPaths;
   
    printf("printing various path mount points and returning the info\n");
    if (getStatfs("/usr", &pusr) > 0)
    {
        XtoC(pusr, buf);
        printf("%s\n", buf);
    }
    else
    {
        printf("%d", errno);
        printf("okay, so error:\n");
        if (errno == EOVERFLOW)
        {
            printf("ENOSYS");
        }
    }
    if (getStatfs("/home", &phome) > 0)
    {
        XtoC(phome, buf);
        printf("%s\n", buf);
    }
    if(getStatfs("/var", &pvar) > 0)
    {
        XtoC(pvar, buf);
        printf("%s\n", buf);
    }
    free(pusr);
    free(phome);
    free(pvar);
    plistPaths = getPaths();
    if (plistPaths == NULL)
    {
        printf("Unsuccessful\n");
    }
    else
    {
        printf("getPaths() Output:");
        while(plistPaths != NULL)
        {
            _LIST *plistT;
            XCHAR *pstzStat;
            getStatfs(plistPaths->pstzPath, &pstzStat);
            XtoC(pstzStat, buf);
            printf("\t%s:\t%s\n", plistPaths->pstzPath, buf);
            free(pstzStat);
            free(plistPaths->pstzPath);
            plistT = plistPaths;
            plistPaths = plistPaths->next;
            free(plistT);
        }
    }
    pstz = fsInfoStz();
    XtoC(pstz, buf);
    printf("fsInfoStz() Output: %s\n", buf);
    free(pstz);

    return 0;
}
#endif // test
#else //LINUX
#ifdef TEST
int main(void)
{
	char buf[512];
	XCHAR *pstz;

	getStatfs("C:\\", &pstz);
	XtoC(pstz, buf);
	printf("getStatfs(\"C:\\\"): %s", buf);
	free(pstz);

	pstz = fsInfoStz();
	XtoC(pstz, buf);
	printf("fsInfostz(): %s\n", buf);
	free(pstz);

	return 0;
}
#endif //TEST
#endif //!LINUX
