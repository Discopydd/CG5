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

     // ===== Descriptor Range（t0 を使う SRV）=====
    D3D12_DESCRIPTOR_RANGE srvDescRange[1]{};
    srvDescRange[0].BaseShaderRegister = 0; // t0
    srvDescRange[0].NumDescriptors = 1;
    srvDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ===== Root Parameter（PixelShader 用）=====
    D3D12_ROOT_PARAMETER rootParameters[1]{};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = srvDescRange;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(srvDescRange);

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    // ===== Sampler の設定（s0）=====
    D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0; // s0
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


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
