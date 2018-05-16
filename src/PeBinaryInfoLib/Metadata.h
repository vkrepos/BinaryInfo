#pragma once
#include "stdafx.h"
#include "Helpers.h"

namespace peinfo
{
	void CheckComError(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::runtime_error("");
		}
	}

	class CustomAttributeReader
	{
	public:
		CustomAttributeReader(void* metadata, ULONG metadataSize)
		{
			CComPtr<IMetaDataDispenser> metadataDispenser;
			HRESULT hr = CoCreateInstance(CLSID_CorMetaDataDispenser, nullptr, CLSCTX_INPROC_SERVER, IID_IMetaDataDispenser, (void**)&metadataDispenser);
			CheckComError(hr);

			CComPtr<IUnknown> metadataImportUnknown;
			hr = metadataDispenser->OpenScopeOnMemory(
				metadata,
				metadataSize,
				CorOpenFlags::ofReadOnly,
				IID_IMetaDataImport2,
				&metadataImportUnknown);
			CheckComError(hr);

			CComQIPtr<IMetaDataImport2, &IID_IMetaDataImport2> metadataImport(metadataImportUnknown);
			metadataImport_ = metadataImport;
		}

		std::string GetTargetFramework()
		{
			LPCWSTR TargetFrameworkAttribute = L"System.Runtime.Versioning.TargetFrameworkAttribute";
			const void* blob = 0;
			ULONG blobSize = 0;
			HRESULT hr = metadataImport_->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), TargetFrameworkAttribute, &blob, &blobSize);
			if (hr == S_FALSE)
			{
				return ".NET v3.5 or less";
			}
			CheckComError(hr);
			
			auto customAttributrMarker = ReadAtOffset<std::uint16_t>(blob, 0);
			CheckError(customAttributrMarker == 1, "customAttributrMarker");

			auto packedLengthAddress = AddOffset<void>(blob, sizeof(std::uint16_t));
			auto packedLength = ReadPackedLength(packedLengthAddress);

			auto stringAddress = AddOffset<char>(packedLengthAddress, packedLength.size);
			return std::string(stringAddress, packedLength.value);
		}

		std::string GetAssemblyVersion()
		{
			CComPtr<IMetaDataAssemblyImport> metadataAssemblyImport;
			HRESULT hr = metadataImport_->QueryInterface(IID_IMetaDataAssemblyImport, (void**)&metadataAssemblyImport);
			CheckComError(hr);

			ASSEMBLYMETADATA assemblyMetadata{};
			DWORD flags = 0;
			hr = metadataAssemblyImport->GetAssemblyProps(
				TokenFromRid(1, mdtAssembly),
				NULL,
				NULL,
				NULL,
				NULL,
				0,
				NULL,
				&assemblyMetadata,
				&flags);
			CheckComError(hr);

			return std::to_string(assemblyMetadata.usMajorVersion) + "." 
				+ std::to_string(assemblyMetadata.usMinorVersion) + "." 
				+ std::to_string(assemblyMetadata.usBuildNumber) + "."
				+ std::to_string(assemblyMetadata.usRevisionNumber);
		}

		bool AreOptimizationsDisabled()
		{
			LPCWSTR DebuggableAttribute = L"System.Diagnostics.DebuggableAttribute";
			const void* blob = 0;
			ULONG blobSize = 0;
			HRESULT hr = metadataImport_->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), DebuggableAttribute, &blob, &blobSize);
			if (hr == S_FALSE)
			{
				return false;
			}
			CheckComError(hr);

			auto customAttributrMarker = ReadAtOffset<std::uint16_t>(blob, 0);
			CheckError(customAttributrMarker == 1, "customAttributrMarker");

			const std::uint32_t DisableOptimizations = 1u << 8; // System.Diagnostics.DebuggingModes.DisableOptimizations
			auto debuggingModes = ReadAtOffset<std::uint32_t>(blob, 2);
			return IsFlagSet(debuggingModes, DisableOptimizations);
		}

	private:
		struct PackedLength
		{
			std::uint32_t value;
			std::uint32_t size;
		};

		PackedLength ReadPackedLength(void* address)
		{
			auto firstByte = ReadAtOffset<std::uint8_t>(address, 0);
			if ((firstByte & 0x80) == 0)
			{
				return PackedLength { firstByte, sizeof(std::uint8_t) };
			}

			if ((firstByte & 0xC0) == 0x80)
			{
				auto value = ReadAtOffset<std::uint16_t>(address, 0);
				return PackedLength{ value, sizeof(std::uint16_t) };
			}

			if ((firstByte & 0xE0) == 0xC0)
			{
				auto value = ReadAtOffset<std::uint32_t>(address, 0);
				return PackedLength{ value, sizeof(std::uint32_t) };
			}

			throw std::runtime_error("invalid packed length");
		}



		CComPtr<IMetaDataImport2> metadataImport_;
	};
}