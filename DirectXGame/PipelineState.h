#pragma once

#include <d3dx12.h>  // ID3D12PipelineState を使うため

class PipelineState {
public:
    // 初期化（Create関数）
    void Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);

    // ゲッター（ポインタを取得）
    ID3D12PipelineState* Get();

    // コンストラクタ
    PipelineState();

    // デストラクタ
    ~PipelineState();

private:
    // パイプラインステートオブジェクトへのポインタ
    ID3D12PipelineState* pipelineState_ = nullptr;
};
