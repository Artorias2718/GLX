// Header Files
//=============

#include "../Graphics.h"
#include "../sVertex.h"

#include <cstddef>
#include <cstdint>
#include "Includes.h"
#include "../../Asserts/Asserts.h"
#include "../../Logging/Logging.h"
#include "../../Time/Time.h"

// Static Data Initialization
//===========================

namespace
{
	// This is the main window handle from Windows
	HWND s_renderingWindow = NULL;
	// These are D3D interfaces
	ID3D11Device* s_direct3dDevice = NULL;
	IDXGISwapChain* s_swapChain = NULL;
	ID3D11DeviceContext* s_direct3dImmediateContext = NULL;
	ID3D11RenderTargetView* s_renderTargetView = NULL;

	// D3D has an "input layout" object that associates the layout of the struct above
	// with the input from a vertex shader
	ID3D11InputLayout* s_vertexLayout = NULL;

	// The vertex buffer holds the data for each vertex
	ID3D11Buffer* s_vertexBuffer = NULL;

	// The vertex shader is a program that operates on vertices.
	// Its input comes from a C/C++ "draw call" and is:
	//	* Position
	//	* Any other data we want
	// Its output is:
	//	* Position
	//		(So that the graphics hardware knows which pixels to fill in for the triangle)
	//	* Any other data we want
	ID3D11VertexShader* s_vertexShader = NULL;
	// The fragment shader is a program that operates on fragments
	// (or potential pixels).
	// Its input is:
	//	* The data that was output from the vertex shader,
	//		interpolated based on how close the fragment is
	//		to each vertex in the triangle.
	// Its output is:
	//	* The final color that the pixel should be
	ID3D11PixelShader* s_fragmentShader = NULL;

	// This struct determines the layout of the constant data that the CPU will send to the GPU
	struct
	{
		union
		{
			float g_elapsedSecondCount_total;
			float register0[4];	// You won't have to worry about why I do this until a later assignment
		};
	} s_constantBufferData;
	ID3D11Buffer* s_constantBuffer = NULL;
}

// Helper Function Declarations
//=============================

namespace
{
	bool CreateConstantBuffer();
	bool CreateDevice(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
	bool CreateVertexBuffer(ID3D10Blob& i_compiledShader);
	bool CreateView(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
	bool LoadFragmentShader();
	bool LoadVertexShader(ID3D10Blob*& o_compiledShader);
}

// Interface
//==========

// Render
//-------

void Engine::Graphics::RenderFrame()
{
	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		// Black is usually used
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		s_direct3dImmediateContext->ClearRenderTargetView(s_renderTargetView, clearColor);
	}

	// Update the constant buffer
	{
		// Update the struct (i.e. the memory that we own)
		s_constantBufferData.g_elapsedSecondCount_total = Time::GetElapsedSecondCount_total();
		// Get a pointer from Direct3D that can be written to
		void* memoryToWriteTo = NULL;
		{
			D3D11_MAPPED_SUBRESOURCE mappedSubResource;
			{
				// Discard previous contents when writing
				const unsigned int noSubResources = 0;
				const D3D11_MAP mapType = D3D11_MAP_WRITE_DISCARD;
				const unsigned int noFlags = 0;
				const HRESULT result = s_direct3dImmediateContext->Map(s_constantBuffer, noSubResources, mapType, noFlags, &mappedSubResource);
				if (SUCCEEDED(result))
				{
					memoryToWriteTo = mappedSubResource.pData;
				}
				else
				{
					ASSERT(false);
				}
			}
		}
		if (memoryToWriteTo)
		{
			// Copy the new data to the memory that Direct3D has provided
			memcpy(memoryToWriteTo, &s_constantBufferData, sizeof(s_constantBufferData));
			// Let Direct3D know that the memory contains the data
			// (the pointer will be invalid after this call)
			const unsigned int noSubResources = 0;
			s_direct3dImmediateContext->Unmap(s_constantBuffer, noSubResources);
			memoryToWriteTo = NULL;
		}
		// Bind the constant buffer to the shader
		{
			const unsigned int registerAssignedInShader = 0;
			const unsigned int bufferCount = 1;
			s_direct3dImmediateContext->VSSetConstantBuffers(registerAssignedInShader, bufferCount, &s_constantBuffer);
			s_direct3dImmediateContext->PSSetConstantBuffers(registerAssignedInShader, bufferCount, &s_constantBuffer);
		}
	}

	// Draw the geometry
	{
		// Set the vertex and fragment shaders
		{
			ID3D11ClassInstance** const noInterfaces = NULL;
			const unsigned int interfaceCount = 0;
			s_direct3dImmediateContext->VSSetShader(s_vertexShader, noInterfaces, interfaceCount);
			s_direct3dImmediateContext->PSSetShader(s_fragmentShader, noInterfaces, interfaceCount);
		}
		// Bind a specific vertex buffer to the device as a data source
		{
			const unsigned int startingSlot = 0;
			const unsigned int vertexBufferCount = 1;
			// The "stride" defines how large a single vertex is in the stream of data
			const unsigned int bufferStride = sizeof(sVertex);
			// It's possible to start streaming data in the middle of a vertex buffer
			const unsigned int bufferOffset = 0;
			s_direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &s_vertexBuffer, &bufferStride, &bufferOffset);
		}
		// Specify what kind of data the vertex buffer holds
		{
			// Set the layout (which defines how to interpret a single vertex)
			s_direct3dImmediateContext->IASetInputLayout(s_vertexLayout);
			// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
			// we have defined the vertex buffer as a triangle list
			// (meaning that every primitive is a triangle and will be defined by three vertices)
			s_direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		// Render triangles from the currently-bound vertex buffer
		{
			// As of this comment we are only drawing a single triangle
			// (you will have to update this code in future assignments!)
			const unsigned int triangleCount = 1;
			const unsigned int vertexCountPerTriangle = 3;
			const unsigned int vertexCountToRender = triangleCount * vertexCountPerTriangle;
			// It's possible to start rendering primitives in the middle of the stream
			const unsigned int indexOfFirstVertexToRender = 0;
			s_direct3dImmediateContext->Draw(vertexCountToRender, indexOfFirstVertexToRender);
		}
	}

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it the contents of the back buffer must be "presented"
	// (to the front buffer)
	{
		const unsigned int swapImmediately = 0;
		const unsigned int presentNextFrame = 0;
		const HRESULT result = s_swapChain->Present(swapImmediately, presentNextFrame);
		ASSERT(SUCCEEDED(result));
	}
}

// Initialization / Clean Up
//--------------------------

bool Engine::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
{
	bool wereThereErrors = false;

	s_renderingWindow = i_initializationParameters.mainWindow;
	ID3D10Blob* compiledVertexShader = NULL;

	// Create an interface to a Direct3D device
	if (!CreateDevice(i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight))
	{
		wereThereErrors = true;
		goto OnExit;
	}
	// Initialize the viewport of the device
	if (!CreateView(i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight))
	{
		wereThereErrors = true;
		goto OnExit;
	}

	// Initialize the graphics objects
	if (!LoadVertexShader(compiledVertexShader))
	{
		wereThereErrors = true;
		goto OnExit;
	}
	if (!CreateVertexBuffer(*compiledVertexShader))
	{
		wereThereErrors = true;
		goto OnExit;
	}
	if (!LoadFragmentShader())
	{
		wereThereErrors = true;
		goto OnExit;
	}
	if (!CreateConstantBuffer())
	{
		wereThereErrors = true;
		goto OnExit;
	}

OnExit:

	// A vertex shader object is used to render the triangle.
	// The compiled vertex shader is the actual compiled code,
	// and once it has been used to create the vertex input layout
	// it can be freed.
	if (compiledVertexShader)
	{
		compiledVertexShader->Release();
		compiledVertexShader = NULL;
	}

	return !wereThereErrors;
}

bool Engine::Graphics::CleanUp()
{
	bool wereThereErrors = false;

	if (s_direct3dDevice)
	{
		if (s_vertexLayout)
		{
			s_vertexLayout->Release();
			s_vertexLayout = NULL;
		}
		if (s_vertexBuffer)
		{
			s_vertexBuffer->Release();
			s_vertexBuffer = NULL;
		}

		if (s_vertexShader)
		{
			s_vertexShader->Release();
			s_vertexShader = NULL;
		}
		if (s_fragmentShader)
		{
			s_fragmentShader->Release();
			s_fragmentShader = NULL;
		}

		if (s_constantBuffer)
		{
			s_constantBuffer->Release();
			s_constantBuffer = NULL;
		}

		if (s_renderTargetView)
		{
			s_renderTargetView->Release();
			s_renderTargetView = NULL;
		}

		s_direct3dDevice->Release();
		s_direct3dDevice = NULL;
	}
	if (s_direct3dImmediateContext)
	{
		s_direct3dImmediateContext->Release();
		s_direct3dImmediateContext = NULL;
	}
	if (s_swapChain)
	{
		s_swapChain->Release();
		s_swapChain = NULL;
	}

	s_renderingWindow = NULL;

	return !wereThereErrors;
}

// Helper Function Definitions
//============================

namespace
{
	bool CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC bufferDescription = { 0 };
		{
			// The byte width must be rounded up to a multiple of 16
			bufferDescription.ByteWidth = sizeof(s_constantBufferData);
			bufferDescription.Usage = D3D11_USAGE_DYNAMIC;	// The CPU must be able to update the buffer
			bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// The CPU must write, but doesn't read
			bufferDescription.MiscFlags = 0;
			bufferDescription.StructureByteStride = 0;	// Not used
		}
		D3D11_SUBRESOURCE_DATA initialData = { 0 };
		{
			// Fill in the constant buffer
			s_constantBufferData.g_elapsedSecondCount_total = Engine::Time::GetElapsedSecondCount_total();
			initialData.pSysMem = &s_constantBufferData;
			// (The other data members are ignored for non-texture buffers)
		}

		const HRESULT result = s_direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &s_constantBuffer);
		if (SUCCEEDED(result))
		{
			return true;
		}
		else
		{
			ASSERT(false);
			Engine::Logging::OutputError("Direct3D failed to create a constant buffer with HRESULT %#010x", result);
			return false;
		}
	}

	bool CreateDevice(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight)
	{
		IDXGIAdapter* const useDefaultAdapter = NULL;
		const D3D_DRIVER_TYPE useHardwareRendering = D3D_DRIVER_TYPE_HARDWARE;
		const HMODULE dontUseSoftwareRendering = NULL;
		unsigned int flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
		{
#ifdef GRAPHICS_ISDEVICEDEBUGINFOENABLED
			flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		}
		D3D_FEATURE_LEVEL* const useDefaultFeatureLevels = NULL;
		const unsigned int requestedFeatureLevelCount = 0;
		const unsigned int sdkVersion = D3D11_SDK_VERSION;
		DXGI_SWAP_CHAIN_DESC swapChainDescription = { 0 };
		{
			{
				DXGI_MODE_DESC& bufferDescription = swapChainDescription.BufferDesc;

				bufferDescription.Width = i_resolutionWidth;
				bufferDescription.Height = i_resolutionHeight;
				{
					DXGI_RATIONAL& refreshRate = bufferDescription.RefreshRate;

					refreshRate.Numerator = 0;	// Refresh as fast as possible
					refreshRate.Denominator = 1;
				}
				bufferDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				bufferDescription.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				bufferDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			}
			{
				DXGI_SAMPLE_DESC& multiSamplingDescription = swapChainDescription.SampleDesc;

				multiSamplingDescription.Count = 1;
				multiSamplingDescription.Quality = 0;	// Anti-aliasing is disabled
			}
			swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDescription.BufferCount = 1;
			swapChainDescription.OutputWindow = s_renderingWindow;
			swapChainDescription.Windowed = TRUE;
			swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDescription.Flags = 0;
		}
		D3D_FEATURE_LEVEL highestSupportedFeatureLevel;
		const HRESULT result = D3D11CreateDeviceAndSwapChain(useDefaultAdapter, useHardwareRendering, dontUseSoftwareRendering,
			flags, useDefaultFeatureLevels, requestedFeatureLevelCount, sdkVersion, &swapChainDescription,
			&s_swapChain, &s_direct3dDevice, &highestSupportedFeatureLevel, &s_direct3dImmediateContext);
		if (SUCCEEDED(result))
		{
			return true;
		}
		else
		{
			ASSERT(false);
			Engine::Logging::OutputError("Direct3D failed to create a Direct3D11 device with HRESULT %#010x", result);
			return false;
		}
	}

	bool CreateVertexBuffer(ID3D10Blob& i_compiledShader)
	{
		// Create the vertex layout
		{
			// These elements must match the VertexFormat::sVertex layout struct exactly.
			// They instruct Direct3D how to match the binary data in the vertex buffer
			// to the input elements in a vertex shader
			// (by using so-called "semantic" names so that, for example,
			// "POSITION" here matches with "POSITION" in shader code).
			// Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
			const unsigned int vertexElementCount = 1;
			D3D11_INPUT_ELEMENT_DESC layoutDescription[vertexElementCount] = { 0 };
			{
				// POSITION
				// 2 floats == 8 bytes
				// Offset = 0
				{
					D3D11_INPUT_ELEMENT_DESC& positionElement = layoutDescription[0];

					positionElement.SemanticName = "POSITION";
					positionElement.SemanticIndex = 0;	// (Semantics without modifying indices at the end can always use zero)
					positionElement.Format = DXGI_FORMAT_R32G32_FLOAT;
					positionElement.InputSlot = 0;
					positionElement.AlignedByteOffset = offsetof(Engine::Graphics::sVertex, position.x);
					positionElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					positionElement.InstanceDataStepRate = 0;	// (Must be zero for per-vertex data)
				}
			}

			const HRESULT result = s_direct3dDevice->CreateInputLayout(layoutDescription, vertexElementCount,
				i_compiledShader.GetBufferPointer(), i_compiledShader.GetBufferSize(), &s_vertexLayout);
			if (FAILED(result))
			{
				ASSERT(false);
				Engine::Logging::OutputError("Direct3D failed to create a vertex input layout with HRESULT %#010x", result);
				return false;
			}
		}

		// Eventually the vertex data should come from a file but for now it is hard-coded here.
		// You will have to update this in a future assignment
		// (one of the most common mistakes in the class is to leave hard-coded values here).

		const unsigned int triangleCount = 1;
		const unsigned int vertexCountPerTriangle = 3;
		const unsigned int vertexCount = triangleCount * vertexCountPerTriangle;
		const unsigned int bufferSize = vertexCount * sizeof(Engine::Graphics::sVertex);
		Engine::Graphics::sVertex vertexData[vertexCount];
		{
			vertexData[0].position.x = -0.125f;
			vertexData[0].position.y = -0.125f;

			vertexData[1].position.x = 0.125f;
			vertexData[1].position.y = 0.125f;

			vertexData[2].position.x = 0.125f;
			vertexData[2].position.y = -0.125f;
		}

		D3D11_BUFFER_DESC bufferDescription = { 0 };
		{
			bufferDescription.ByteWidth = bufferSize;
			bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
			bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
			bufferDescription.MiscFlags = 0;
			bufferDescription.StructureByteStride = 0;	// Not used
		}
		D3D11_SUBRESOURCE_DATA initialData = { 0 };
		{
			initialData.pSysMem = vertexData;
			// (The other data members are ignored for non-texture buffers)
		}

		const HRESULT result = s_direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &s_vertexBuffer);
		if (FAILED(result))
		{
			ASSERT(false);
			Engine::Logging::OutputError("Direct3D failed to create the vertex buffer with HRESULT %#010x", result);
			return false;
		}

		return true;
	}

	bool CreateView(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight)
	{
		bool wereThereErrors = false;

		// Create and bind the render target view
		ID3D11Texture2D* backBuffer = NULL;
		{
			// Get the back buffer from the swap chain
			HRESULT result;
			{
				const unsigned int bufferIndex = 0;	// This must be 0 since the swap chain is discarded
				result = s_swapChain->GetBuffer(bufferIndex, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
				if (FAILED(result))
				{
					ASSERT(false);
					Engine::Logging::OutputError("Direct3D failed to get the back buffer from the swap chain with HRESULT %#010x", result);
					goto OnExit;
				}
			}
			// Create the view
			{
				const D3D11_RENDER_TARGET_VIEW_DESC* const accessAllSubResources = NULL;
				result = s_direct3dDevice->CreateRenderTargetView(backBuffer, accessAllSubResources, &s_renderTargetView);
			}
			if (SUCCEEDED(result))
			{
				// Bind it
				const unsigned int renderTargetCount = 1;
				ID3D11DepthStencilView* const noDepthStencilState = NULL;
				s_direct3dImmediateContext->OMSetRenderTargets(renderTargetCount, &s_renderTargetView, noDepthStencilState);
			}
			else
			{
				ASSERT(false);
				Engine::Logging::OutputError("Direct3D failed to create the render target view with HRESULT %#010x", result);
				goto OnExit;
			}
		}

		// Specify that the entire render target should be visible
		{
			D3D11_VIEWPORT viewPort = { 0 };
			viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;
			viewPort.Width = static_cast<float>(i_resolutionWidth);
			viewPort.Height = static_cast<float>(i_resolutionHeight);
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;
			const unsigned int viewPortCount = 1;
			s_direct3dImmediateContext->RSSetViewports(viewPortCount, &viewPort);
		}

	OnExit:

		if (backBuffer)
		{
			backBuffer->Release();
			backBuffer = NULL;
		}

		return !wereThereErrors;
	}

	bool LoadFragmentShader()
	{
		// Load the source code and compile it
		ID3D10Blob* compiledShader = NULL;
		{
			const char* const sourceCodeFileName = "data/shaders/fragment.hlsl";
			D3D10_SHADER_MACRO* const noMacros = NULL;
			ID3DInclude* const noIncludes = NULL;
			const char* const entryPoint = "main";
			const char* const profile = "ps_4_0";
			const unsigned int noFlags = 0;
			ID3DX11ThreadPump* const blockUntilLoaded = NULL;
			ID3D10Blob* errorMessages = NULL;
			HRESULT result;
			result = D3DX11CompileFromFile(sourceCodeFileName, noMacros, noIncludes, entryPoint, profile,
				noFlags, noFlags, blockUntilLoaded, &compiledShader, &errorMessages, &result);
			if (SUCCEEDED(result))
			{
				if (errorMessages)
				{
					errorMessages->Release();
				}
			}
			else
			{
				if (errorMessages)
				{
					ASSERTF(false, reinterpret_cast<char*>(errorMessages->GetBufferPointer()));
					Engine::Logging::OutputError("Direct3D failed to compile the fragment shader from the file %s: %s",
						sourceCodeFileName, reinterpret_cast<char*>(errorMessages->GetBufferPointer()));
					errorMessages->Release();
				}
				else
				{
					ASSERT(false);
					Engine::Logging::OutputError("Direct3D failed to compile the fragment shader from the file %s",
						sourceCodeFileName);
				}
				return false;
			}
		}
		// Create the fragment shader object
		bool wereThereErrors = false;
		{
			ID3D11ClassLinkage* const noInterfaces = NULL;
			const HRESULT result = s_direct3dDevice->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
				noInterfaces, &s_fragmentShader);
			if (FAILED(result))
			{
				ASSERT(false);
				Engine::Logging::OutputError("Direct3D failed to create the fragment shader with HRESULT %#010x", result);
				wereThereErrors = true;
			}
			compiledShader->Release();
		}
		return !wereThereErrors;
	}

	bool LoadVertexShader(ID3D10Blob*& o_compiledShader)
	{
		// Load the source code and compile it
		{
			const char* const sourceCodeFileName = "data/shaders/vertex.hlsl";
			D3D10_SHADER_MACRO* const noMacros = NULL;
			ID3DInclude* const noIncludes = NULL;
			const char* const entryPoint = "main";
			const char* const profile = "vs_4_0";
			const unsigned int noFlags = 0;
			ID3DX11ThreadPump* const blockUntilLoaded = NULL;
			ID3D10Blob* errorMessages = NULL;
			HRESULT result;
			result = D3DX11CompileFromFile(sourceCodeFileName, noMacros, noIncludes, entryPoint, profile,
				noFlags, noFlags, blockUntilLoaded, &o_compiledShader, &errorMessages, &result);
			if (SUCCEEDED(result))
			{
				if (errorMessages)
				{
					errorMessages->Release();
				}
			}
			else
			{
				if (errorMessages)
				{
					ASSERTF(false, reinterpret_cast<char*>(errorMessages->GetBufferPointer()));
					Engine::Logging::OutputError("Direct3D failed to compile the vertex shader from the file %s: %s",
						sourceCodeFileName, reinterpret_cast<char*>(errorMessages->GetBufferPointer()));
					errorMessages->Release();
				}
				else
				{
					ASSERT(false);
					Engine::Logging::OutputError("Direct3D failed to compile the vertex shader from the file %s",
						sourceCodeFileName);
				}
				return false;
			}
		}
		// Create the vertex shader object
		bool wereThereErrors = false;
		{
			ID3D11ClassLinkage* const noInterfaces = NULL;
			const HRESULT result = s_direct3dDevice->CreateVertexShader(o_compiledShader->GetBufferPointer(), o_compiledShader->GetBufferSize(),
				noInterfaces, &s_vertexShader);
			if (FAILED(result))
			{
				ASSERT(false);
				Engine::Logging::OutputError("Direct3D failed to create the vertex shader with HRESULT %#010x", result);
				wereThereErrors = true;
			}
		}
		return !wereThereErrors;
	}
}