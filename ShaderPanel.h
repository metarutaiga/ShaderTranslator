//==============================================================================
// ShaderTranslator
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include <interface.h>

void ShaderPanelInitialize(const char* folder);
void ShaderPanelShutdown();
void ShaderPanel(const UpdateData& updateData, bool& show);
