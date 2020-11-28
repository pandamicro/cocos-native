#pragma once

namespace cc {
namespace gfx {

class CCMTLStateCache;
class CCMTLCommandAllocator;
class CCMTLGPUStagingBufferPool;

class CCMTLDevice : public Device {
public:
    CCMTLDevice();
    ~CCMTLDevice();

    using Device::createCommandBuffer;
    using Device::createFence;
    using Device::createQueue;
    using Device::createBuffer;
    using Device::createTexture;
    using Device::createSampler;
    using Device::createShader;
    using Device::createInputAssembler;
    using Device::createRenderPass;
    using Device::createFramebuffer;
    using Device::createDescriptorSet;
    using Device::createDescriptorSetLayout;
    using Device::createPipelineLayout;
    using Device::createPipelineState;
    using Device::copyBuffersToTexture;

    virtual bool initialize(const DeviceInfo &info) override;
    virtual void destroy() override;
    virtual void resize(uint width, uint height) override;
    virtual void acquire() override;
    virtual void present() override;
    virtual CommandBuffer *createCommandBuffer() override;
    virtual Fence *createFence() override;
    virtual Queue *createQueue() override;
    virtual Buffer *createBuffer() override;
    virtual Texture *createTexture() override;
    virtual Sampler *createSampler() override;
    virtual Shader *createShader() override;
    virtual InputAssembler *createInputAssembler() override;
    virtual RenderPass *createRenderPass() override;
    virtual Framebuffer *createFramebuffer() override;
    virtual DescriptorSet *createDescriptorSet() override;
    virtual DescriptorSetLayout *createDescriptorSetLayout() override;
    virtual PipelineLayout *createPipelineLayout() override;
    virtual PipelineState *createPipelineState() override;
    virtual void copyBuffersToTexture(const uint8_t *const *buffers, Texture *dst, const BufferTextureCopy *regions, uint count) override;
    virtual void blitBuffer(void *srcBuffer, uint offset, uint size, void *dstBuffer);

    CC_INLINE CCMTLStateCache *getStateCache() const { return _stateCache; }
    CC_INLINE void *getMTLCommandQueue() const { return _mtlCommandQueue; }
    CC_INLINE void *getMTKView() const { return _mtkView; }
    CC_INLINE void *getMTLLayer() const { return _mtlLayer; }
    CC_INLINE void *getMTLDevice() const { return _mtlDevice; }
    CC_INLINE uint getMaximumSamplerUnits() const { return _maxSamplerUnits; }
    CC_INLINE uint getMaximumColorRenderTargets() const { return _maxColorRenderTargets; }
    CC_INLINE uint getMaximumBufferBindingIndex() const { return _maxBufferBindingIndex; }
    CC_INLINE bool isIndirectCommandBufferSupported() const { return _icbSuppored; }
    CC_INLINE bool isIndirectDrawSupported() const { return _indirectDrawSupported; }
    CC_INLINE CCMTLGPUStagingBufferPool *gpuStagingBufferPool() const { return _gpuStagingBufferPool; }
    CC_INLINE bool isSamplerDescriptorCompareFunctionSupported() const { return _isSamplerDescriptorCompareFunctionSupported; }

private:
    CCMTLStateCache *_stateCache = nullptr;

    void *_mtlCommandQueue = nullptr;
    void *_mtkView = nullptr;
    void *_mtlDevice = nullptr;
    void *_mtlLayer = nullptr;
    unsigned long _mtlFeatureSet = 0;
    uint _maxSamplerUnits = 0;
    uint _maxColorRenderTargets = 0;
    uint _maxBufferBindingIndex = 0;
    bool _icbSuppored = false;
    bool _indirectDrawSupported = false;
    bool _isSamplerDescriptorCompareFunctionSupported = false;
    CCMTLGPUStagingBufferPool *_gpuStagingBufferPool = nullptr;
};

} // namespace gfx
} // namespace cc
