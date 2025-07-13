#include "part.h"

Part::Part(): BasePart(&TYPE) {
}

Part::Part(PartConstructParams params): BasePart(&TYPE, params) {                      
    
}