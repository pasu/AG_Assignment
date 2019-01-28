#include"precomp.h"
#include"GPURT_Material.h"


using namespace gpurt;

gpurt::Mtl::Mtl(aiMaterial & mat) {
    static int count = 0;
    
    mat.Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse);// Kd
    count++;
    mat.Get(AI_MATKEY_COLOR_EMISSIVE, color_diffuse);// Ke

    if (color_diffuse.r > 0.01) {
        _ni = 0;
        _ns = 0;
        _alpha = 0;
        _prefract = 0;
        _pdiffuse = 0;
        _pspecular = 0;
        for (int i = 0; i < 256; i++) {
            _shading_types_[i] = EMMISION;
        }
        return;
    }

    mat.Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse);// Kd

    mat.Get(AI_MATKEY_REFRACTI, _ni);// ior
    
    mat.Get(AI_MATKEY_SHININESS, _ns);// Ns

    mat.Get(AI_MATKEY_OPACITY, _alpha);// blender: alpha, mtl: d
    _prefract = 1 - _alpha;

    mat.Get(AI_MATKEY_COLOR_SPECULAR, color_specular);// Ks
    float strengthDiffuse = color_diffuse.r + color_diffuse.g + color_diffuse.b;
    float strengthSpecular = color_specular.r + color_specular.g + color_specular.b;

    float diffuseRatio;
    if (strengthDiffuse + strengthSpecular < 0.00001) {
        diffuseRatio = 1;
    }
    else {
        diffuseRatio = strengthDiffuse / (strengthDiffuse + strengthSpecular);
    }

    _pdiffuse = diffuseRatio * _alpha;
    _pspecular = (1 - diffuseRatio) * _alpha;

    int nSpecular = 256 * _pspecular;
    int nRefract = 256 * _prefract;
    int nDiffuse = 256 * _pdiffuse;


    // round up to 256
    int total = nDiffuse + nSpecular + nRefract;
    if ( total < 256) {
        if (nDiffuse > 0) {
            nDiffuse += 256 - total;
        }
        else if (nSpecular > 0) {
            nSpecular += 256 - total;
        }
        else if (nRefract > 0) {
            nRefract += 256 - total;
        }
    }
    if (total > 256) {
        while (total > 256) {
            if (nDiffuse > 0) {
                nDiffuse--;
            }
            else if (nSpecular > 0) {
                nSpecular--;
            }
            else if (nRefract > 0) {
                nRefract--;
            }
        }
    }

    // fix possibilities to avoid bias
    _pdiffuse = float(nDiffuse) / 256.0f;
    _prefract = float(nRefract) / 255.0f;
    _pspecular = float(nSpecular) / 255.0f;

    // fill shading type list
    for (int i = 0; i < nDiffuse; i++) {
        _shading_types_[i] = DIFFUSE;
    }
    for (int i = 0; i < nSpecular; i++) {
        _shading_types_[nDiffuse+i] = SPECULAR;
    }
    for (int i = 0; i < nRefract; i++) {
        _shading_types_[nDiffuse + nSpecular + i] = REFRACT;
    }
}
