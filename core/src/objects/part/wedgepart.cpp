#include "wedgepart.h"

WedgePart::WedgePart(): BasePart(&TYPE) {
}

WedgePart::WedgePart(PartConstructParams params): BasePart(&TYPE, params) {}