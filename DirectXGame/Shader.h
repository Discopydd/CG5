#pragma once

#include <string>     // for std::wstring, std::string
#include <d3d12.h>    // for ID3DBlob

class Shader {
public:
    Shader();
    ~Shader();

    // シェーダーファイルを読み込み、コンパイル済みデータを生成する
    void Load(const std::wstring& filePath, const std::string& shaderModel);

    // コンパイル済みデータ（Blob）を取得する
    ID3DBlob* GetBlob();

private:
    ID3DBlob* blob_ = nullptr;  // C++11以降初期化OK
};
