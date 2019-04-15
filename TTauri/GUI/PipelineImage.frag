#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler samp;
layout(set = 0, binding = 1) uniform texture2D textures[16];

layout(location = 0) in vec2 inClippingRectangleMinimum;
layout(location = 1) in vec2 inClippingRectangleMaximum;
layout(location = 2) in vec3 inAtlasPosition;
layout(location = 3) in float inAlpha;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;

void main() {
    if (clamp(gl_FragCoord.xy, inClippingRectangleMinimum, inClippingRectangleMaximum) != gl_FragCoord.xy) {
        discard;
    }

    int atlasTextureIndex = int(inAtlasPosition.z);
    vec2 textureCoord = inAtlasPosition.xy;

    outColor = texture(sampler2D(textures[atlasTextureIndex], samp), textureCoord);
}