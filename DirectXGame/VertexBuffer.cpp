#include "VertexBuffer.h"
#include "KamataEngine.h"

#include <d3dx12.h>  // ID3D12Resource, D3D12_VERTEX_BUFFER_VIEW
#include <cassert>   // assert

using namespace KamataEngine;

// 頂点バッファ生成
void VertexBuffer::Create(const UINT size, const UINT stride) {
    // DirectXCommon を取得
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    // ヒープ設定（UPLOAD = CPU から書き込み可能）
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    // リソース設定（バッファ）
    D3D12_RESOURCE_DESC vertexResourceDesc{};
    vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexResourceDesc.Width = size;  // 引数から受け取る全体サイズ
    vertexResourceDesc.Height = 1;
    vertexResourceDesc.DepthOrArraySize = 1;
    vertexResourceDesc.MipLevels = 1;
    vertexResourceDesc.SampleDesc.Count = 1;
    vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    // 頂点リソースを作成
    ID3D12Resource* vertexResource = nullptr;
    HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &vertexResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexResource)
    );
    assert(SUCCEEDED(hr));

    // 作成したリソースをメンバ変数に保存
    vertexBuffer_ = vertexResource;

    // 頂点バッファビューを構築
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = size;
    vertexBufferView.StrideInBytes = stride;

    vertexBufferView_ = vertexBufferView;
}

// 頂点バッファを取得
ID3D12Resource* VertexBuffer::Get() {
    return vertexBuffer_;
}

// 頂点バッファビューを取得
D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::GetView() {
    return &vertexBufferView_;
}

// コンストラクタ
VertexBuffer::VertexBuffer() {}

// デストラクタ（リソース解放）
VertexBuffer::~VertexBuffer() {
    if (vertexBuffer_) {
        vertexBuffer_->Release();
        vertexBuffer_ = nullptr;
    }
}
