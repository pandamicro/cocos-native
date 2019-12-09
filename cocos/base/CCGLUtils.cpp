/****************************************************************************
 Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "CCGLUtils.h"
#include "platform/CCApplication.h"
#include <stdio.h>
#include <cfloat>
#include <cassert>
#include <array>
#include <unordered_map>

NS_CC_BEGIN

// todo: use gl to get the supported number
#define MAX_ATTRIBUTE_UNIT  16
#define MAX_TEXTURE_UNIT 32

//IDEA: Consider to use variable to enable/disable cache state since using macro will not be able to close it if there're serious bugs.
//#undef CC_ENABLE_GL_STATE_CACHE
//#define CC_ENABLE_GL_STATE_CACHE 0


namespace
{
    GLint __currentVertexBuffer = -1;
    GLint __currentIndexBuffer = -1;

    GLint __currentVertexArray = -1;
    std::unordered_map<GLuint, GLint> __vaoNativeIdToBufferSize;
    
    uint32_t __enabledVertexAttribArrayFlag = 0;
    VertexAttributePointerInfo __enabledVertexAttribArrayInfo[MAX_ATTRIBUTE_UNIT];
    
    uint8_t __currentActiveTextureUnit = 0;
    std::array<BoundTextureInfo, MAX_TEXTURE_UNIT> __boundTextureInfos;

    GLint _currentUnpackAlignment = -1;

    bool __unpackFlipY = false;
    bool __premultiplyAlpha = false;

    GLuint __currentOffScreenFbo = 0;
}

static GLint _getBufferSize(GLuint VAO) {
    GLint bufferSize = 0;
    auto iter = __vaoNativeIdToBufferSize.find(VAO);
    if (iter != __vaoNativeIdToBufferSize.end()) {
        bufferSize = iter->second;
    }
    return bufferSize;
}

//IDEA: need to consider invoking this after restarting game.
void ccInvalidateStateCache()
{
    __currentVertexBuffer = -1;
    __currentIndexBuffer = -1;

    __currentVertexArray = -1;

    __enabledVertexAttribArrayFlag = 0;
    for (int i = 0; i < MAX_ATTRIBUTE_UNIT; ++i)
        __enabledVertexAttribArrayInfo[i] = VertexAttributePointerInfo();

    _currentUnpackAlignment = -1;
    __unpackFlipY = false;
    __premultiplyAlpha = false;
}

/****************************************************************************************
 Texture related
 ***************************************************************************************/
void ccActiveTexture(GLenum texture)
{
#if CC_ENABLE_GL_STATE_CACHE
    auto activeTextureUnit = texture - GL_TEXTURE0;
    if(activeTextureUnit < MAX_TEXTURE_UNIT)
    {
       __currentActiveTextureUnit = activeTextureUnit;
    }
#endif
    glActiveTexture(texture);
}

void ccBindTexture(GLenum target, GLuint texture)
{
#if CC_ENABLE_GL_STATE_CACHE
    auto& boundTextureInfo = __boundTextureInfos[__currentActiveTextureUnit];
    //todo: support cache
    if (boundTextureInfo.texture != texture || boundTextureInfo.target != target) {
        boundTextureInfo.texture = texture;
        boundTextureInfo.target = target;
    }
    glBindTexture(target, texture);
#else
    glBindTexture(target, texture);
#endif
}

BoundTextureInfo* getBoundTextureInfo(uint32_t textureUnit)
{
    return &__boundTextureInfos[textureUnit];
}

/****************************************************************************************
 FrameBuffer related
 ***************************************************************************************/

void ccBindFramebuffer(GLenum target,GLuint buffer)
{
    if(Application::getInstance()->isDownsampleEnabled())
    {
        if(target == GL_FRAMEBUFFER && buffer == Application::getInstance()->getMainFBO())
        {
            buffer = __currentOffScreenFbo;
        }
    }

    glBindFramebuffer(target , buffer);
}

void ccActiveOffScreenFramebuffer(GLuint offscreenFbo)
{
    __currentOffScreenFbo = offscreenFbo;
}

/****************************************************************************************
 Buffer related
 ***************************************************************************************/

void ccBindBuffer(GLenum target, GLuint buffer)
{
#if CC_ENABLE_GL_STATE_CACHE
    if (target == GL_ARRAY_BUFFER)
    {
        if (__currentVertexBuffer != buffer)
        {
            __currentVertexBuffer = buffer;
            glBindBuffer(target, buffer);
        }
    }
    else if (target == GL_ELEMENT_ARRAY_BUFFER)
    {
        if (__currentIndexBuffer != buffer)
        {
            __currentIndexBuffer = buffer;
            glBindBuffer(target, buffer);
        }
    }
    else
    {
        glBindBuffer(target, buffer);
    }
#else
    // Should cache it, ccVertexAttribPointer depends on it.
    __currentVertexBuffer = buffer;
    
    glBindBuffer(target, buffer);
#endif
}

void ccDeleteBuffers(GLsizei n, const GLuint * buffers)
{
    for (GLsizei i = 0; i < n; ++i)
    {
        if (buffers[i] == __currentVertexBuffer)
            __currentVertexBuffer = -1;
        else if (buffers[i] == __currentIndexBuffer)
            __currentIndexBuffer = -1;
    }
    glDeleteBuffers(n, buffers);
}

GLint ccGetBoundVertexBuffer()
{
    return __currentVertexBuffer;
}

GLint ccGetBoundIndexBuffer()
{
    return __currentIndexBuffer;
}

void ccBindVertexArray(GLuint VAO) {
#if CC_ENABLE_GL_STATE_CACHE
    if (__currentVertexArray != VAO)
    {
        if (__currentVertexArray >= 1) {
            GLint dataSize = ccGetBufferDataSize();
            __vaoNativeIdToBufferSize[__currentVertexArray] = dataSize;
        }
        if (VAO >= 1 && __vaoNativeIdToBufferSize.find(VAO) == __vaoNativeIdToBufferSize.end()) {
            __vaoNativeIdToBufferSize.emplace(VAO, 0);
            __currentVertexBuffer = -1;
            __currentIndexBuffer = -1;
            __enabledVertexAttribArrayFlag = 0;
        }
        __currentVertexArray = VAO;
        glBindVertexArray(VAO);
    }
#else
    __currentVertexArray = VAO;
    glBindVertexArray(VAO);
#endif
}

void ccDeleteVertexArrays(GLsizei n, const GLuint *arrays) {
    for (GLsizei i = 0; i < n; ++i) {
        if (arrays[i] >= 1) {
            auto iter = __vaoNativeIdToBufferSize.find(arrays[i]);
            if (iter != __vaoNativeIdToBufferSize.end()) {
                __vaoNativeIdToBufferSize.erase(iter);
            }
        }

        if (arrays[i] == __currentVertexArray) {
            __currentVertexArray = -1;
        }
    }
    glDeleteVertexArrays(n, arrays);
}

GLint ccGetBoundVertexArray()
{
    return __currentVertexArray;
}

/****************************************************************************************
 Vertex attribute related
 ***************************************************************************************/

void ccEnableVertexAttribArray(GLuint index)
{
    assert(index < MAX_ATTRIBUTE_UNIT);
    if (index >= MAX_ATTRIBUTE_UNIT)
        return;

    uint32_t flag = 1 << index;
    if (__enabledVertexAttribArrayFlag & flag)
        return;

    __enabledVertexAttribArrayFlag |= flag;
    glEnableVertexAttribArray(index);
}

void ccDisableVertexAttribArray(GLuint index)
{
    if (index >= MAX_ATTRIBUTE_UNIT)
        return;
    uint32_t flag = 1 << index;
    if (__enabledVertexAttribArrayFlag & flag)
    {
        glDisableVertexAttribArray(index);
        __enabledVertexAttribArrayFlag &= ~(1 << index);
    }
}

void ccVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
    assert(index < MAX_ATTRIBUTE_UNIT);
    if (index >= MAX_ATTRIBUTE_UNIT)
        return;

    __enabledVertexAttribArrayInfo[index] = VertexAttributePointerInfo(__currentVertexBuffer, index, size, type, normalized, stride, pointer);

    // IDEA: should check all the values to determine if need to invoke glVertexAttribPointer or not?
    // We don't know if it is a good idea to do it because it needs to compare so many parameters.
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

const VertexAttributePointerInfo* getVertexAttribPointerInfo(GLuint index)
{
    assert(index < MAX_ATTRIBUTE_UNIT);
    if (index >= MAX_ATTRIBUTE_UNIT)
        return nullptr;
    
    // The index is not enabled, return null.
    if (! (__enabledVertexAttribArrayFlag & (1 << index)) )
        return nullptr;
    
    return &__enabledVertexAttribArrayInfo[index];
}

/****************************************************************************************
 Other functions.
 ***************************************************************************************/

void ccViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    if (currentFBO == Application::getInstance()->getMainFBO())
    {
        float scale = Application::getInstance()->getScreenScale();
        x *= scale;
        y *= scale;
        width *= scale;
        height *= scale;
    }
#endif
    glViewport(x, y, width, height);
}

void ccScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    if (currentFBO == Application::getInstance()->getMainFBO())
    {
        float scale = Application::getInstance()->getScreenScale();
        x *= scale;
        y *= scale;
        width *= scale;
        height *= scale;
    }
#endif
    glScissor(x, y, width, height);
}

//IDEA:: ONLY SUPPORT RGBA format now.
static void flipPixelsY(GLubyte *pixels, int bytesPerRow, int rows)
{
    if( !pixels ) { return; }

    GLuint middle = rows/2;
    GLuint intsPerRow = bytesPerRow / sizeof(GLuint);
    GLuint remainingBytes = bytesPerRow - intsPerRow * sizeof(GLuint);

    for( GLuint rowTop = 0, rowBottom = rows-1; rowTop < middle; rowTop++, rowBottom-- ) {

        // Swap bytes in packs of sizeof(GLuint) bytes
        GLuint *iTop = (GLuint *)(pixels + rowTop * bytesPerRow);
        GLuint *iBottom = (GLuint *)(pixels + rowBottom * bytesPerRow);

        GLuint itmp;
        GLint n = intsPerRow;
        do {
            itmp = *iTop;
            *iTop++ = *iBottom;
            *iBottom++ = itmp;
        } while(--n > 0);

        // Swap the remaining bytes
        GLubyte *bTop = (GLubyte *)iTop;
        GLubyte *bBottom = (GLubyte *)iBottom;

        GLubyte btmp;
        switch( remainingBytes ) {
            case 3: btmp = *bTop; *bTop++ = *bBottom; *bBottom++ = btmp;
            case 2: btmp = *bTop; *bTop++ = *bBottom; *bBottom++ = btmp;
            case 1: btmp = *bTop; *bTop = *bBottom; *bBottom = btmp;
        }
    }
}

// Lookup tables for fast [un]premultiplied alpha color values
// From https://bugzilla.mozilla.org/show_bug.cgi?id=662130
static GLubyte* __premultiplyTable = nullptr;

static const GLubyte* premultiplyTable()
{
    if( !__premultiplyTable ) {
        __premultiplyTable = (GLubyte*)malloc(256*256);

        unsigned char *data = __premultiplyTable;
        for( int a = 0; a <= 255; a++ ) {
            for( int c = 0; c <= 255; c++ ) {
                data[a*256+c] = (a * c + 254) / 255;
            }
        }
    }

    return __premultiplyTable;
}

bool ccIsUnpackFlipY()
{
    return __unpackFlipY;
}

bool ccIsPremultiplyAlpha()
{
    return __premultiplyAlpha;
}

void ccPixelStorei(GLenum pname, GLint param)
{
    if (pname == GL_UNPACK_FLIP_Y_WEBGL)
    {
        __unpackFlipY = param == 0 ? false : true;
        return;
    }
    else if (pname == GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL)
    {
        __premultiplyAlpha = param == 0 ? false : true;
        return;
    }
    else if (pname == GL_UNPACK_COLORSPACE_CONVERSION_WEBGL)
    {
        CCLOGERROR("Warning: UNPACK_COLORSPACE_CONVERSION_WEBGL is unsupported\n");
        return;
    }
    else if (pname == GL_UNPACK_ALIGNMENT)
    {
#if CC_ENABLE_GL_STATE_CACHE
        if (_currentUnpackAlignment != param)
        {
            glPixelStorei(pname, param);
            _currentUnpackAlignment = param;
        }
#else
        glPixelStorei(pname, param);
#endif
    }
    else
    {
        glPixelStorei(pname, param);
    }
}

void ccFlipYIfNeeded(GLvoid* pixels, uint32_t pixelBytes, GLsizei height)
{
    if (!pixels
        || !__unpackFlipY
        || height <= 1 || pixelBytes <= 0)
    {
        return;
    }
    
    flipPixelsY((GLubyte*)pixels, pixelBytes / height, height);
}

namespace
{
    void premultiply5551(GLvoid* pixels, uint32_t pixelBytes)
    {
        uint16_t* pointer = (uint16_t*)pixels;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 0;
        for (int i = 0; i < pixelBytes; i += 2, pointer++)
        {
            r = (*pointer & 0xF800) >> 11;
            g = (*pointer & 0x07C0) >> 6;
            b = (*pointer & 0x003D)  >> 1;
            a = *pointer & 0x1;
            *pointer = (r * a << 11 & 0xF800) |
                       (g * a << 6 & 0x07C0)  |
                       (b * a << 1 & 0x003D)  |
                       a;
        }
    }
    
#define TO_4BIT(a, b)  (((a * b) + 14) / 15)
    void premultiply4444(GLvoid* pixels, uint32_t pixelBytes)
    {
        uint16_t* pointer = (uint16_t*)pixels;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 0;
        for (int i = 0; i < pixelBytes; i += 2, pointer++)
        {
            r = (*pointer & 0xF000) >> 12;
            g = (*pointer & 0x0F00) >> 8;
            b = (*pointer & 0x00F0) >> 4;
            a = *pointer & 0xF;
            *pointer = (TO_4BIT(r, a) << 12 & 0xF000) |
                       (TO_4BIT(g, a) << 8 & 0x0F00)  |
                       (TO_4BIT(b, a) << 4 & 0x00F0)  |
                       (uint8_t)(*pointer & 0xF);
        }
    }

    void premultiplyByte(GLvoid* pixels, uint32_t pixelBytes)
    {
        uint8_t* pointer = (uint8_t*)pixels;
        const GLubyte *table = premultiplyTable();
        for (int i = 0; i < pixelBytes; i += 4)
        {
            uint32_t index = pointer[i+3] * 256;
            pointer[i] = table[index + pointer[i]];
            pointer[i+1] = table[index + pointer[i+1]];
            pointer[i+2] = table[index + pointer[i+2]];
        }
    }
}

void ccPremultiptyAlphaIfNeeded(GLvoid* pixels, uint32_t pixelBytes, GLenum format, GLenum type)
{
    if (!pixels
        || !__premultiplyAlpha || pixelBytes <= 0)
    {
        return;
    }
    
    switch (type) {
        case GL_UNSIGNED_SHORT_5_5_5_1:
            premultiply5551(pixels, pixelBytes);
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
            premultiply4444(pixels, pixelBytes);
            break;
        case GL_UNSIGNED_BYTE:
            if (format == GL_RGBA || format == GL_LUMINANCE_ALPHA)
                premultiplyByte(pixels, pixelBytes);
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
            // no alpha channel
            break;
        default:
            // Doesn't support currently.
            break;
    }
}

GLint ccGetBufferDataSize()
{
    GLint result = 0, size = 0;
    if (__currentVertexArray >= 1)
    {
        result = _getBufferSize(__currentVertexArray);
        if (result > 0)
        {
            return result;
        }
    }
    for( int i = 0; i < MAX_ATTRIBUTE_UNIT; i++ ) {
        const VertexAttributePointerInfo *info = getVertexAttribPointerInfo(i);
        if (info != nullptr && info->VBO == __currentVertexBuffer) {
            switch (info->type)
            {
                case GL_BYTE:
                case GL_UNSIGNED_BYTE:
                    size = info->size * sizeof(GLbyte);
                    break;
                case GL_SHORT:
                case GL_UNSIGNED_SHORT:
                    size = info->size * sizeof(GLshort);
                    break;
                case GL_FLOAT:
                    size = info->size * sizeof(GLclampf);
                    break;
                default:
                    size = 0;
                    break;
            }

            result += size;
        }
    }

    return result;
}

void ccClearVaoMap() {
    __vaoNativeIdToBufferSize.clear();
}

NS_CC_END
