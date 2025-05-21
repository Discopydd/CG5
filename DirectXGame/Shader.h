#pragma once

#include <string>     // for std::wstring, std::string
#include <d3d12.h>    // for ID3DBlob

#include<d3dcompiler.h>
#include<dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include "MiscUtility.h"
class Shader {
public:
    Shader();
    ~Shader();

    // シェーダーファイルを読み込み、コンパイル済みデータを生成する
    void Load(const std::wstring& filePath, const std::wstring& shaderModel);
    void LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel);
    // コンパイル済みデータ（Blob）を取得する
    ID3DBlob* GetBlob();

    IDxcBlob* GetDxcBlob();
private:
    ID3DBlob* blob_ = nullptr;  // C++11以降初期化OK

    IDxcBlob* dxcBlod_ = nullptr;
};
