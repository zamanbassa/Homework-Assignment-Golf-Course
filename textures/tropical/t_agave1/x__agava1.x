xof 0303txt 0032

// DirectX file
// Creator: Ultimate Unwrap3D v2.24
// Time: Sun Dec 18 12:50:24 2005

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
    13;
    -14.235313; 44.582336; -21.592922;,
    -15.914327; 0.012657; -14.306391;,
    -0.404552; 22.534626; 1.035116;,
    16.624729; 0.012657; 16.841116;,
    10.306356; 53.950447; 24.851130;,
    -17.935287; 58.582336; 20.575134;,
    -11.740630; 0.012657; 21.324001;,
    9.403878; 0.012657; -18.448690;,
    20.005154; 66.886765; -15.078516;,
    22.119675; 0.012657; 0.113717;,
    26.398609; 44.582336; -5.012369;,
    -22.920630; 0.012657; 0.686946;,
    -27.418335; 33.796722; 10.930912;;
    24;
    3;0, 1, 2;,
    3;2, 1, 0;,
    3;3, 2, 1;,
    3;1, 2, 3;,
    3;3, 4, 2;,
    3;2, 4, 3;,
    3;0, 2, 4;,
    3;4, 2, 0;,
    3;5, 6, 2;,
    3;2, 6, 5;,
    3;7, 2, 6;,
    3;6, 2, 7;,
    3;7, 8, 2;,
    3;2, 8, 7;,
    3;5, 2, 8;,
    3;8, 2, 5;,
    3;2, 9, 10;,
    3;10, 9, 2;,
    3;9, 2, 11;,
    3;11, 2, 9;,
    3;2, 12, 11;,
    3;11, 12, 2;,
    3;12, 2, 10;,
    3;10, 2, 12;;

   MeshNormals {
    13;
    0.000000; -1.000000; 0.000000;,
    0.000000; 0.242536; -0.970143;,
    0.529813; -0.662266; -0.529813;,
    0.000000; 0.351123; -0.936329;,
    0.000000; -0.298275; -0.954480;,
    0.847998; -0.529999; 0.000000;,
    0.842032; -0.539427; 0.000000;,
    0.000000; -1.000000; 0.000000;,
    0.000000; -1.000000; 0.000000;,
    0.000000; -1.000000; 0.000000;,
    0.000000; 1.000000; 0.000000;,
    0.000000; 0.000000; 0.000000;,
    0.000000; 1.000000; 0.000000;;
    24;
    3;0, 1, 2;,
    3;2, 1, 0;,
    3;3, 2, 1;,
    3;1, 2, 3;,
    3;3, 4, 2;,
    3;2, 4, 3;,
    3;0, 2, 4;,
    3;4, 2, 0;,
    3;5, 6, 2;,
    3;2, 6, 5;,
    3;7, 2, 6;,
    3;6, 2, 7;,
    3;7, 8, 2;,
    3;2, 8, 7;,
    3;5, 2, 8;,
    3;8, 2, 5;,
    3;2, 9, 10;,
    3;10, 9, 2;,
    3;9, 2, 11;,
    3;11, 2, 9;,
    3;2, 12, 11;,
    3;11, 12, 2;,
    3;12, 2, 10;,
    3;10, 2, 12;;
   }

   MeshTextureCoords {
    13;
    0.006411; 0.006692;,
    0.004851; 0.991234;,
    0.499420; 0.496664;,
    0.990139; 0.991234;,
    0.990900; 0.006342;,
    0.006411; 0.006692;,
    0.004851; 0.991234;,
    0.990139; 0.991234;,
    0.990900; 0.006342;,
    0.004851; 0.991234;,
    0.006411; 0.006692;,
    0.990139; 0.991234;,
    0.990900; 0.006342;;
   }

   MeshMaterialList {
    1;
    24;
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
      "agava1.tga";
     }
    }

   }
  }
}
