#ifndef VERTEX_DESC_H
#define VERTEX_DESC_H
#include <vector>

enum class VertexLayout
{
	Position = 0x01,
	Color = 0x02,
	Normal = 0x04,
	Tangent = 0x08,
	BoneWeights = 0x10,
	UV0 = 0x20,
	UV1 = 0x40,
	PC = Position | Color,
	PU = Position | UV0,
	PCU = Position | Color | UV0,
	PCN = Position | Color | Normal,
	PCNU = Position | Color | Normal | UV0,
	PCNT = Position | Color | Normal | Tangent,
	PCNTU = Position | Color | Normal | Tangent | UV0,
	PN = Position | Normal,
	PNU = Position | Normal | UV0,
	PNT = Position | Normal | Tangent,
	PNTU = Position | Normal | Tangent | UV0,
};

enum VertexElementSemantic
{
	VES_POSITION = 1, //Position
	VES_BLEND_WEIGHTS = 2, //Blend weights
	VES_BLEND_INDICES = 3, //Blend indices
	VES_NORMAL = 4, //Normal
	VES_COLOR = 5, //Color
	VES_TEXCOORD = 6, //UV coordinate
	VES_BITANGENT = 7, //Bitangent
	VES_TANGENT = 8, //Tangent
	VES_POSITIONT = 9, //Transformed position
	VES_PSIZE = 10 //Point size
};

enum VertexElementType
{
	VET_FLOAT1 = 0,
	VET_FLOAT2 = 1, 
	VET_FLOAT3 = 2, 
	VET_FLOAT4 = 3, 
	VET_COLOR = 4, 
	VET_SHORT1 = 5, 
	VET_SHORT2 = 6,
	VET_SHORT4 = 8, 
	VET_UBYTE4 = 9, 
	VET_COLOR_ARGB = 10,
	VET_COLOR_ABGR = 11,
	VET_UINT4 = 12, 
	VET_INT4 = 13,
	VET_USHORT1 = 14, 
	VET_USHORT2 = 15, 
	VET_USHORT4 = 17,
	VET_INT1 = 18, 
	VET_INT2 = 19,
	VET_INT3 = 20,
	VET_UINT1 = 21,
	VET_UINT2 = 22,
	VET_UINT3 = 23,
	VET_UBYTE4_NORM = 24,
	VET_COUNT,
	VET_UNKNOWN = 0xffff
};

class VertexElement
{
public:
	VertexElement(VertexElementType type, VertexElementSemantic semantic)
	{
		mType = type;
		mSemantic = semantic;
		mOffset = 0;
		mSize = getTypeSize(type);
	}
	~VertexElement()
	{
	}
	bool operator== (const VertexElement& rhs) const;
	bool operator!= (const VertexElement& rhs) const;

	uint32_t getOffset() const { return mOffset; }

	VertexElementType getType() const { return mType; }

	VertexElementSemantic getSemantic() const { return mSemantic; }

	uint32_t getSize(){ return mSize;}

	static uint32_t getTypeSize(VertexElementType etype);

	static uint16_t getTypeCount(VertexElementType etype);

	static VertexElementType getBestColorVertexElementType();

	size_t getHash();
protected:
	uint32_t mOffset;
	uint32_t mSize;
	VertexElementType mType;
	VertexElementSemantic mSemantic;
};

class VertexDesc
{
public:
	VertexDesc()
	{
	}
	void addVertElem(VertexElementType type, VertexElementSemantic semantic);

	bool hasElement(VertexElementSemantic semantic);

	uint32_t getElementSize(VertexElementSemantic semantic);

	uint32_t getElementOffset(VertexElementSemantic semantic);

	uint32_t getVertexStride();

	uint32_t getNumElements() const { return (uint32_t)mVertexElements.size(); }

	const VertexElement& getElement(uint32_t idx) const { return mVertexElements[idx]; }

	const VertexElement* getElement(VertexElementSemantic semantic);

	size_t getHash();

private:
	void clearIfItExists(VertexElementType type, VertexElementSemantic semantic);

private:
	std::vector<VertexElement> mVertexElements;
};
#endif