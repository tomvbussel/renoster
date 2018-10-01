#include "renoster/shading.h"

#include "renoster/primitive.h"

namespace renoster {

void ShadingPoint::ComputeShadingInfo()
{
    PrimitiveContext ctx;
    primitive->ComputeShadingInfo(ctx, this);
}

void ShadingPoint::ComputeScatteringFunctions(Allocator & alloc)
{
    PrimitiveContext ctx;
    primitive->ComputeScatteringFunctions(ctx, alloc, this);
}

}  // namespace renoster
