#ifndef __MARIO_H__
#define __MARIO_H__

#include "Musica.h"

class Mario : public Musica
{
  private:
    static int melody[];
    static int tempo[];
    
  public:
    Mario();
};

#endif //__MARIO_H__
