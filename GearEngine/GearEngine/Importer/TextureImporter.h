#ifndef TEXTURE_IMPORTER_H
#define TEXTURE_IMPORTER_H
#include "../Texture/Texture.h"
#include "../Resource/TextureManager.h"
#include "../ThirdParty/stb_image.h"

class TextureImporter
{
public:
	TextureImporter();
	~TextureImporter();
	void load(std::string name)
	{
		//todo:add more texture detail
		//todo:file system
		std::string path = "D:/GearEngine/GearEngine/Resource/Textures/" + name;//hard code
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		std::shared_ptr<Texture> tex = std::shared_ptr<Texture>(new Texture());
		tex->init(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		tex->createSampler();
		tex->setName(path);
		TextureManager::instance().addRes(tex);
		//todo:add default texture
	}
private:

};

TextureImporter::TextureImporter()
{
}

TextureImporter::~TextureImporter()
{
}
#endif