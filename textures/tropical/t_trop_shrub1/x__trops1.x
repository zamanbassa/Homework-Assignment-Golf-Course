xof 0303txt 0032

// DirectX file
// Creator: Ultimate Unwrap3D v2.24
// Time: Sun Dec 18 13:02:55 2005

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
    20.460970; 90.945099; -20.648386;,
    -16.941868; 60.282402; 20.755358;,
    19.456823; 47.222427; -9.416577;,
    -15.908415; 98.429001; -5.500162;,
    -7.354110; 84.701569; 8.102489;,
    6.700777; 21.363342; 22.326683;,
    -14.249244; 44.350121; -15.599983;,
    -3.684196; 66.609146; 39.711029;,
    22.290562; 86.010284; 12.870587;,
    -16.109077; 40.838718; -10.333595;,
    13.592017; 44.559135; 28.522121;,
    7.896638; 78.123215; -23.719236;,
    21.439148; 57.995495; -10.140065;,
    0.349025; 10.690029; 18.259630;,
    -2.039144; 24.973425; -22.179958;,
    33.569210; 38.518845; 14.730748;,
    -0.991615; 73.367226; 1.680882;,
    -11.355459; 6.441612; 14.771681;,
    10.693144; 30.617573; -21.767118;,
    -28.447977; 54.709274; 24.180254;,
    10.112720; 49.658646; 19.444376;,
    -12.352275; 0.610607; -24.156481;,
    10.112720; 0.610607; 19.444376;,
    1.476813; 49.658646; -29.442612;,
    -25.590349; 49.653286; -2.602121;,
    23.457693; 0.605244; -2.602121;,
    -25.590349; 0.605244; -2.602121;,
    21.822760; 49.653286; 12.112292;;
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
    -0.870388; 0.348155; 0.348155;,
    -0.870388; 0.348155; 0.348155;,
    -0.948683; 0.000000; 0.316228;,
    0.447214; 0.894427; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.707107; 0.000000; -0.707107;,
    0.707107; 0.000000; -0.707107;,
    0.666667; -0.333333; -0.666667;,
    0.000000; 0.000000; 0.000000;,
    -0.408248; 0.816497; 0.408248;,
    -0.408248; 0.816497; 0.408248;,
    -0.894427; 0.447214; 0.000000;,
    0.577350; 0.577350; 0.577350;,
    -0.704361; -0.088045; -0.704361;,
    -0.704361; -0.088045; -0.704361;,
    0.000000; 0.000000; -1.000000;,
    -0.992278; -0.124035; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;;
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
      "trops1.tga";
     }
    }

   }
  }
}
