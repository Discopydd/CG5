#include "Shader.h"
#include <d3dcompiler.h>
#include <cassert>
#include <Windows.h>

Shader::Shader() {
}

Shader::~Shader() {
    if (blob_ != nullptr) {
        blob_->Release();
        blob_ = nullptr;
    }
     if (dxcBlod_ != nullptr) {
        dxcBlod_->Release();
        dxcBlod_ = nullptr;
    }
}

void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    // wstring => string 変換
    std::string mbShaderModel = ConvertString(shaderModel);

    HRESULT hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", mbShaderModel.c_str(),
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &shaderBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        assert(false);
    }

    blob_ = shaderBlob;
}

void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel)
{
    // DXC（DirectX Shader Compiler）を初期化
static IDxcUtils* dxcUtils = nullptr;
static IDxcCompiler3* dxcCompiler = nullptr;
static IDxcIncludeHandler* includeHandler = nullptr;

HRESULT hr;
  // DXC の初期化
    if (dxcUtils == nullptr) {
        hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
        assert(SUCCEEDED(hr));
    }

    if (dxcCompiler == nullptr) {
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
        assert(SUCCEEDED(hr));
    }

    if (includeHandler == nullptr) {
        hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
        assert(SUCCEEDED(hr));
    }

    // 1. hlslファイルを読み込む
    IDxcBlobEncoding* shaderSource = nullptr;
    hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
    assert(SUCCEEDED(hr));

    // 2. 読み込んだファイルの内容を DxcBuffer に設定
    DxcBuffer shaderSourceBuffer{};
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;
      // 3. コンパイルオプション設定
    LPCWSTR arguments[] = {
        filePath.c_str(),
        L"-E", L"main",             // エントリーポイント
        L"-T", shaderModel.c_str(),// ターゲットプロファイル
        L"-Zi",                     // デバッグ情報
        L"-Qembed_debug",          // デバッグ埋め込み
        L"-Od",                     // 最適化無効
        L"-Zpr"                     // 行優先のメモリレイアウト
    };

    // 4. コンパイル
    IDxcResult* shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,
        arguments,
        _countof(arguments),
        includeHandler,
        IID_PPV_ARGS(&shaderResult)
    );
    assert(SUCCEEDED(hr));

    // 5. エラー出力確認
    IDxcBlobUtf8* shaderError = nullptr;
    IDxcBlobWide* nameBlob = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), &nameBlob);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        OutputDebugStringA(reinterpret_cast<char*>(shaderError->GetBufferPointer()));
        assert(false);
    }

    // 6. 成功時のバイナリ取得
    IDxcBlob* shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &nameBlob);
    assert(SUCCEEDED(hr));

    shaderSource->Release();
    shaderResult->Release();

    dxcBlod_ = shaderBlob;
}

ID3DBlob* Shader::GetBlob() {
    return blob_;
}

IDxcBlob* Shader::GetDxcBlob()
{
    return dxcBlod_;
}
