#include "GearEditor.h"
#include "TextureImporter.h"
#include "GltfImporter/GltfImporter.h"

GearEditor::GearEditor() {
    mTextureImporter = new TextureImporter();
    mGltfImporter = new GltfImporter();
}

GearEditor::~GearEditor() {
    delete mTextureImporter;
    mTextureImporter = nullptr;
    delete mGltfImporter;
    mGltfImporter = nullptr;
}

TextureImporter* GearEditor::getTextureImporter() {
    return mTextureImporter;
}

GltfImporter* GearEditor::getGltfImporter() {
    return mGltfImporter;
}

GearEditor gEditor;