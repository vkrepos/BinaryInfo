#pragma once
#include "stdafx.h"
#include "Helpers.h"

namespace peinfo
{
	enum class TableId
	{
		MinTableId = 0,
		Module = 0,
		TypeRef = 1,
		TypeDef = 2,
		Field = 4,
		MethodDef = 6,
		Param = 8,
		InterfaceImpl = 9,
		MemberRef = 10,
		Constant = 11,
		CustomAttribute = 12,
		FieldMarshal = 13,
		DeclSecurity = 14,
		ClassLayout = 15,
		FieldLayout = 16,
		StandAloneSig = 17,
		EventMap = 18,
		Event = 20,
		PropertyMap = 21,
		Property = 23,
		MethodSemantics = 24,
		MethodImpl = 25,
		ModuleRef = 26,
		TypeSpec = 27,
		ImplMap = 28,
		FieldRVA = 29,
		Assembly = 32,
		AssemblyProcessor = 33,
		AssemblyOS = 34,
		AssemblyRef = 35,
		AssemblyRefProcessor = 36,
		AssemblyRefOS = 37,
		File = 38,
		ExportedType = 39,
		ManifestResource = 40,
		NestedClass = 41,
		GenericParam = 42,
		MethodSpec = 43,
		GenericParamConstraint = 44,

		MaxTableId = 63,
		NotUsed = 64
	};

	enum class ColumnType
	{
		Byte,
		Word,
		Dword,
		String,
		Guid,
		Blob,
		
		// single table indicies
		TypeDef,
		Field,
		MethodDef,
		Param,

		// coded indicies
		ResolutionScope,
		TypeDefOrRef,
		MemberRefParent,
		HasConstant,
		HasCustomAttribute,
		CustomAttributeType
	};

	class SchemaInfoProvider
	{
		DECLARE_NONCOPYABLE(SchemaInfoProvider);
	public:
		SchemaInfoProvider(std::array<std::uint32_t, 64> tableRowCounts, BYTE heapIndexSizes)
			: tableRowCounts_(tableRowCounts), heapIndexSizes_(heapIndexSizes)
		{
		}

		std::uint32_t GetRowCount(TableId tableId) const
		{
			// check

			return tableRowCounts_[static_cast<int>(tableId)];
		}

		std::uint32_t GetColumnTypeSize(TableId tableId, std::uint32_t columnIndex) const
		{
			auto columnType = GetColumnTypesByTableId(tableId).at(columnIndex);
			return GetColumnSizeByType(columnType);
		}

		std::uint32_t GetColumnOffset(TableId tableId, std::uint32_t columnIndex) const
		{
			// check

			std::uint32_t resultSize = 0;
			auto columnTypes = GetColumnTypesByTableId(tableId);
			for (std::uint32_t i = 0; i < columnIndex; ++i)
			{
				auto columnType = columnTypes[i];
				resultSize += GetColumnSizeByType(columnType);
			}

			return resultSize;
		}

		std::uint32_t GetRowSize(TableId tableId) const
		{
			std::uint32_t resultSize = 0;
			for (auto columnType : GetColumnTypesByTableId(tableId))
			{
				resultSize += GetColumnSizeByType(columnType);
			}

			return resultSize;
		}

		std::uint32_t GetTableOffset(TableId tableId) const
		{
			std::uint32_t tableOffset = 0;
			auto tableIndex = static_cast<std::uint32_t>(tableId);
			for (uint32_t i = 0; i < tableIndex; i++)
			{
				auto currentTableId = static_cast<TableId>(i);
				tableOffset += GetTableSize(currentTableId);
			}

			return tableOffset;
		}

		std::uint32_t GetTableSize(TableId tableId) const
		{
			return GetRowCount(tableId) * GetRowSize(tableId);
		}

	private:
		std::uint32_t GetColumnSizeByType(ColumnType columnType) const
		{
			switch (columnType)
			{
			case ColumnType::Byte: return 1;
			case ColumnType::Word: return 2;
			case ColumnType::Dword: return 4;
			case ColumnType::String: return ((heapIndexSizes_ & 1) == 0) ? 2 : 4;
			case ColumnType::Guid: return ((heapIndexSizes_ & 2) == 0) ? 2 : 4;
			case ColumnType::Blob: return ((heapIndexSizes_ & 4) == 0) ? 2 : 4;
			case ColumnType::ResolutionScope: 
				return GetCodedIndexSize(
				{ 
					TableId::Module, 
					TableId::ModuleRef, 
					TableId::AssemblyRef, 
					TableId::TypeRef 
				});
			case ColumnType::TypeDefOrRef: 
				return GetCodedIndexSize(
				{ 
					TableId::TypeDef, 
					TableId::TypeRef, 
					TableId::TypeSpec 
				});
			case ColumnType::MemberRefParent: 
				return GetCodedIndexSize(
				{
					TableId::TypeRef, 
					TableId::ModuleRef, 
					TableId::MethodDef, 
					TableId::TypeSpec, 
					TableId::TypeDef
				});
			case ColumnType::HasConstant:
				return GetCodedIndexSize(
				{
					TableId::Param,
					TableId::Field,
					TableId::Property
				});
			case ColumnType::HasCustomAttribute:
				return GetCodedIndexSize(
					{
						TableId::MethodDef,
						TableId::Field,
						TableId::TypeRef,
						TableId::TypeDef,
						TableId::Param,
						TableId::InterfaceImpl,
						TableId::MemberRef,
						TableId::Module,
						TableId::DeclSecurity,
						TableId::Property,
						TableId::Event,
						TableId::StandAloneSig,
						TableId::ModuleRef,
						TableId::TypeSpec,
						TableId::Assembly,
						TableId::AssemblyRef,
						TableId::File,
						TableId::ExportedType,
						TableId::ManifestResource,
						TableId::GenericParam,
						TableId::GenericParamConstraint,
						TableId::MethodSpec
					});
			case ColumnType::CustomAttributeType:
				return GetCodedIndexSize(
					{
						TableId::NotUsed,
						TableId::NotUsed,
						TableId::MethodDef,
						TableId::MemberRef,
						TableId::NotUsed
					});
			case ColumnType::TypeDef:
				return GetCodedIndexSize({ TableId::TypeDef });
			case ColumnType::Field:
				return GetCodedIndexSize({ TableId::Field });
			case ColumnType::MethodDef:
				return GetCodedIndexSize({ TableId::MethodDef });
			case ColumnType::Param:
				return GetCodedIndexSize({ TableId::Param });
			default:
				throw std::logic_error("not implemented");
			}
		}

		std::vector<ColumnType> GetColumnTypesByTableId(TableId tableId) const
		{
			switch (tableId)
			{
			case TableId::Module:
				return
				{
					ColumnType::Word,
					ColumnType::String,
					ColumnType::Guid,
					ColumnType::Guid,
					ColumnType::Guid
				};
			case TableId::TypeRef:
				return
				{
					ColumnType::ResolutionScope,
					ColumnType::String,
					ColumnType::String
				};
			case TableId::TypeDef:
				return
				{
					ColumnType::Dword,
					ColumnType::String,
					ColumnType::String,
					ColumnType::TypeDefOrRef,
					ColumnType::Field,
					ColumnType::MethodDef
				};
			case TableId::Field: 
				return
				{
					ColumnType::Word,
					ColumnType::String,
					ColumnType::Blob
				};
			case TableId::MethodDef: 
				return 
				{
					ColumnType::Dword,
					ColumnType::Word,
					ColumnType::Word,
					ColumnType::String,
					ColumnType::Blob,
					ColumnType::Param
				};
			case TableId::Param: 
				return
				{
					ColumnType::Word,
					ColumnType::Word,
					ColumnType::String
				};
			case TableId::InterfaceImpl:
				return
				{
					ColumnType::TypeDef,
					ColumnType::TypeDefOrRef
				};
			case TableId::MemberRef:
				return
				{
					ColumnType::MemberRefParent,
					ColumnType::String,
					ColumnType::Blob
				};
			case TableId::Constant: 
				return
				{
					ColumnType::Byte,
					ColumnType::HasConstant,
					ColumnType::Blob
				};
			case TableId::CustomAttribute: 
				return
				{
					ColumnType::HasCustomAttribute,
					ColumnType::CustomAttributeType,
					ColumnType::Blob
				};
			case TableId::ModuleRef: break;
			case TableId::TypeSpec: break;
			case TableId::AssemblyRef: break;
			default:
			{
				return {};
			}
			}

			return {};
		}

		constexpr std::uint32_t BitsNeeded(std::uint32_t n) const
		{
			return n <= 1 ? 0 : 1 + BitsNeeded((n + 1) / 2);
		}

		bool CompareRowCount(TableId left, TableId right) const
		{
			return GetRowCount(left) < GetRowCount(right);
		}

		std::uint32_t GetMaxRowCount(std::vector<TableId> tableIds) const
		{
			// check tableIds.empty() == false

			std::vector<TableId> validTableIds(tableIds.size());
			auto validTableIdsEnd = std::remove_copy(tableIds.begin(), tableIds.end(), validTableIds.begin(), TableId::NotUsed);
			if (validTableIdsEnd != validTableIds.end())
			{
				validTableIds.erase(validTableIdsEnd, validTableIds.end());
			}			

			auto tableIdWithMaxRowCount = *std::max_element(
				validTableIds.begin(),
				validTableIds.end(),
				[this](TableId left, TableId right) { return CompareRowCount(left, right); });
			return GetRowCount(tableIdWithMaxRowCount);
		}

		std::uint32_t GetCodedIndexSize(std::vector<TableId> indexedTableIds) const
		{
			auto indexedTableCount = indexedTableIds.size();
			auto maxRowCount = GetMaxRowCount(indexedTableIds);
			auto wordIsEnough = BitsNeeded(indexedTableCount) + BitsNeeded(maxRowCount) <= std::numeric_limits<std::uint16_t>::digits;
			return wordIsEnough ? 2 : 4;
		}

		std::array<std::uint32_t, 64>  tableRowCounts_;
		BYTE heapIndexSizes_;
	};

	class StringHeap
	{
		DECLARE_NONCOPYABLE(StringHeap);
	public:
		StringHeap(void* startAddress)
			: startAddress_(startAddress)
		{
		}

		std::string_view GetString(std::uint32_t index) const
		{
			return std::string_view(AddOffset<char>(startAddress_, index));
		}

	private:
		void* startAddress_;
	};

	class GuidHeap
	{
		DECLARE_NONCOPYABLE(GuidHeap);
	public:
		GuidHeap(void* startAddress)
			: startAddress_(startAddress)
		{
		}

		GUID GetGuid(std::uint32_t index) const
		{
			return *AddOffset<GUID>(startAddress_, index);
		}

	private:
		void* startAddress_;
	};

	class BlobHeap
	{
		DECLARE_NONCOPYABLE(BlobHeap);
	public:
		BlobHeap(void* startAddress)
			: startAddress_(startAddress)
		{
		}

		std::vector<std::uint8_t> GetBlob(std::uint32_t index) const
		{
			std::uint32_t size = 0;
			std::uint8_t* data = nullptr;
			std::bitset<8> firstByte = *AddOffset<std::uint8_t>(startAddress_, index);

			if (firstByte.test(7))
			{
				if (firstByte.test(6))
				{
					size = *AddOffset<std::uint32_t>(startAddress_, index);
					data = AddOffset<std::uint8_t>(startAddress_, index + sizeof(std::uint32_t));
				}
				else
				{
					size = *AddOffset<std::uint16_t>(startAddress_, index);
					data = AddOffset<std::uint8_t>(startAddress_, index + sizeof(std::uint16_t));
				}
			}
			else
			{
				size = *AddOffset<std::uint8_t>(startAddress_, index);
				data = AddOffset<std::uint8_t>(startAddress_, index + sizeof(std::uint8_t));
			}

			return std::vector<std::uint8_t>(data, data + size);
		}

	private:
		void* startAddress_;
	};

	class Heaps
	{
		DECLARE_NONCOPYABLE(Heaps);
	public:
		Heaps(void* stringHeapAddress, void* guidHeapAddress, void* blobHeapAddress)
			: stringHeap_(stringHeapAddress), guidHeap_(guidHeapAddress), blobHeap_(blobHeapAddress)
		{
		}

		const StringHeap& GetStringHeap() const
		{
			return stringHeap_;
		}

		const GuidHeap& GetGuidHeap() const
		{
			return guidHeap_;
		}

		const BlobHeap& GetBlobHeap() const
		{
			return blobHeap_;
		}

	private:
		StringHeap stringHeap_;
		GuidHeap guidHeap_;
		BlobHeap blobHeap_;
	};

	class Table
	{
	public:
		Table(
			void* startAddress,
			TableId tableId, 
			std::shared_ptr<SchemaInfoProvider> schemaInfoProvider, 
			std::shared_ptr<Heaps> heaps)
		: startAddress_(startAddress), tableId_(tableId), schemaInfoProvider_(schemaInfoProvider), heaps_(heaps)
		{			
		}

		std::uint32_t GetRowCount() const
		{
			return schemaInfoProvider_->GetRowCount(tableId_);
		}

		std::string_view GetString(std::uint32_t rowIndex, std::uint32_t columnIndex) const
		{
			auto stringValueIndex = GetValue(rowIndex, columnIndex);
			return heaps_->GetStringHeap().GetString(stringValueIndex);
		}

		std::uint32_t GetValue(std::uint32_t rowIndex, std::uint32_t columnIndex) const
		{
			// check rowIndex, columnIndex

			auto offset = rowIndex * schemaInfoProvider_->GetRowSize(tableId_) + schemaInfoProvider_->GetColumnOffset(tableId_, columnIndex);
			auto valueSize = schemaInfoProvider_->GetColumnTypeSize(tableId_, columnIndex);
			return ReadValue(offset, valueSize);
		}

	private:
		std::uint32_t ReadValue(std::uint32_t offset, std::uint32_t valueSize) const
		{
			switch (valueSize)
			{
			case sizeof(std::uint8_t) :
				return *AddOffset<std::uint8_t>(startAddress_, offset);
			case sizeof(std::uint16_t):
				return *AddOffset<std::uint16_t>(startAddress_, offset);
			case sizeof(std::uint32_t):
				return *AddOffset<std::uint32_t>(startAddress_, offset);
			default:
				throw std::out_of_range("invalid valueSize");
			}
		}

		void* startAddress_;
		TableId tableId_;
		std::shared_ptr<SchemaInfoProvider> schemaInfoProvider_;
		std::shared_ptr<Heaps> heaps_;
	};

	enum TypeDefTableColumns : std::uint32_t
	{
		TypeName = 1,
		Namespace = 2,
		MethodList = 5
	};

	class TableWrapper
	{
	public:
		TableWrapper(Table table)
			: table_(table)
		{
		}

		std::uint32_t GetRowCount() const
		{
			return table_.GetRowCount();
		}

	protected:
		std::uint32_t GetValue(std::uint32_t rowIndex, std::uint32_t columnIndex) const
		{
			return table_.GetValue(rowIndex, columnIndex);
		}

		std::string_view GetString(std::uint32_t rowIndex, std::uint32_t columnIndex) const
		{
			return table_.GetString(rowIndex, columnIndex);
		}

	private:
		Table table_;
	};

	class ModuleTable : public TableWrapper
	{
	public:
		ModuleTable(const Table& table)
			: TableWrapper(table)
		{
		}

		std::string_view GetName(std::uint32_t rowIndex) const
		{
			return GetString(rowIndex, NameColumnIndex);
		}

	private:
		const std::uint32_t NameColumnIndex = 1;
	};

	class TypeDefTable : public TableWrapper
	{
	public:
		TypeDefTable(const Table& table)
			: TableWrapper(table)
		{
		}

		std::string_view GetTypeName(std::uint32_t rowIndex)
		{
			return GetString(rowIndex, TypeNameColumnIndex);
		}

		std::string_view GetTypeNamespace(std::uint32_t rowIndex)
		{
			return GetString(rowIndex, TypeNamespaceColumnIndex);
		}

		std::uint32_t GetMethodListIndex(std::uint32_t rowIndex)
		{
			return GetValue(rowIndex, MethodListIndex);
		}

		const std::uint32_t TypeNameColumnIndex = 1;
		const std::uint32_t TypeNamespaceColumnIndex = 2;
		const std::uint32_t MethodListIndex = 5;
	};

	class TypeRefTable : public TableWrapper
	{
	public:
		TypeRefTable(const Table& table)
			: TableWrapper(table)
		{
		}

		std::string_view GetTypeName(std::uint32_t rowIndex)
		{
			return GetString(rowIndex, TypeNameColumnIndex);
		}

		std::string_view GetTypeNamespace(std::uint32_t rowIndex)
		{
			return GetString(rowIndex, TypeNamespaceColumnIndex);
		}

		const std::uint32_t TypeNameColumnIndex = 1;
		const std::uint32_t TypeNamespaceColumnIndex = 2;
	};

	class MethodDefTable : public TableWrapper
	{
	public:
		MethodDefTable(const Table& table)
			: TableWrapper(table)
		{
		}

		std::string_view GetMethodName(std::uint32_t rowIndex)
		{
			return GetString(rowIndex, MethodNameColumnIndex);
		}

		const std::uint32_t MethodNameColumnIndex = 3;
	};

	class MemberRefTable : public TableWrapper
	{
	public:
		MemberRefTable(const Table& table)
			: TableWrapper(table)
		{
		}
		
		std::uint32_t GetParentIndex(std::uint32_t rowIndex)
		{
			return GetValue(rowIndex, ParentIndexColumnIndex);
		}

		std::string_view GetMethodName(std::uint32_t rowIndex)
		{
			return GetString(rowIndex, MethodNameColumnIndex);
		}

		const std::uint32_t ParentIndexColumnIndex = 0;
		const std::uint32_t MethodNameColumnIndex = 1;
	};

	class AssemblyTable : public TableWrapper
	{
	public:
		AssemblyTable(const Table& table)
			: TableWrapper(table)
		{
		}

	private:
	};

	class MetadataTables
	{
		DECLARE_NONCOPYABLE(MetadataTables);
	public:
		MetadataTables(
			void* tablesStartAddress, 
			std::shared_ptr<SchemaInfoProvider> schemaInfoProvider, 
			std::shared_ptr<Heaps> heaps)
		{
			auto currentTableStartAddress = tablesStartAddress;
			for (size_t i = 0; i < 64; i++)
			{
				TableId tableId = static_cast<TableId>(i);
				tables_.emplace_back(std::make_unique<Table>(currentTableStartAddress, tableId, schemaInfoProvider, heaps));
				currentTableStartAddress = AddOffset<void>(currentTableStartAddress, schemaInfoProvider->GetTableSize(tableId));
			}
		}

		ModuleTable GetModuleTable() const
		{			
			return ModuleTable(GetTableById(TableId::Module));
		}

		TypeDefTable GetTypeDefTable() const
		{
			return TypeDefTable(GetTableById(TableId::TypeDef));
		}

		TypeRefTable GetTypeRefTable() const
		{
			return TypeRefTable(GetTableById(TableId::TypeRef));
		}

		MethodDefTable GetMethodDefTable() const
		{
			return MethodDefTable(GetTableById(TableId::MethodDef));
		}

	private:
		const Table& GetTableById(TableId tableId) const
		{
			return *tables_[static_cast<size_t>(tableId)];
		}

		std::vector<std::unique_ptr<Table>> tables_;
	};

	class MetadataDirectoryFacade
	{
		DECLARE_NONCOPYABLE(MetadataDirectoryFacade);
	public:
		MetadataDirectoryFacade(std::shared_ptr<MetadataTables> tables)
			: tables_(tables)
		{
		}

		const MetadataTables& GetMetadataTables()
		{
			return *tables_;
		}

	private:
		std::shared_ptr<MetadataTables> tables_;
	};

	struct MetadataStorageSignature
	{
		DWORD lSignature;
		WORD iMajorVer;
		WORD iMinorVer;
		DWORD iExtraData;
		DWORD iVersionString;
	};

	struct MetadataStorageHeader
	{
		BYTE fFlags;
		BYTE padding;
		WORD iStreams;
	};

	struct MetadataStreamHeader
	{
		DWORD iOffset;
		DWORD iSize;
	};

	struct MetadataTableStreamHeader
	{
		DWORD Reserved;
		BYTE Major;
		BYTE Minor;
		BYTE HeapOffsetSizes;
		BYTE Rid;
		ULONG64 MaksValid;
		ULONG64 Sorted;
	};

	class MetadataDirectoryReader
	{
	public:
		std::unique_ptr<MetadataDirectoryFacade> Read(void* startAddress, std::uint32_t size)
		{
			auto storageSignature = static_cast<MetadataStorageSignature*>(startAddress);

			auto versionStringStart = AddOffset<char>(storageSignature, sizeof(MetadataStorageSignature));

			std::string_view versionString(versionStringStart);

			auto storageHeader = AddOffset<MetadataStorageHeader>(versionStringStart, storageSignature->iVersionString);

			auto currentStreamHeader = AddOffset<MetadataStreamHeader>(storageHeader, sizeof(MetadataStorageHeader));

			MetadataTableStreamHeader* metadataTableStreamHeader = nullptr;
			void* blobStream = nullptr;
			void* stringStream = nullptr;
			void* guidStream = nullptr;

			for (auto i = 0; i < storageHeader->iStreams; ++i)
			{
				std::string currentStreamName = AddOffset<char>(currentStreamHeader, sizeof(MetadataStreamHeader));
				if (currentStreamName == "#~"s)
				{
					metadataTableStreamHeader = AddOffset<MetadataTableStreamHeader>(startAddress, currentStreamHeader->iOffset);
				}
				else if (currentStreamName == "#Blob"s)
				{
					blobStream = AddOffset<void>(startAddress, currentStreamHeader->iOffset);
				}
				else if (currentStreamName == "#Strings"s)
				{
					stringStream = AddOffset<void>(startAddress, currentStreamHeader->iOffset);
				}
				else if (currentStreamName == "#GUID"s)
				{
					guidStream = AddOffset<void>(startAddress, currentStreamHeader->iOffset);
				}

				auto currentHeaderSize = sizeof(MetadataStreamHeader) + currentStreamName.size() + 1;
				currentHeaderSize = ((currentHeaderSize + 3) / 4) * 4; // TODO: extract func
				currentStreamHeader = AddOffset<MetadataStreamHeader>(currentStreamHeader, currentHeaderSize);
			}

			auto recordNumbersStart = AddOffset<std::uint32_t>(metadataTableStreamHeader, sizeof(MetadataTableStreamHeader));

			std::array<std::uint32_t, 64> recordNumberByTable{};

			std::bitset<64> maskValid(metadataTableStreamHeader->MaksValid);

			auto currentRecordNumber = recordNumbersStart;
			for (size_t i = 0; i < maskValid.size(); ++i)
			{
				if (maskValid.test(i))
				{
					recordNumberByTable[i] = *currentRecordNumber;
					++currentRecordNumber;
				}
			}

			auto heaps = std::make_shared<Heaps>(stringStream, guidStream, blobStream);
			auto schemaInfoProvider = std::make_shared<SchemaInfoProvider>(recordNumberByTable, metadataTableStreamHeader->HeapOffsetSizes);
			void* tablesStartAddress = AddOffset<void>(recordNumbersStart, maskValid.count() * sizeof(std::uint32_t));
			
			auto tables = std::make_shared<MetadataTables>(tablesStartAddress, schemaInfoProvider, heaps);

			return std::make_unique<MetadataDirectoryFacade>(tables);
		}
	};

	void Test0(ModuleTable moduleTable, TypeRefTable typeRefTable)
	{
		auto name = moduleTable.GetName(0);
		auto rowCount = moduleTable.GetRowCount();

		std::cout << name << ":" << rowCount << std::endl;

		auto typeRefRowCount = typeRefTable.GetRowCount();
		for (size_t i = 0; i < typeRefRowCount; i++)
		{
			auto typeName = typeRefTable.GetTypeName(i);
			auto namespaceName = typeRefTable.GetTypeNamespace(i);

			std::cout << namespaceName << "." << typeName << std::endl;
		}
	}

	void Test1(TypeDefTable typeDefTable)
	{
		for (size_t i = 0; i < typeDefTable.GetRowCount(); ++i)
		{
			auto typeName = typeDefTable.GetTypeName(i);
			auto namespaceName = typeDefTable.GetTypeNamespace(i);

			std::cout << namespaceName << "." << typeName << std::endl;
		}
	}

	void Test2(MethodDefTable methodDefTable)
	{
		for (size_t i = 0; i < methodDefTable.GetRowCount(); ++i)
		{
			auto methodName = methodDefTable.GetMethodName(i);

			std::cout << methodName << std::endl;
		}
	}

	//std::string GetAssemblyVersion(TypeRefTable typeRefTable, MemberRefTable memberRefTable)
	//{
	//	auto typeName = "AssemblyVersionAttribute";
	//	auto namespaceName = "System.Reflection";
	//	
	//	for (std::uint32_t i = 0; i < typeRefTable.GetRowCount(); ++i)
	//	{
	//		auto currentTypeName = typeRefTable.GetTypeName(i);
	//		if (currentTypeName != typeName)
	//		{
	//			continue;
	//		}

	//		auto currentNamespace = typeRefTable.GetTypeNamespace(i);
	//		if (currentNamespace != namespaceName)
	//		{
	//			continue;
	//		}
	//	}

	//	for (std::uint32_t i = 0; i < memberRefTable.GetRowCount(); ++i)
	//	{
	//		memberRefTable.GetParentIndex();
	//	}

	//	return "";
	//}

	//std::string GetTargetFramework()
	//{
	//	return "";
	//}
}
