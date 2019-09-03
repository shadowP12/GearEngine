#include "VertexDescription.h"
#include "Math/GMath.h"
#include "Utility/Hash.h"
#include<algorithm>

uint32_t VertexElement::getTypeSize(VertexElementType etype)
{
	switch (etype)
	{
	case VET_COLOR:
	case VET_COLOR_ABGR:
	case VET_COLOR_ARGB:
		return sizeof(glm::vec4);
	case VET_UBYTE4_NORM:
		return sizeof(uint32_t);
	case VET_FLOAT1:
		return sizeof(float);
	case VET_FLOAT2:
		return sizeof(float) * 2;
	case VET_FLOAT3:
		return sizeof(float) * 3;
	case VET_FLOAT4:
		return sizeof(float) * 4;
	case VET_USHORT1:
		return sizeof(uint16_t);
	case VET_USHORT2:
		return sizeof(uint16_t) * 2;
	case VET_USHORT4:
		return sizeof(uint16_t) * 4;
	case VET_SHORT1:
		return sizeof(int16_t);
	case VET_SHORT2:
		return sizeof(int16_t) * 2;
	case VET_SHORT4:
		return sizeof(int16_t) * 4;
	case VET_UINT1:
		return sizeof(uint32_t);
	case VET_UINT2:
		return sizeof(uint32_t) * 2;
	case VET_UINT3:
		return sizeof(uint32_t) * 3;
	case VET_UINT4:
		return sizeof(uint32_t) * 4;
	case VET_INT4:
		return sizeof(int32_t) * 4;
	case VET_INT1:
		return sizeof(int32_t);
	case VET_INT2:
		return sizeof(int32_t) * 2;
	case VET_INT3:
		return sizeof(int32_t) * 3;
	case VET_UBYTE4:
		return sizeof(uint8_t) * 4;
	default:
		break;
	}

	return 0;
}

unsigned short VertexElement::getTypeCount(VertexElementType etype)
{
	switch (etype)
	{
	case VET_COLOR:
	case VET_COLOR_ABGR:
	case VET_COLOR_ARGB:
		return 4;
	case VET_FLOAT1:
	case VET_SHORT1:
	case VET_USHORT1:
	case VET_INT1:
	case VET_UINT1:
		return 1;
	case VET_FLOAT2:
	case VET_SHORT2:
	case VET_USHORT2:
	case VET_INT2:
	case VET_UINT2:
		return 2;
	case VET_FLOAT3:
	case VET_INT3:
	case VET_UINT3:
		return 3;
	case VET_FLOAT4:
	case VET_SHORT4:
	case VET_USHORT4:
	case VET_INT4:
	case VET_UINT4:
	case VET_UBYTE4:
	case VET_UBYTE4_NORM:
		return 4;
	default:
		break;
	}
	//todo:Exception
	return 0;
}

VertexElementType VertexElement::getBestColorVertexElementType()
{
	return VET_COLOR_ARGB;
}

bool VertexElement::operator== (const VertexElement& rhs) const
{
	if (mType != rhs.mType || mOffset != rhs.mOffset || mSemantic != rhs.mSemantic)
	{
		return false;
	}
	else
		return true;
}

bool VertexElement::operator!= (const VertexElement& rhs) const
{
	return !(*this == rhs);
}

size_t VertexElement::getHash()
{
	size_t hash = 0;
	Hash(hash, mType);
	Hash(hash, mOffset);
	Hash(hash, mSemantic);
	return hash;
}

void VertexDesc::addVertElem(VertexElementType type, VertexElementSemantic semantic)
{
	clearIfItExists(type, semantic);

	VertexElement newElement(type, semantic);

	uint32_t insertToIndex = (uint32_t)mVertexElements.size();
	uint32_t idx = 0;
	for (auto& elem : mVertexElements)
	{
		idx++;
	}

	mVertexElements.insert(mVertexElements.begin() + insertToIndex, newElement);
}

bool VertexDesc::hasElement(VertexElementSemantic semantic)
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(),
		[semantic](const VertexElement& x)
	{
		return x.getSemantic() == semantic;
	});

	if (findIter != mVertexElements.end())
	{
		return true;
	}

	return false;
}

uint32_t VertexDesc::getElementSize(VertexElementSemantic semantic)
{
	for (auto& element : mVertexElements)
	{
		if (element.getSemantic() == semantic)
			return element.getSize();
	}

	return -1;
}

uint32_t VertexDesc::getElementOffset(VertexElementSemantic semantic)
{
	uint32_t vertexOffset = 0;
	for (auto& element : mVertexElements)
	{
		if (element.getSemantic() == semantic)
			break;

		vertexOffset += element.getSize();
	}

	return vertexOffset;
}

uint32_t VertexDesc::getVertexStride()
{
	uint32_t vertexStride = 0;
	for (uint32_t i = 0; i < mVertexElements.size(); i++)
	{
		vertexStride += mVertexElements[i].getSize();
	}

	return vertexStride;
}

const VertexElement* VertexDesc::getElement(VertexElementSemantic semantic)
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(),
		[semantic](const VertexElement& x)
	{
		return x.getSemantic() == semantic;
	});

	if (findIter != mVertexElements.end())
		return &(*findIter);

	return nullptr;
}

void VertexDesc::clearIfItExists(VertexElementType type, VertexElementSemantic semantic)
{
	auto findIter = std::find_if(mVertexElements.begin(), mVertexElements.end(),
		[semantic](const VertexElement& x)
	{
		return x.getSemantic() == semantic;
	});

	if (findIter != mVertexElements.end())
	{
		mVertexElements.erase(findIter);
	}
}

size_t VertexDesc::getHash()
{
	size_t hash = 0;
	for (uint32_t i = 0; i < mVertexElements.size(); i++)
	{
		size_t hash_t = mVertexElements[i].getHash();
		Hash(hash, hash_t);
	}
	return hash;
}