xof 0303txt 0032

// DirectX file
// Creator: Ultimate Unwrap3D v2.24
// Time: Sun Dec 18 13:03:34 2005

// Start of Templates

template VertexDuplicationIndices {
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}

template FVFData {
 <b6e70a0e-8ef9-4e83-94ad-ecc8b0c04897>
 DWORD dwFVF;
 DWORD nDWords;
 array DWORD data[nDWords];
}

template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

template FrameTransformMatrix {
 <F6F23F41-7686-11cf-8F52-0040333594A3>
 Matrix4x4 frameMatrix;
}

template Frame {
 <3D82AB46-62DA-11cf-AB39-0020AF71E433>
 [...]
}

template FloatKeys {
 <10DD46A9-775B-11cf-8F52-0040333594A3>
 DWORD nValues;
 array FLOAT values[nValues];
}

template TimedFloatKeys {
 <F406B180-7B3B-11cf-8F52-0040333594A3>
 DWORD time;
 FloatKeys tfkeys;
}

template AnimationKey {
 <10DD46A8-775B-11cf-8F52-0040333594A3>
 DWORD keyType;
 DWORD nKeys;
 array TimedFloatKeys keys[nKeys];
}

template AnimationOptions {
 <E2BF56C0-840F-11cf-8F52-0040333594A3>
 DWORD openclosed;
 DWORD positionquality;
}

template Animation {
 <3D82AB4F-62DA-11cf-AB39-0020AF71E433>
 [...]
}

template AnimationSet {
 <3D82AB50-62DA-11cf-AB39-0020AF71E433>
 [Animation]
}

// Start of Frames

Frame Body {
   FrameTransformMatrix {
    1.000000, 0.000000, 0.000000, 0.000000,
    0.000000, 1.000000, 0.000000, 0.000000,
    0.000000, 0.000000, 1.000000, 0.000000,
    0.000000, 0.000000, 0.000000, 1.000000;;
   }

   Mesh staticMesh {
    28;
    20.460970; 67.116486; -20.648386;,
    -16.941868; 44.359013; 20.755358;,
    19.456823; 34.666065; -9.416576;,
    -15.908416; 72.670944; -5.500162;,
    -7.354109; 62.482613; 8.102489;,
    6.700777; 15.473774; 22.326683;,
    -14.249244; 32.534275; -15.599983;,
    -3.684196; 49.054646; 39.711029;,
    22.290562; 63.453926; 12.870588;,
    -16.109077; 29.928154; -10.333595;,
    13.592017; 32.689404; 28.522121;,
    7.896638; 57.600243; -23.719236;,
    21.439148; 42.661701; -10.140064;,
    0.349025; 7.552174; 18.259630;,
    -2.039144; 18.153133; -22.179958;,
    33.569210; 28.206375; 14.730749;,
    -0.991615; 54.070404; 1.680882;,
    -11.355459; 4.399052; 14.771681;,
    10.693144; 22.342148; -21.767118;,
    -28.447977; 40.222710; 24.180254;,
    10.112720; 36.474194; 19.444376;,
    -12.352275; 0.071354; -24.156481;,
    10.112720; 0.071354; 19.444376;,
    1.476813; 36.474194; -29.442612;,
    -25.590349; 36.470215; -2.602121;,
    23.457693; 0.067371; -2.602121;,
    -25.590349; 0.067371; -2.602121;,
    21.822760; 36.470215; 12.112293;;
    28;
    3;0, 1, 2;,
    3;2, 1, 0;,
    3;3, 1, 0;,
    3;0, 1, 3;,
    3;4, 5, 6;,
    3;6, 5, 4;,
    3;7, 5, 4;,
    3;4, 5, 7;,
    3;8, 9, 10;,
    3;10, 9, 8;,
    3;11, 9, 8;,
    3;8, 9, 11;,
    3;12, 13, 14;,
    3;14, 13, 12;,
    3;15, 13, 12;,
    3;12, 13, 15;,
    3;16, 17, 18;,
    3;18, 17, 16;,
    3;19, 17, 16;,
    3;16, 17, 19;,
    3;20, 21, 22;,
    3;22, 21, 20;,
    3;23, 21, 20;,
    3;20, 21, 23;,
    3;24, 25, 26;,
    3;26, 25, 24;,
    3;27, 25, 24;,
    3;24, 25, 27;;

   MeshNormals {
    28;
    -0.727607; 0.485071; 0.485071;,
    -0.727607; 0.485071; 0.485071;,
    -0.894427; 0.000000; 0.447214;,
    0.447214; 0.894427; 0.000000;,
    -0.705001; 0.705001; 0.077110;,
    -0.705001; 0.705001; 0.077110;,
    0.000000; 1.000000; 0.000000;,
    -0.890178; 0.445089; 0.097363;,
    0.000000; 0.000000; -1.000000;,
    0.000000; 0.000000; -1.000000;,
    0.000000; 0.000000; -1.000000;,
    0.000000; 0.000000; 0.000000;,
    -0.447214; 0.000000; -0.894427;,
    -0.447214; 0.000000; -0.894427;,
    0.000000; 0.000000; -1.000000;,
    -1.000000; 0.000000; 0.000000;,
    0.992278; -0.124035; 0.000000;,
    0.992278; -0.124035; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.992278; -0.124035; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.242536; 0.000000; -0.970143;,
    0.242536; 0.000000; -0.970143;,
    0.000000; 0.000000; 0.000000;,
    0.242536; 0.000000; -0.970143;;
    28;
    3;0, 1, 2;,
    3;2, 1, 0;,
    3;3, 1, 0;,
    3;0, 1, 3;,
    3;4, 5, 6;,
    3;6, 5, 4;,
    3;7, 5, 4;,
    3;4, 5, 7;,
    3;8, 9, 10;,
    3;10, 9, 8;,
    3;11, 9, 8;,
    3;8, 9, 11;,
    3;12, 13, 14;,
    3;14, 13, 12;,
    3;15, 13, 12;,
    3;12, 13, 15;,
    3;16, 17, 18;,
    3;18, 17, 16;,
    3;19, 17, 16;,
    3;16, 17, 19;,
    3;20, 21, 22;,
    3;22, 21, 20;,
    3;23, 21, 20;,
    3;20, 21, 23;,
    3;24, 25, 26;,
    3;26, 25, 24;,
    3;27, 25, 24;,
    3;24, 25, 27;;
   }

   MeshTextureCoords {
    28;
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;,
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;,
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;,
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;,
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;,
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;,
    0.996832; 0.003168;,
    0.003168; 0.996832;,
    0.996832; 0.996832;,
    0.003168; 0.003168;;
   }

   MeshMaterialList {
    1;
    28;
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0;

    Material {
     0.752941; 0.752941; 0.752941; 1.000000;;
     40.000000;
     0.000000; 0.000000; 0.000000;;
     0.000000; 0.000000; 0.000000;;

     TextureFilename {
      "trops2.tga";
     }
    }

   }
  }
}
