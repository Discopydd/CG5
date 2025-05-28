#pragma once

#include <d3dx12.h>  // ID3D12Resource, D3D12_VERTEX_BUFFER_VIEW を使う

class VertexBuffer {
public:
    // コンストラクタ
    VertexBuffer();

    // デストラクタ
    ~VertexBuffer();

    // VertexBuffer を生成
    void Create(const UINT size, const UINT stride);

    // 頂点バッファ取得
    ID3D12Resource* Get();

    // 頂点バッファビュー取得
    D3D12_VERTEX_BUFFER_VIEW* GetView();

private:
    // 頂点バッファ
    ID3D12Resource* vertexBuffer_ = nullptr;

    // 頂点バッファビュー
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};
