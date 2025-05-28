#pragma once

#include <d3dx12.h>  // ID3D12RootSignature を使うため

class RootSignature {
public:
    // 生成（ルートシグネチャを作成する関数）
    void Create();

    // ゲッター（ルートシグネチャへのポインタを取得）
    ID3D12RootSignature* Get();

    // コンストラクタ
    RootSignature();

    // デストラクタ
    ~RootSignature();

private:
    // ルートシグネチャオブジェクトへのポインタ
    ID3D12RootSignature* rootSignature_ = nullptr;
};
