#include "pile.h"
#include "moteur.h"
#include <stdlib.h>
#include <assert.h>

static int _haut = -1;
static surface_t * _pile[PILE_MAX];

void push(surface_t * planet){
    _pile[++_haut] = planet;
}

surface_t * pop(void){
    return _pile[_haut--];
}


int empty(void){
    return _haut < 0;
}

