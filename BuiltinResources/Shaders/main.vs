void main() { 
    MaterialVertexInputs material;
    initMaterialVertex(material);

    materialVertex(material);

    gl_Position = getWorldPosition(material);
}
