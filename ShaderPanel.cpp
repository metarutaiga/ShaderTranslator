//==============================================================================
// ShaderTranslator
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#ifndef _HAS_EXCEPTIONS
#   define _HAS_EXCEPTIONS 0
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include <vector>
#include <d3dcompiler.h>

#include "ShaderSample.h"
#include "ShaderPanel.h"

struct ExternalCompiler
{
    std::string profile;
    const char* sample;
    std::string compile;
    std::string disassemble;
};

static std::string folder;
static std::vector<ExternalCompiler> externalCompilers;
static char version[32];
static char source[65536];
static char target[65536];
static char logger[256];

//------------------------------------------------------------------------------
void ShaderPanelInitialize(const char* folder)
{
    char filename[MAX_PATH];
    snprintf(filename, MAX_PATH, "%s/tools/ExternalShaderCompiler.ini", folder);

    FILE* file = fopen(filename, "r");
    if (file)
    {
        char line[1024];
        while (fgets(line, 1024, file))
        {
            if (char* ln = strchr(line, '\n'))
                ln[0] = '\0';

            char* profile = strtok(line, ",");
            char* sample = strtok(nullptr, ",");
            char* compile = strtok(nullptr, ",");
            char* disassemble = strtok(nullptr, ",");

            ExternalCompiler externalCompiler;
            externalCompiler.profile = profile ? profile : "";
            switch (atoi(sample ? sample : ""))
            {
            case 00: externalCompiler.sample = sampleHLSLVertexShaderLegacy; break;
            case 01: externalCompiler.sample = sampleHLSLVertexShader;       break;
            case 10: externalCompiler.sample = sampleHLSLPixelShaderLegacy;  break;
            case 11: externalCompiler.sample = sampleHLSLPixelShader;        break;
            }
            externalCompiler.compile = compile ? compile : "";
            externalCompiler.disassemble = disassemble ? disassemble : "";
            externalCompilers.emplace_back(externalCompiler);
        }

        fclose(file);
    }

    snprintf(filename, MAX_PATH, "%s/tools", folder);
    ::folder = filename;
}
//------------------------------------------------------------------------------
void ShaderPanelShutdown()
{
    externalCompilers.clear();
}
//------------------------------------------------------------------------------
static void HLSLCompile()
{
    static HMODULE D3DCompiler = LoadLibraryA(D3DCOMPILER_DLL_A);
    static pD3DCompile D3DCompile = (pD3DCompile)GetProcAddress(D3DCompiler, "D3DCompile");
    static pD3DDisassemble D3DDisassemble = (pD3DDisassemble)GetProcAddress(D3DCompiler, "D3DDisassemble");

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
    snprintf(logger, sizeof(logger), "Compile time : %.2f ms", time * 1000.0f);
}
//------------------------------------------------------------------------------
static void ExternalCompile(const ExternalCompiler& compiler)
{
    char temp[MAX_PATH];
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    FILE* file = nullptr;

    xxLocalBreak()
    {
        snprintf(temp, MAX_PATH, "%s/temp.txt", folder.c_str());
        file = fopen(temp, "w");
        if (file == nullptr)
            break;
        fwrite(source, 1, strlen(source), file);
        fclose(file);

        snprintf(temp, MAX_PATH, compiler.compile.c_str(), folder.c_str(), folder.c_str(), folder.c_str());
        float time = xxGetCurrentTime();
        if (CreateProcessA(nullptr, temp, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi) == FALSE)
            break;

        WaitForSingleObject(pi.hProcess, INFINITE);
        time = xxGetCurrentTime() - time;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        snprintf(temp, MAX_PATH, compiler.disassemble.c_str(), folder.c_str(), folder.c_str(), folder.c_str());
        if (CreateProcessA(nullptr, temp, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi) == FALSE)
            break;

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        snprintf(temp, MAX_PATH, "%s/temp.txt", folder.c_str());
        file = fopen(temp, "r");
        if (file == nullptr)
            break;
        size_t length = fread(target, 1, sizeof(target), file);
        fclose(file);
        target[length] = '\0';

        snprintf(logger, sizeof(logger), "Compile time : %.2f ms", time * 1000.0f);
    }

    snprintf(temp, MAX_PATH, "%s/temp.txt", folder.c_str());
    remove(temp);
    snprintf(temp, MAX_PATH, "%s/temp.bin", folder.c_str());
    remove(temp);
}
//------------------------------------------------------------------------------
void ShaderPanel(const UpdateData& updateData, bool& show)
{
    ImGui::SetNextWindowSize(ImVec2(1200 * updateData.windowScale, 600 * updateData.windowScale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Shader Panel", &show))
    {
        ImVec2 windowSize = ImGui::GetWindowSize();
        bool hlslCompile = false;
        ExternalCompiler* externalCompiler = nullptr;

        // HLSL
        static struct { const char* const version; const char* const code; } hlsl[] =
        {
            { "vs_1_1", sampleHLSLVertexShaderLegacy },
            { "vs_2_0", sampleHLSLVertexShaderLegacy },
            { "vs_3_0", sampleHLSLVertexShader },
            { "vs_4_0", sampleHLSLVertexShader },
            { "vs_5_0", sampleHLSLVertexShader },
            { "ps_2_0", sampleHLSLPixelShaderLegacy },
            { "ps_3_0", sampleHLSLPixelShaderLegacy },
            { "ps_4_0", sampleHLSLPixelShader },
            { "ps_5_0", sampleHLSLPixelShader },
        };
        for (size_t i = 0; i < xxCountOf(hlsl); ++i)
        {
            if (ImGui::Button(hlsl[i].version))
            {
                snprintf(version, sizeof(version), hlsl[i].version);
                snprintf(source, sizeof(source), "%s", hlsl[i].code);
                hlslCompile = true;
            }
            ImGui::SameLine();
        }

        // External Compiler
        for (size_t i = 0; i < externalCompilers.size(); ++i)
        {
            if (ImGui::Button(externalCompilers[i].profile.c_str()))
            {
                snprintf(version, sizeof(version), externalCompilers[i].profile.c_str());
                snprintf(source, sizeof(source), "%s", externalCompilers[i].sample);
                externalCompiler = &externalCompilers[i];
            }
            ImGui::SameLine();
        }

        ImGui::NewLine();
        if (ImGui::InputTextMultiline("##source", source, IM_ARRAYSIZE(source), ImVec2(windowSize.x / 2.0f, ImGui::GetTextLineHeight() * 36)))
        {
            hlslCompile = true;
        }

        if (hlslCompile)
        {
            HLSLCompile();
        }

        if (externalCompiler)
        {
            ExternalCompile(*externalCompiler);
        }

        ImGui::SameLine();
        ImGui::InputTextMultiline("##target", target, IM_ARRAYSIZE(target), ImVec2(windowSize.x / 2.0f, ImGui::GetTextLineHeight() * 36), ImGuiInputTextFlags_ReadOnly);
        ImGui::TextUnformatted(logger);
        ImGui::End();
    }
}
//------------------------------------------------------------------------------
