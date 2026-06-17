#ifndef IMATCHER_H
#define IMATCHER_H

#include "BipartiteGraph.h"

class IMatcher {
public:
    virtual ~IMatcher() = default;
    virtual int match(const BipartiteGraph& g) = 0;
};

#endif // IMATCHER_H