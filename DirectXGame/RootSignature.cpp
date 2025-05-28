#include "RootSignature.h"
#include "KamataEngine.h"  // DirectXCommon を含む

using namespace KamataEngine;

void RootSignature::Create() {
    // すでに存在しているなら解放
    if (rootSignature_) {
        rootSignature_->Release();
        rootSignature_ = nullptr;
    }

    // DirectXCommon を取得
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    // RootSignature の構造体を用意
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    // シリアライズ（バイナリ化）
    HRESULT hr = D3D12SerializeRootSignature(
        &descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    // シリアライズ結果から RootSignature を生成
    ID3D12RootSignature* rootSignature = nullptr;
    hr = dxCommon->GetDevice()->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    );
    assert(SUCCEEDED(hr));

    // signatureBlob はもう不要
    signatureBlob->Release();

    // メンバに保存
    rootSignature_ = rootSignature;
}

// RootSignature を返す
ID3D12RootSignature* RootSignature::Get() {
    return rootSignature_;
}

// コンストラクタ
RootSignature::RootSignature() {}

// デストラクタ（リソース解放）
RootSignature::~RootSignature() {
    if (rootSignature_) {
        rootSignature_->Release();
        rootSignature_ = nullptr;
    }
}
