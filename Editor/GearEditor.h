#pragma once
class TextureImporter;
class GltfImporter;
class GearEditor {
public:
    GearEditor();

    ~GearEditor();

    TextureImporter* getTextureImporter();

private:
    TextureImporter* mTextureImporter = nullptr;
};

extern GearEditor gEditor;