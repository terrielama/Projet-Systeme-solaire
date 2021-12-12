#include "moteur.h"
#ifndef _PILE_H
#define _PILE_H

#define PILE_MAX 256


#ifdef __cplusplus
extern "C" {
#endif

    extern void push(surface_t * planet);
    extern surface_t * pop(void);
    extern int empty(void);

#ifdef __cplusplus
}

#endif

#endif