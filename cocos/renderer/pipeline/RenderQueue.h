#pragma once

#include "Define.h"

namespace cc {
namespace pipeline {

class CC_DLL RenderQueue : public gfx::Object {
public:
    RenderQueue(const RenderPassDesc &desc);
    
    void clear();
    bool insertRenderPass(RenderObject *renderObj, uint subModelIdx, uint passIdx);
    void recordCommandBuffer(gfx::GFXDevice *device, gfx::GFXRenderPass *renderPass, gfx::GFXCommandBuffer *cmdBuff);
    void sort();
    
private:
    RenderPassList _queue;
    RenderPassDesc _passDesc;
};

} // namespace pipeline
} // namespace cc