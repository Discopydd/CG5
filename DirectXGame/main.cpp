#include <Windows.h>
#include"KamataEngine.h"
#include <d3dcompiler.h>
#include "Shader.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace KamataEngine;

//関数プロトタイプ宣言
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);
//RenderTextureResourceの生成
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device,
	uint32_t width, uint32_t height,
	DXGI_FORMAT clearFormat, const FLOAT* clearColor);
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"LE3C_28_リ_ヨン");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(
		std::format("width: {}, height: {}\n", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	ID3D12Device* device = dxCommon->GetDevice();
	//===============================
	// 0. RenderTextureResourceの作成
	//===============================
	const FLOAT kRenderTargetClearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤色
	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(
		device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);
	//===============================
	// RTV用 DescriptorHeap の作成
	//===============================
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 1;
	HRESULT hr = device->CreateDescriptorHeap(
		&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU =
		rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//===============================
	// RTV View の作成
	//===============================
	device->CreateRenderTargetView(
		renderTextureResource,
		nullptr,
		rtvHandleCPU);
	//===============================
	// DepthStencilTextureResourceの作成
	//===============================
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(
		device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	//===============================
	// DSV用のDescriptorHeapの作成
	//===============================
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//===============================
	// DSV用のViewを生成
	//===============================
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);
	//
	//
	//
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;  // SRV
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // GPU
	srvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// Handleの取得
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  // RenderTarget と一致
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(renderTextureResource, &srvDesc, srvHandleCPU);

	// RootSignature作成 ------------------------------
	RootSignature rs;
	rs.Create();

	// 頂点シェーダの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェーダの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	//PipelineState作成 ----------
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vs, ps);
	//VertexBuffer(VertexResource, VertexResourceView)の生成

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};


	VertexBuffer vb;
	// 頂点リソースにデータを書き込む ----------
	VertexData vertices[] = {
		{ { -1.0f,  1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, // 左上
		{ {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }, // 右上
		{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }, // 左下
		{ {  1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }  // 右下
	};
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}
	uint16_t indices[] = { 0, 1, 2, 2, 1, 3 };

	//IndexBuffer(IndexResource, IndexResourceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	//mainループ
	while (true) {
		if (KamataEngine::Update()) {
			break;
		}
		// 描画開始：RTV 用への切替
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrier);


		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);
		// ===========================
		// Viewport 設定
		// ===========================
		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		commandList->RSSetViewports(1, &viewport);

		// ===========================
		// Scissor 設定
		// ===========================
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.bottom = WinApp::kWindowHeight;
		commandList->RSSetScissorRects(1, &scissorRect);

		// ===========================
		// RTV / DSV を設定し、Clear
		// ===========================

		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// 描画開始
		dxCommon->PreDraw();
		// コマンドを積む（设置绘图命令）
		commandList->SetGraphicsRootSignature(rs.Get());  // 设置RootSignature
		commandList->SetPipelineState(pipelineState.Get());  // 设置PSO
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // 设置顶点缓冲视图
		commandList->IASetIndexBuffer(ib.GetView());

		// トポロジの設定（设置图元类型：三角形列表）
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->SetDescriptorHeaps(srvDescriptorHeap->GetDesc().NumDescriptors, &srvDescriptorHeap);

		// ====== SRV テーブルの先頭設定（RootParam[0] = t0）=====
		commandList->SetGraphicsRootDescriptorTable(
			0,           // RootParam[0] = t0
			srvHandleGPU // ヒープから取得した SRV の GPU ハンドル
		);

		// 頂点数、インスタンス数、開始位置（绘制命令）
		//commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);


		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = renderTextureResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrier);
		// 描画終了
		dxCommon->PostDraw();
	}
	// 解放処理
	renderTextureResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();

	depthStencilResource->Release();
	dsvDescriptorHeap->Release();
	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps)
{
	// InputLayout ----------------------------------------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// TEXCOORD
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState ----------------------------------------
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState -----------------------------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Pipeline State Object (PSO) の作成 ------------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;

	graphicsPipelineStateDesc.VS = {
		vs.GetDxcBlob()->GetBufferPointer(),
		vs.GetDxcBlob()->GetBufferSize()
	};

	graphicsPipelineStateDesc.PS = {
		ps.GetDxcBlob()->GetBufferPointer(),
		ps.GetDxcBlob()->GetBufferSize()
	};

	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ（図形の種類）
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// サンプル数など（今回はデフォルトでOK）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 最後に PipelineState クラスの Create を呼んで生成する
	pipelineState.Create(graphicsPipelineStateDesc);
}

ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor)
{
	// 1. RenderTexture の設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);                          // 幅
	resourceDesc.Height = UINT(height);                        // 高さ
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;     // Format
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 2. ヒープ設定（GPUメモリ）
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 3. ClearValue 設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4. リソース作成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,  // 初期状態
		&clearValue,
		IID_PPV_ARGS(&resource)
	);

	assert(SUCCEEDED(hr));

	return resource;
}

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height)
{
	  // 1. Resource 設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.MipLevels = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;  // 深度フォーマット
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // 2. Heap 設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // 3. Clear値 設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;

    // 4. Resource 作成
    ID3D12Resource* resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&resource)
    );
    assert(SUCCEEDED(hr));

    return resource;
}
