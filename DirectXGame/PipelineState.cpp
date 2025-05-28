#include "PipelineState.h"
#include "KamataEngine.h"  // DirectXCommon を使うため

using namespace KamataEngine;

void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc) {
    // DirectXCommon シングルトンを取得
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    // 一時的なパイプラインステート用ポインタ
    ID3D12PipelineState* graphicsPipelineState = nullptr;

    // パイプラインステートを作成
    HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&graphicsPipelineState)
    );
    assert(SUCCEEDED(hr));

    // 作成したものをメンバに保持
    pipelineState_ = graphicsPipelineState;
}

ID3D12PipelineState* PipelineState::Get() {
    return pipelineState_;
}

PipelineState::PipelineState() {
}

PipelineState::~PipelineState() {
    if (pipelineState_) {
        pipelineState_->Release();
        pipelineState_ = nullptr;
    }
}
