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
	};


	VertexBuffer vb;
	vb.Create(sizeof(Vector4) * 3, sizeof(Vector4));
	// 頂点リソースにデータを書き込む ----------
	VertexData vertices[] = {
	{ -1.0f,  1.0f, 0.0f, 1.0f }, // 左上
    {  1.0f,  1.0f, 0.0f, 1.0f }, // 右上
    { -1.0f, -1.0f, 0.0f, 1.0f }, // 左下
    {  1.0f, -1.0f, 0.0f, 1.0f }  // 右下
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

		// 描画開始
		dxCommon->PreDraw();
		// コマンドを積む（设置绘图命令）
		commandList->SetGraphicsRootSignature(rs.Get());  // 设置RootSignature
		commandList->SetPipelineState(pipelineState.Get());  // 设置PSO
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // 设置顶点缓冲视图
		commandList->IASetIndexBuffer(ib.GetView());

		// トポロジの設定（设置图元类型：三角形列表）
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 頂点数、インスタンス数、開始位置（绘制命令）
		//commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
		// 描画終了
		dxCommon->PostDraw();
	}
	// 解放処理

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps)
{
	// InputLayout ----------------------------------------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

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
