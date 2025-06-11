#include "IndexBuffer.h"
#include "KamataEngine.h"

#include <d3d12.h>
#include <cassert>

using namespace KamataEngine;

IndexBuffer::IndexBuffer() {
}

IndexBuffer::~IndexBuffer() {
    if (indexBuffer_) {
        indexBuffer_->Release();
        indexBuffer_ = nullptr;
    }
}

void IndexBuffer::Create(const UINT size, const UINT stride) {
    // strideに応じたフォーマットを決定（2 or 4 byte）
    assert(stride == 2 || stride == 4);
    DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    // dxCommon取得
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    // ヒーププロパティ設定
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

    // リソース設定
    D3D12_RESOURCE_DESC indexResourceDesc{};
    indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexResourceDesc.Width = size;
    indexResourceDesc.Height = 1;
    indexResourceDesc.DepthOrArraySize = 1;
    indexResourceDesc.MipLevels = 1;
    indexResourceDesc.SampleDesc.Count = 1;
    indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    // 実際のリソース生成
    ID3D12Resource* indexResource = nullptr;
    HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
        &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexResource)
    );
    assert(SUCCEEDED(hr));

    // メンバに保持
    indexBuffer_ = indexResource;

    // IndexBufferViewの作成
    D3D12_INDEX_BUFFER_VIEW indexBufferView{};
    indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = size;
    indexBufferView.Format = format;

    indexBufferView_ = indexBufferView;
}

ID3D12Resource* IndexBuffer::Get() {
    return indexBuffer_;
}

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() {
    return &indexBufferView_;
}
