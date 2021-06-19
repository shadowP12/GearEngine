#include "GearEditor.h"
#include "TextureImporter.h"

GearEditor::GearEditor() {
    mTextureImporter = new TextureImporter();
}

GearEditor::~GearEditor() {
    delete mTextureImporter;
}

TextureImporter * GearEditor::getTextureImporter() {
    return mTextureImporter;
}

GearEditor gEditor;