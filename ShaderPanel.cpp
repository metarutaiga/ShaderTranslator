//==============================================================================
// xxImGui : Plugin ShaderTranslator Source
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include <d3dcompiler.h>

#include "ShaderPanel.h"

//------------------------------------------------------------------------------
static const char* const vertexShader =
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
static const char* const pixelShader20 =
    "sampler2D Sampler;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 position : SV_POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "float4 main(PS_INPUT input) : SV_Target\n"
    "{\n"
    "    float4 color = input.color * tex2D(Sampler, input.uv);\n"
    "    return color;\n"
    "}";
//------------------------------------------------------------------------------
static const char* const pixelShader40 =
    "sampler Sampler;\n"
    "Texture2D Texture;\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 position : SV_POSITION;\n"
    "    float4 color : COLOR0;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "\n"
    "float4 main(PS_INPUT input) : SV_Target\n"
    "{\n"
    "    float4 color = input.color * Texture.Sample(Sampler, input.uv);\n"
    "    return color;\n"
    "}";
//------------------------------------------------------------------------------
void ShaderPanel(const UpdateData& updateData, bool& show)
{
    ImGui::SetNextWindowSize(ImVec2(1200 * updateData.windowScale, 600 * updateData.windowScale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Shader Panel", &show))
    {
        static char version[32];
        static char source[65536];
        static char target[65536];
        static char log[256];

        ImVec2 windowSize = ImGui::GetWindowSize();
        bool compile = false;

        if (ImGui::Button("vs_1_1"))
        {
            snprintf(version, sizeof(version), "vs_1_1");
            snprintf(source, sizeof(source), "%s", vertexShader);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("vs_2_0"))
        {
            snprintf(version, sizeof(version), "vs_2_0");
            snprintf(source, sizeof(source), "%s", vertexShader);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("vs_3_0"))
        {
            snprintf(version, sizeof(version), "vs_3_0");
            snprintf(source, sizeof(source), "%s", vertexShader);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("vs_4_0"))
        {
            snprintf(version, sizeof(version), "vs_4_0");
            snprintf(source, sizeof(source), "%s", vertexShader);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("vs_5_0"))
        {
            snprintf(version, sizeof(version), "vs_5_0");
            snprintf(source, sizeof(source), "%s", vertexShader);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("ps_2_0"))
        {
            snprintf(version, sizeof(version), "ps_2_0");
            snprintf(source, sizeof(source), "%s", pixelShader20);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("ps_3_0"))
        {
            snprintf(version, sizeof(version), "ps_3_0");
            snprintf(source, sizeof(source), "%s", pixelShader20);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("ps_4_0"))
        {
            snprintf(version, sizeof(version), "ps_4_0");
            snprintf(source, sizeof(source), "%s", pixelShader40);
            compile = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("ps_5_0"))
        {
            snprintf(version, sizeof(version), "ps_5_0");
            snprintf(source, sizeof(source), "%s", pixelShader40);
            compile = true;
        }

        if (ImGui::InputTextMultiline("##source", source, IM_ARRAYSIZE(source), ImVec2(windowSize.x / 2.0f, ImGui::GetTextLineHeight() * 36)))
        {
            compile = true;
        }

        if (compile)
        {
            static void* D3DCompiler = xxLoadLibrary(D3DCOMPILER_DLL_A);
            static pD3DCompile D3DCompile = (pD3DCompile)xxGetProcAddress(D3DCompiler, "D3DCompile");
            static pD3DDisassemble D3DDisassemble = (pD3DDisassemble)xxGetProcAddress(D3DCompiler, "D3DDisassemble");

            target[0] = '\0';

            ID3DBlob* compiled = nullptr;
            ID3DBlob* error = nullptr;
            float time = xxGetCurrentTime();
            D3DCompile(source, strlen(source), nullptr, nullptr, nullptr, "main", version, 0, 0, &compiled, &error);
            time = xxGetCurrentTime() - time;
            if (compiled)
            {
                ID3DBlob* disassembled = nullptr;
                D3DDisassemble(compiled->GetBufferPointer(), compiled->GetBufferSize(), D3D_DISASM_INSTRUCTION_ONLY, nullptr, &disassembled);
                if (disassembled)
                {
                    size_t size = disassembled->GetBufferSize();
                    memcpy(target, disassembled->GetBufferPointer(), size);
                    target[size] = '\0';

                    disassembled->Release();
                }

                compiled->Release();
                if (error)
                {
                    error->Release();
                    error = nullptr;
                }
            }
            if (error)
            {
                size_t size = error->GetBufferSize();
                memcpy(target, error->GetBufferPointer(), size);
                target[size] = '\0';

                error->Release();
            }
            snprintf(log, sizeof(log), "Compile time : %.2f ms", time * 1000.0f);
        }

        ImGui::SameLine();
        ImGui::InputTextMultiline("##target", target, IM_ARRAYSIZE(target), ImVec2(windowSize.x / 2.0f, ImGui::GetTextLineHeight() * 36), ImGuiInputTextFlags_ReadOnly);
        ImGui::TextUnformatted(log);
        ImGui::End();
    }
}
//------------------------------------------------------------------------------
