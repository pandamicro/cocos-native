#pragma once
#include "base/ccConfig.h"

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"

extern se::Object* __jsb_cc_gfx_GLES2Device_proto;
extern se::Class* __jsb_cc_gfx_GLES2Device_class;

bool js_register_cc_gfx_GLES2Device(se::Object* obj);
bool register_all_gles2(se::Object* obj);
SE_DECLARE_FUNC(js_gles2_GLES2Device_checkExtension);
SE_DECLARE_FUNC(js_gles2_GLES2Device_useInstancedArrays);
SE_DECLARE_FUNC(js_gles2_GLES2Device_useVAO);
SE_DECLARE_FUNC(js_gles2_GLES2Device_useDrawInstanced);
SE_DECLARE_FUNC(js_gles2_GLES2Device_useDiscardFramebuffer);
SE_DECLARE_FUNC(js_gles2_GLES2Device_GLES2Device);
