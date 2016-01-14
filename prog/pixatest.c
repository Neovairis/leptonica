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
 * pixatest.c
 *
 *    tests removal of components
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32 CONNECTIVITY = 8;


main(int    argc,
     char **argv)
{
l_int32      size, i, n, n0;
BOXA        *boxa;
GPLOT       *gplot;
NUMA        *nax, *nay1, *nay2;
PIX         *pixs, *pixd;
static char  mainName[] = "pixatest";

    if (argc != 1)
	exit(ERROR_INT(" Syntax:  pixatest", mainName, 1));

    if ((pixs = pixRead("feyn.tif")) == NULL)
	exit(ERROR_INT("pixs not made", mainName, 1));

    /* ----------------  Remove small components --------------- */
    boxa = pixConnComp(pixs, NULL, 8);
    n0 = boxaGetCount(boxa);
    nax = numaMakeSequence(0, 2, 51);
    nay1 = numaCreate(51);
    nay2 = numaCreate(51);
    boxaDestroy(&boxa);

    fprintf(stderr, "\n Remove if Either\n");
    fprintf(stderr, "Iter 0: n = %d\n", n0);
    numaAddNumber(nay1, n0);
    for (i = 1; i <= 50; i++) {
        size = 2 * i;
        pixd = pixRemoveSmallComponents(pixs, size, size, CONNECTIVITY,
                                        L_REMOVE_IF_EITHER, L_CLONE, NULL);
        boxa = pixConnComp(pixd, NULL, 8);
        n = boxaGetCount(boxa);
        numaAddNumber(nay1, n);
        fprintf(stderr, "Iter %d: n = %d\n", i, n);
        boxaDestroy(&boxa);
        pixDestroy(&pixd);
    }

    fprintf(stderr, "\n Remove if Both\n");
    fprintf(stderr, "Iter 0: n = %d\n", n0);
    numaAddNumber(nay2, n0);
    for (i = 1; i <= 50; i++) {
        size = 2 * i;
        pixd = pixRemoveSmallComponents(pixs, size, size, CONNECTIVITY,
                                        L_REMOVE_IF_BOTH, L_CLONE, NULL);
        boxa = pixConnComp(pixd, NULL, 8);
        n = boxaGetCount(boxa);
        numaAddNumber(nay2, n);
        fprintf(stderr, "Iter %d: n = %d\n", i, n);
        boxaDestroy(&boxa);
        pixDestroy(&pixd);
    }

    gplot = gplotCreate(nax, nay1, "junkplot1", GPLOT_X11, GPLOT_LINES,
                        "Remove small: number of cc vs size removed",
                        "remove if either", "min size", "number of c.c.");
    gplotAddPlot(gplot, nax, nay2, GPLOT_LINES, "remove if both");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);

    /* ----------------  Remove large components --------------- */
    numaEmpty(nay1);
    numaEmpty(nay2);

    fprintf(stderr, "\n Remove if Either\n");
    fprintf(stderr, "Iter 0: n = %d\n", 0);
    numaAddNumber(nay1, 0);
    for (i = 1; i <= 50; i++) {
        size = 2 * i;
        pixd = pixRemoveLargeComponents(pixs, size, size, CONNECTIVITY,
                                        L_REMOVE_IF_EITHER, L_CLONE, NULL);
        boxa = pixConnComp(pixd, NULL, 8);
        n = boxaGetCount(boxa);
        numaAddNumber(nay1, n);
        fprintf(stderr, "Iter %d: n = %d\n", i, n);
        boxaDestroy(&boxa);
        pixDestroy(&pixd);
    }

    fprintf(stderr, "\n Remove if Both\n");
    fprintf(stderr, "Iter 0: n = %d\n", 0);
    numaAddNumber(nay2, 0);
    for (i = 1; i <= 50; i++) {
        size = 2 * i;
        pixd = pixRemoveLargeComponents(pixs, size, size, CONNECTIVITY,
                                        L_REMOVE_IF_BOTH, L_CLONE, NULL);
        boxa = pixConnComp(pixd, NULL, 8);
        n = boxaGetCount(boxa);
        numaAddNumber(nay2, n);
        fprintf(stderr, "Iter %d: n = %d\n", i, n);
        boxaDestroy(&boxa);
        pixDestroy(&pixd);
    }

    gplot = gplotCreate(nax, nay1, "junkplot2", GPLOT_X11, GPLOT_LINES,
                        "Remove large: number of cc vs size removed",
                        "remove if either", "min size", "number of c.c.");
    gplotAddPlot(gplot, nax, nay2, GPLOT_LINES, "remove if both");
    gplotMakeOutput(gplot);
    gplotDestroy(&gplot);

    numaDestroy(&nax);
    numaDestroy(&nay1);
    numaDestroy(&nay2);
    pixDestroy(&pixs);
    exit(0);
}
