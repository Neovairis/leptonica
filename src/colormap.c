/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

/*
 *  colormap.c
 *
 *      Colormap creation, copy, destruction, addition
 *           PIXCMAP    *pixcmapCreate()
 *           PIXCMAP    *pixcmapCopy()
 *           void        pixcmapDestroy()
 *           l_int32     pixcmapAddColor()
 *           l_int32     pixcmapAddNewColor()
 *           l_int32     pixcmapAddBlackOrWhite()
 *           l_int32     pixcmapGetCount()
 *           l_int32     pixcmapGetFreeCount()
 *           l_int32     pixcmapClear()
 *
 *      Colormap random access and test
 *           l_int32     pixcmapGetColor()
 *           l_int32     pixcmapResetColor()
 *           l_int32     pixcmapGetIndex()
 *           l_int32     pixcmapHasColor()
 *           l_int32     pixcmapGetRankIntensity()
 *
 *      Colormap I/O
 *           l_int32     pixcmapReadStream()
 *           l_int32     pixcmapWriteStream()
 *
 *      Extract colormap arrays
 *           l_int32     pixcmapToArrays()
 *
 *      Colormap transforms
 *           l_int32     pixcmapGammaTRC()
 *           l_int32     pixcmapContrastTRC()
 *           l_int32     pixcmapShiftIntensity()
 *           l_int32     pixcmapConvertRGBToHSV()
 *           l_int32     pixcmapConvertHSVToRGB()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"


/*-------------------------------------------------------------*
 *                Colormap creation and addition               *
 *-------------------------------------------------------------*/
/*!
 *  pixcmapCreate()
 *
 *      Input:  depth (bpp, of pix)
 *      Return: cmap, or null on error
 */
PIXCMAP *
pixcmapCreate(l_int32  depth)
{
RGBA_QUAD  *cta;
PIXCMAP    *cmap;

    PROCNAME("pixcmapCreate");

    if (depth != 1 && depth != 2 && depth !=4 && depth != 8)
	return (PIXCMAP *)ERROR_PTR("depth not in {1,2,4,8}", procName, NULL);

    if ((cmap = (PIXCMAP *)CALLOC(1, sizeof(PIXCMAP))) == NULL)
	return (PIXCMAP *)ERROR_PTR("cmap not made", procName, NULL);
    cmap->depth = depth;
    cmap->nalloc = 1 << depth;
    if ((cta = (RGBA_QUAD *)CALLOC(cmap->nalloc, sizeof(RGBA_QUAD))) == NULL)
	return (PIXCMAP *)ERROR_PTR("cta not made", procName, NULL);
    cmap->array = cta;
    cmap->n = 0;

    return cmap;
}


/*!
 *  pixcmapCopy()
 *
 *      Input:  cmaps 
 *      Return: cmapd, or null on error
 */
PIXCMAP *
pixcmapCopy(PIXCMAP  *cmaps)
{
l_int32   nbytes;
PIXCMAP  *cmapd;

    PROCNAME("pixcmapCopy");

    if (!cmaps)
	return (PIXCMAP *)ERROR_PTR("cmaps not defined", procName, NULL);

    if ((cmapd = (PIXCMAP *)CALLOC(1, sizeof(PIXCMAP))) == NULL)
	return (PIXCMAP *)ERROR_PTR("cmapd not made", procName, NULL);
    nbytes = cmaps->nalloc * sizeof(RGBA_QUAD);
    if ((cmapd->array = (void *)CALLOC(1, nbytes)) == NULL)
	return (PIXCMAP *)ERROR_PTR("cmap array not made", procName, NULL);
    memcpy(cmapd->array, cmaps->array, nbytes);
    cmapd->n = cmaps->n;
    cmapd->nalloc = cmaps->nalloc;
    cmapd->depth = cmaps->depth;

    return cmapd;
}


/*!
 *  pixcmapDestroy()
 *
 *      Input:  &cmap (<set to null>)
 *      Return: void
 */
void
pixcmapDestroy(PIXCMAP  **pcmap)
{
PIXCMAP  *cmap;

    PROCNAME("pixcmapDestroy");

    if (pcmap == NULL) {
        L_WARNING("ptr address is null!", procName);
	return;
    }

    if ((cmap = *pcmap) == NULL)
        return;

    FREE(cmap->array);
    FREE(cmap);
    *pcmap = NULL;

    return;
}


/*!
 *  pixcmapAddColor()
 *
 *      Input:  cmap
 *              rval, gval, bval (colormap entry to be added; each number
 *                                is in range [0, ... 255])
 *      Return: 0 if OK, 1 on error
 *
 *  Note: always adds the color if there is room.
 */
l_int32
pixcmapAddColor(PIXCMAP  *cmap,
                l_int32   rval,
                l_int32   gval,
                l_int32   bval)
{
RGBA_QUAD  *cta;

    PROCNAME("pixcmapAddColor");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (cmap->n >= cmap->nalloc)
	return ERROR_INT("no free color entries", procName, 1);

    cta = (RGBA_QUAD *)cmap->array;
    cta[cmap->n].red = rval;
    cta[cmap->n].green = gval;
    cta[cmap->n].blue = bval;
    cmap->n++;

    return 0;
}


/*!
 *  pixcmapAddNewColor()
 *
 *      Input:  cmap
 *              rval, gval, bval (colormap entry to be added; each number
 *                                is in range [0, ... 255])
 *              &index (<return> index of color)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This only adds color if not already there.
 *      (2) This sets index to the input color.
 *      (3) Returns 1 if unable to add this color, so the caller
 *          should check the return value.
 */
l_int32
pixcmapAddNewColor(PIXCMAP  *cmap,
                   l_int32   rval,
                   l_int32   gval,
                   l_int32   bval,
                   l_int32  *pindex)
{
    PROCNAME("pixcmapAddNewColor");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!pindex)
	return ERROR_INT("&index not defined", procName, 1);
    *pindex = 0;  /* init for safety if caller doesn't check return val */

    if (!pixcmapGetIndex(cmap, rval, gval, bval, pindex))  /* found */
        return 0;

    if (cmap->n >= cmap->nalloc)
	return ERROR_INT("no free color entries", procName, 1);

        /* ok, we have to add a color and there is room */
    pixcmapAddColor(cmap, rval, gval, bval);
    *pindex = pixcmapGetCount(cmap) - 1;
    return 0;
}


/*!
 *  pixcmapAddBlackOrWhite()
 *
 *      Input:  cmap
 *              color (0 for black, 1 for white) 
 *              &index (<return> index of color)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This only adds color if not already there.
 *      (2) This sets index to the requested color.
 *      (3) If there is no room in the colormap, returns the index
 *          of the closest color.
 */
l_int32
pixcmapAddBlackOrWhite(PIXCMAP  *cmap,
                       l_int32   color,
                       l_int32  *pindex)
{
    PROCNAME("pixcmapAddBlackOrWhite");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!pindex)
	return ERROR_INT("&index not defined", procName, 1);

    if (color == 0) {  /* black */
        if (pixcmapGetFreeCount(cmap) > 0)
            pixcmapAddNewColor(cmap, 0, 0, 0, pindex);
        else
            pixcmapGetRankIntensity(cmap, 0.0, pindex);
    }
    else {  /* white */
        if (pixcmapGetFreeCount(cmap) > 0)
            pixcmapAddNewColor(cmap, 255, 255, 255, pindex);
        else
            pixcmapGetRankIntensity(cmap, 1.0, pindex);
    }

    return 0;
}


/*!
 *  pixcmapGetCount()
 *
 *      Input:  cmap
 *      Return: count, or 0 on error
 */
l_int32
pixcmapGetCount(PIXCMAP *cmap)
{
    PROCNAME("pixcmapGetCount");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 0);

    return cmap->n;
}


/*!
 *  pixcmapGetFreeCount()
 *
 *      Input:  cmap
 *      Return: free entries, or 0 on error
 */
l_int32
pixcmapGetFreeCount(PIXCMAP *cmap)
{
    PROCNAME("pixcmapGetFreeCount");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 0);

    return (cmap->nalloc - cmap->n);
}


/*!
 *  pixcmapClear()
 *
 *      Input:  cmap
 *      Return: 0 if OK, 1 on error
 *
 *  Note: this removes the colors by setting the count to 0.
 */
l_int32
pixcmapClear(PIXCMAP *cmap)
{
    PROCNAME("pixcmapClear");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    cmap->n = 0;
    return 0;
}


/*-------------------------------------------------------------*
 *                      Colormap random access                 *
 *-------------------------------------------------------------*/
/*!
 *  pixcmapGetColor()
 *
 *      Input:  cmap
 *              index
 *              &rval, &gval, &bval (<return> each color value in l_int32)
 *      Return: 0 if OK, 1 if not accessable (caller should check)
 */
l_int32
pixcmapGetColor(PIXCMAP  *cmap,
                l_int32   index,
		l_int32  *prval,
		l_int32  *pgval,
		l_int32  *pbval)
{
RGBA_QUAD  *cta;

    PROCNAME("pixcmapGetColor");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!prval || !pgval || !pbval)
	return ERROR_INT("&rval, &gval, &bval not all defined", procName, 1);
    if (index >= cmap->n)
	return ERROR_INT("index out of bounds", procName, 1);

    cta = (RGBA_QUAD *)cmap->array;
    *prval = cta[index].red;
    *pgval = cta[index].green;
    *pbval = cta[index].blue;

    return 0;
}


/*!
 *  pixcmapResetColor()
 *
 *      Input:  cmap
 *              index
 *              rval, gval, bval (colormap entry to be reset; each number
 *                                is in range [0, ... 255])
 *      Return: 0 if OK, 1 if not accessable (caller should check)
 *
 *  Note: this resets sets the color of an entry that has already
 *        been set and included in the count of colors
 */
l_int32
pixcmapResetColor(PIXCMAP  *cmap,
                  l_int32   index,
		  l_int32   rval,
		  l_int32   gval,
		  l_int32   bval)
{
RGBA_QUAD  *cta;

    PROCNAME("pixcmapResetColor");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (index >= cmap->n)
	return ERROR_INT("index out of bounds", procName, 1);

    cta = (RGBA_QUAD *)cmap->array;
    cta[index].red = rval;
    cta[index].green = gval;
    cta[index].blue = bval;

    return 0;
}


/*!
 *  pixcmapGetIndex()
 *
 *      Input:  cmap
 *              rval, gval, bval (colormap colors to search for; each number
 *                                is in range [0, ... 255])
 *              &index (<return>)
 *      Return: 0 if found, 1 if not found (caller must check)
 */
l_int32
pixcmapGetIndex(PIXCMAP  *cmap,
		l_int32   rval,
		l_int32   gval,
		l_int32   bval,
                l_int32  *pindex)
{
l_int32     n, i;
RGBA_QUAD  *cta;

    PROCNAME("pixcmapGetIndex");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!pindex)
	return ERROR_INT("&index not defined", procName, 1);
    *pindex = 0;  /* init; not very meaningful */
    n = pixcmapGetCount(cmap);

    cta = (RGBA_QUAD *)cmap->array;
    for (i = 0; i < n; i++) {
        if (rval == cta[i].red && gval == cta[i].green && bval == cta[i].blue) {
            *pindex = i;
            return 0;
        }
    }

    return 1;
}


/*!
 *  pixcmapHasColor()
 *
 *      Input:  cmap
 *              &color (<return> TRUE if cmap has color; FALSE otherwise)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixcmapHasColor(PIXCMAP  *cmap,
                l_int32  *pcolor)
{
l_int32   n, i;
l_int32  *rmap, *gmap, *bmap;

    PROCNAME("pixcmapHasColor");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!pcolor)
	return ERROR_INT("&color not defined", procName, 1);
    *pcolor = FALSE;

    if (pixcmapToArrays(cmap, &rmap, &gmap, &bmap))
	return ERROR_INT("colormap arrays not made", procName, 1);
    n = pixcmapGetCount(cmap);
    for (i = 0; i < n; i++) {
        if ((rmap[i] != gmap[i]) || (rmap[i] != bmap[i])) {
            *pcolor = TRUE;
            break;
        }
    }

    FREE(rmap);
    FREE(gmap);
    FREE(bmap);
    return 0;
}


/*!
 *  pixcmapGetRankIntensity()
 *
 *      Input:  cmap
 *              rankval (0.0 for darkest, 1.0 for lightest color)
 *              &index (<return> the index into the colormap that
 *                      corresponds to the rank intensity color)
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixcmapGetRankIntensity(PIXCMAP    *cmap,
                        l_float32   rankval,
                        l_int32    *pindex)
{
l_int32  n, i, rval, gval, bval, rankindex;
NUMA    *na, *nasort;

    PROCNAME("pixcmapGetRankIntensity");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!pindex)
	return ERROR_INT("&index not defined", procName, 1);
    if (rankval < 0.0 || rankval > 1.0)
	return ERROR_INT("rankval not in [0.0 ... 1.0]", procName, 1);

    n = pixcmapGetCount(cmap);
    na = numaCreate(n);
    for (i = 0; i < n; i++) {
        pixcmapGetColor(cmap, i, &rval, &gval, &bval);
        numaAddNumber(na, rval + gval + bval);
    }
    nasort = numaGetSortIndex(na, L_SORT_INCREASING);
    rankindex = (l_int32)(rankval * (n - 1) + 0.5);
    numaGetIValue(nasort, rankindex, pindex);

    numaDestroy(&na);
    numaDestroy(&nasort);
    return 0;
}



/*-------------------------------------------------------------*
 *                         Colormap I/O                        *
 *-------------------------------------------------------------*/
/*!
 *  pixcmapReadStream()
 *
 *      Input:  stream
 *      Return: cmap, or null on error
 */
PIXCMAP *
pixcmapReadStream(FILE  *fp)
{
l_int32   rval, gval, bval;
l_int32   i, index, ret, depth, ncolors;
PIXCMAP  *cmap;

    PROCNAME("pixcmapReadStream");

    if (!fp)
	return (PIXCMAP *)ERROR_PTR("stream not defined", procName, NULL);

    ret = fscanf(fp, "\nPixcmap: depth = %d bpp; %d colors\n",
	         &depth, &ncolors);
    if (ret != 2 ||
	(depth != 1 && depth != 2 && depth != 4 && depth != 8) ||
	(ncolors < 2 || ncolors > 256))
	return (PIXCMAP *)ERROR_PTR("invalid cmap size", procName, NULL);
    fscanf(fp, "Color    R-val    G-val    B-val\n");
    fscanf(fp, "--------------------------------\n");

    if ((cmap = pixcmapCreate(depth)) == NULL)
	return (PIXCMAP *)ERROR_PTR("cmap not made", procName, NULL);
    for (i = 0; i < ncolors; i++) {
        fscanf(fp, "%3d       %3d      %3d      %3d\n",
	        &index, &rval, &gval, &bval);
	pixcmapAddColor(cmap, rval, gval, bval);
    }

    return cmap;
}


/*!
 *  pixcmapWriteStream()
 *
 *      Input:  stream, cmap
 *      Return: 0 if OK, 1 on error
 */
l_int32
pixcmapWriteStream(FILE     *fp,
                   PIXCMAP  *cmap)
{
l_int32  *rmap, *gmap, *bmap;
l_int32   i;

    PROCNAME("pixcmapWriteStream");

    if (!fp)
	return ERROR_INT("stream not defined", procName, 1);
    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);

    if (pixcmapToArrays(cmap, &rmap, &gmap, &bmap))
	return ERROR_INT("cmap not defined", procName, 1);

    fprintf(fp, "\nPixcmap: depth = %d bpp; %d colors\n", cmap->depth, cmap->n);
    fprintf(fp, "Color    R-val    G-val    B-val\n");
    fprintf(fp, "--------------------------------\n");
    for (i = 0; i < cmap->n; i++)
        fprintf(fp, "%3d       %3d      %3d      %3d\n",
	        i, rmap[i], gmap[i], bmap[i]);
    fprintf(fp, "\n");

    FREE(rmap);
    FREE(gmap);
    FREE(bmap);
    return 0;
}


/*-------------------------------------------------------------*
 *                   Extract colormap arrays                   *
 *-------------------------------------------------------------*/
/*!
 *  pixcmapToArrays()
 *
 *      Input:  colormap
 *              &rmap, &gmap, &bmap  (<return> colormap arrays)
 *      Return: 0 if OK; 1 on error
 */
l_int32
pixcmapToArrays(PIXCMAP   *cmap,
                l_int32  **prmap,
                l_int32  **pgmap,
                l_int32  **pbmap)
{
l_int32    *rmap, *gmap, *bmap;
l_int32     i, ncolors;
RGBA_QUAD  *cta;

    PROCNAME("pixcmapToArrays");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (!prmap || !pgmap || !pbmap)
	return ERROR_INT("&rmap, &gmap, &bmap not all defined", procName, 1);

    *prmap = *pgmap = *pbmap = NULL;
    ncolors = pixcmapGetCount(cmap);

    if (((rmap = (l_int32 *)CALLOC(ncolors, sizeof(l_int32))) == NULL) ||
	((gmap = (l_int32 *)CALLOC(ncolors, sizeof(l_int32))) == NULL) ||
	((bmap = (l_int32 *)CALLOC(ncolors, sizeof(l_int32))) == NULL))
	    return ERROR_INT("calloc fail for *map", procName, 1);
    *prmap = rmap;
    *pgmap = gmap;
    *pbmap = bmap;

    cta = (RGBA_QUAD *)cmap->array;
    for (i = 0; i < ncolors; i++) {
	rmap[i] = cta[i].red;
	gmap[i] = cta[i].green;
	bmap[i] = cta[i].blue;
    }

    return 0;
}


/*-------------------------------------------------------------*
 *                     Colormap transforms                     *
 *-------------------------------------------------------------*/
/*!
 *  pixcmapGammaTRC()
 *
 *      Input:  colormap
 *              gamma (gamma correction; must be > 0.0)
 *              minval  (input value that gives 0 for output; can be < 0)
 *              maxval  (input value that gives 255 for output; can be > 255)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      - in-place transform
 *      - see pixGammaTRC() and numaGammaTRC() in enhance.c for
 *        description and use of transform
 */
l_int32
pixcmapGammaTRC(PIXCMAP   *cmap,
                l_float32  gamma,
                l_int32    minval,
                l_int32    maxval)
{
l_int32   rval, gval, bval, trval, tgval, tbval, i, ncolors;
NUMA     *nag;

    PROCNAME("pixcmapGammaTRC");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (gamma <= 0.0) {
        L_WARNING("gamma must be > 0.0; setting to 1.0", procName);
        gamma = 1.0;
    }
    if (minval >= maxval)
        return ERROR_INT("minval not < maxval", procName, 1);

    if ((nag = numaGammaTRC(gamma, minval, maxval)) == NULL)
        return ERROR_INT("nag not made", procName, 1);

    ncolors = pixcmapGetCount(cmap);
    for (i = 0; i < ncolors; i++) {
        pixcmapGetColor(cmap, i, &rval, &gval, &bval);
        numaGetIValue(nag, rval, &trval);
        numaGetIValue(nag, gval, &tgval);
        numaGetIValue(nag, bval, &tbval);
        pixcmapResetColor(cmap, i, trval, tgval, tbval);
    }

    numaDestroy(&nag);
    return 0;
}


/*!
 *  pixcmapContrastTRC()
 *
 *      Input:  colormap
 *              factor (generally between 0.0 (no enhancement)
 *                      and 1.0, but can be larger than 1.0)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      - in-place transform
 *      - see pixContrastTRC() and numaContrastTRC() in enhance.c for
 *        description and use of transform
 */
l_int32
pixcmapContrastTRC(PIXCMAP   *cmap,
                   l_float32  factor)
{
l_int32   i, ncolors, rval, gval, bval, trval, tgval, tbval;
NUMA     *nac;

    PROCNAME("pixcmapContrastTRC");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (factor < 0.0) {
        L_WARNING("factor must be >= 0.0; setting to 0.0", procName);
        factor = 0.0;
    }

    if ((nac = numaContrastTRC(factor)) == NULL)
        return ERROR_INT("nac not made", procName, 1);

    ncolors = pixcmapGetCount(cmap);
    for (i = 0; i < ncolors; i++) {
        pixcmapGetColor(cmap, i, &rval, &gval, &bval);
        numaGetIValue(nac, rval, &trval);
        numaGetIValue(nac, gval, &tgval);
        numaGetIValue(nac, bval, &tbval);
        pixcmapResetColor(cmap, i, trval, tgval, tbval);
    }

    numaDestroy(&nac);
    return 0;
}


/*!
 *  pixcmapShiftIntensity()
 *
 *      Input:  colormap
 *              fraction (between -1.0 and +1.0)
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      - in-place transform
 *      - This does a proportional shift of the intensity for each color.
 *      - If fraction < 0.0, it moves all colors towards (0,0,0).
 *        This darkens the image.
 *      - If fraction > 0.0, it moves all colors towards (255,255,255)
 *        This fades the image.
 *      - The equivalent transform can be accomplished with pixcmapGammaTRC(),
 *        but it is considerably more difficult (see numaGammaTRC()).
 */
l_int32
pixcmapShiftIntensity(PIXCMAP   *cmap,
                      l_float32  fraction)
{
l_int32   i, ncolors, rval, gval, bval;

    PROCNAME("pixcmapShiftIntensity");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);
    if (fraction < -1.0 || fraction > 1.0)
	return ERROR_INT("fraction not in [-1.0, 1.0]", procName, 1);

    ncolors = pixcmapGetCount(cmap);
    for (i = 0; i < ncolors; i++) {
        pixcmapGetColor(cmap, i, &rval, &gval, &bval);
        if (fraction < 0.0)
            pixcmapResetColor(cmap, i,
	                      (l_int32)((1.0 + fraction) * rval),
                              (l_int32)((1.0 + fraction) * gval),
                              (l_int32)((1.0 + fraction) * bval));
        else
            pixcmapResetColor(cmap, i,
	                      rval + (l_int32)(fraction * (255 - rval)),
                              gval + (l_int32)(fraction * (255 - gval)),
                              bval + (l_int32)(fraction * (255 - bval)));
    }

    return 0;
}


/*!
 *  pixcmapConvertRGBToHSV()
 *
 *      Input:  colormap
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      - in-place transform
 *      - See convertRGBToHSV() for def'n of HSV space.
 *      - replaces: r --> h, g --> s, b --> v
 */
l_int32
pixcmapConvertRGBToHSV(PIXCMAP   *cmap)
{
l_int32   i, ncolors, rval, gval, bval, hval, sval, vval;

    PROCNAME("pixcmapConvertRGBToHSV");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);

    ncolors = pixcmapGetCount(cmap);
    for (i = 0; i < ncolors; i++) {
        pixcmapGetColor(cmap, i, &rval, &gval, &bval);
        convertRGBToHSV(rval, gval, bval, &hval, &sval, &vval);
        pixcmapResetColor(cmap, i, hval, sval, vval);
    }
    return 0;
}


/*!
 *  pixcmapConvertHSVToRGB()
 *
 *      Input:  colormap
 *      Return: 0 if OK; 1 on error
 *
 *  Notes:
 *      - in-place transform
 *      - See convertRGBToHSV() for def'n of HSV space.
 *      - replaces: h --> r, s --> g, v --> b
 */
l_int32
pixcmapConvertHSVToRGB(PIXCMAP   *cmap)
{
l_int32   i, ncolors, rval, gval, bval, hval, sval, vval;

    PROCNAME("pixcmapConvertHSVToRGB");

    if (!cmap)
	return ERROR_INT("cmap not defined", procName, 1);

    ncolors = pixcmapGetCount(cmap);
    for (i = 0; i < ncolors; i++) {
        pixcmapGetColor(cmap, i, &hval, &sval, &vval);
        convertHSVToRGB(hval, sval, vval, &rval, &gval, &bval);
        pixcmapResetColor(cmap, i, rval, gval, bval);
    }
    return 0;
}
