#pragma once
#include <d3d12.h>  // ID3D12Resource, D3D12_INDEX_BUFFER_VIEW

class IndexBuffer {
public:
    // コンストラクタ
    IndexBuffer();
    // デストラクタ
    ~IndexBuffer();

    // IndexBuffer生成
    void Create(const UINT size, const UINT stride);

    // ゲッター
    ID3D12Resource* Get();                    // インデックスバッファー
    D3D12_INDEX_BUFFER_VIEW* GetView();       // インデックスバッファービュー

private:
    ID3D12Resource* indexBuffer_ = nullptr;               // インデックスバッファー
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};           // インデックスバッファービュー
};
