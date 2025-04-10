/*
 *   decode.c string decoding utilities
 *   Copyright (C) 2012-2018 Elio Blanca <eblanca76@users.sourceforge.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/*
 * This takes the command line strings starting at optind and concatenate them in order
 * to build a long string. Then this string will be compared with the actual key (codename).
 * Returns 0 if the strings don't match, nonzero otherwise.
 */
char validate (int argc, char **argv, int optind, int maxlen, char *codename)
{
    char *source;
    int avail=maxlen-1,i=0;

    if (optind<argc)
    {
        /* `calloc' allocates and cleares the memory block before returning */
        source=calloc((size_t)maxlen,sizeof(char));

        if (source!=NULL) /* check allocate */
        {
            strncat(source,argv[optind],avail);
            optind++;
            avail-=strlen(source);
            while ((optind<argc)&&(avail>1))
            {
                strncat(source," ",avail);
                strncat(source,argv[optind],avail-1);
                optind++;
                avail=maxlen-strlen(source)-1;
            }

            /* command line string is now ready, now validate it */
            avail=strlen(codename);

            if ((int)strlen(source)==avail)
            {
                /* place white spaces corresponding to the white spaces in the codename
                   so the comparison will be easier */
                /* to be extended for other critical characters */
                for (i=0; i<avail; i++)
                {
                    if (
                        codename[i]==' '
                        ||
                        codename[i]=='\''
                       )
                        source[i]=codename[i];
                }

                i=!(strcasecmp(codename,source));
            }
            free(source);
        }
    }
    return (char)i;
}

/*
 * The scramble process itself is really trivial.
 * This routine swaps the higher nibble from the first byte with the lower nibble from the last byte.
 * Then it takes into account the second byte and the last but one byte and so on.
 * The result is an un-readable string which can be un-scrambled using the same process
 * (it is a symmetrical coding). The source data into 'scrambled.h' are calculated this way.
 */
void scramble (char *s, int len)
/* `s' is the source string to be scrambled, `len' holds its length */
{
    char g1,g2;
    int i,q;

    if (len)
    {
        for (i=0, q=len-1; i<len/2; i++, q--)
        {
            g1=(s[i]>>4)&0x0f;
            g2=(s[q]&0x0f)<<4;

            s[i]=(s[i]&0x0f)|g2;
            s[q]=(s[q]&0xf0)|g1;
        }

        if (len%2)
        {
            g1=(s[i]>>4)&0x0f;
            g2=(s[i]&0x0f)<<4;
            s[i]=g1|g2;
        }
    }
}

/*
 * This routine works as a wrapper around the real decoding function.
 */
char *decode (char *string, int length)
{
    scramble(string,length);
    return string;
}

