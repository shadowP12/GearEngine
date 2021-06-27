#pragma once
class TextureImporter;
class GltfImporter;
class GearEditor {
public:
    GearEditor();

    ~GearEditor();

    TextureImporter* getTextureImporter();

    GltfImporter* getGltfImporter();

private:
    TextureImporter* mTextureImporter = nullptr;
    GltfImporter* mGltfImporter = nullptr;
};

extern GearEditor gEditor;