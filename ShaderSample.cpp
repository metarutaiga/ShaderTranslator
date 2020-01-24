//==============================================================================
// ShaderTranslator
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================

#include "ShaderSample.h"

//------------------------------------------------------------------------------
static const char* const sampleHLSLVertexShaderLegacy =
    "float4x4 World;\n"
    "float4x4 View;\n"
    "float4x4 Projection;\n"
    "\n"
    "struct VS_INPUT\n"
    "{\n"
    "    float3 position : POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 position : POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "PS_INPUT main(VS_INPUT input)\n"
    "{\n"
    "    PS_INPUT output;\n"
    "    output.position = mul(World * View * Projection, float4(input.position.xyz, 1.0f));\n"
    "    output.color = input.color;\n"
    "    output.uv = input.uv;\n"
    "    return output;\n"
    "}";
//------------------------------------------------------------------------------
static const char* const sampleHLSLVertexShader =
    "float4x4 World;\n"
    "float4x4 View;\n"
    "float4x4 Projection;\n"
    "\n"
    "struct VS_INPUT\n"
    "{\n"
    "    float3 position : POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 position : SV_POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "PS_INPUT main(VS_INPUT input)\n"
    "{\n"
    "    PS_INPUT output;\n"
    "    output.position = mul(World * View * Projection, float4(input.position.xyz, 1.0f));\n"
    "    output.color = input.color;\n"
    "    output.uv = input.uv;\n"
    "    return output;\n"
    "}";
//------------------------------------------------------------------------------
static const char* const sampleHLSLPixelShaderLegacy =
    "sampler2D Sampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 position : POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "float4 main(PS_INPUT input) : COLOR0\n"
    "{\n"
    "    float4 color = input.color * tex2D(Sampler, input.uv);\n"
    "    return color;\n"
    "}";
//------------------------------------------------------------------------------
static const char* const sampleHLSLPixelShader =
    "SamplerState Sampler;\n"
    "Texture2D Texture;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 position : SV_POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "float4 main(PS_INPUT input) : SV_TARGET\n"
    "{\n"
    "    float4 color = input.color * Texture.Sample(Sampler, input.uv);\n"
    "    return color;\n"
    "}";
//------------------------------------------------------------------------------
