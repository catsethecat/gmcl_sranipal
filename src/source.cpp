#include <gmod/Interface.h>
#include <sranipal/SRanipal.h>
#include <sranipal/SRanipal_Eye.h>
#include <sranipal/SRanipal_Lip.h>
#include <Windows.h>

#pragma comment (lib, "SRanipal.lib")

char facial_blend_names[][32] = {
    "Jaw_Right",
    "Jaw_Left",
    "Jaw_Forward",
    "Jaw_Open",
    "Mouth_Ape_Shape",
    "Mouth_Upper_Right",
    "Mouth_Upper_Left",
    "Mouth_Lower_Right",
    "Mouth_Lower_Left",
    "Mouth_Upper_Overturn",
    "Mouth_Lower_Overturn",
    "Mouth_Pout",
    "Mouth_Smile_Right",
    "Mouth_Smile_Left",
    "Mouth_Sad_Right",
    "Mouth_Sad_Left",
    "Cheek_Puff_Right",
    "Cheek_Puff_Left",
    "Cheek_Suck",
    "Mouth_Upper_UpRight",
    "Mouth_Upper_UpLeft",
    "Mouth_Lower_DownRight",
    "Mouth_Lower_DownLeft",
    "Mouth_Upper_Inside",
    "Mouth_Lower_Inside",
    "Mouth_Lower_Overlay",
    "Tongue_LongStep1",
    "Tongue_Left",
    "Tongue_Right",
    "Tongue_Up",
    "Tongue_Down",
    "Tongue_Roll",
    "Tongue_LongStep2",
    "Tongue_UpRight_Morph",
    "Tongue_UpLeft_Morph",
    "Tongue_DownRight_Morph",
    "Tongue_DownLeft_Morph",
};

ViveSR::anipal::Eye::EyeData_v2 eye_data_v2;
ViveSR::anipal::Lip::LipData_v2 lip_data_v2;
HANDLE SRanipalThreadHandle = NULL;
bool SRanipalLipInitialized = false;
bool SRanipalEyeInitialized = false;
int LipTableRef = 0;
int LeftEyeTableRef = 0;
int RightEyeTableRef = 0;

void SRanipalDataThread(){
    char lip_image[800 * 400];
    lip_data_v2.image = lip_image;
    int error = ViveSR::Error::WORK;
    while(SRanipalEyeInitialized || SRanipalLipInitialized){
        if(SRanipalEyeInitialized)
            error = ViveSR::anipal::Eye::GetEyeData_v2(&eye_data_v2);
        if(SRanipalLipInitialized)
            error = ViveSR::anipal::Lip::GetLipData_v2(&lip_data_v2);
    }
    return;
}

LUA_FUNCTION(Init) {
    if(SRanipalThreadHandle != NULL)
        return 0;
    int error = ViveSR::anipal::Initial(ViveSR::anipal::Eye::ANIPAL_TYPE_EYE_V2, NULL);
    SRanipalEyeInitialized = (error == ViveSR::Error::WORK);
    LUA->PushNumber(error);
    error = ViveSR::anipal::Initial(ViveSR::anipal::Lip::ANIPAL_TYPE_LIP_V2, NULL);
    SRanipalLipInitialized = (error == ViveSR::Error::WORK);
    LUA->PushNumber(error);
    if ((SRanipalThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SRanipalDataThread, NULL, 0, NULL)) == NULL)
        LUA->ThrowError("Failed to create SRanipal thread");
    LUA->CreateTable();
    LipTableRef = LUA->ReferenceCreate();
    LUA->CreateTable();
    LeftEyeTableRef = LUA->ReferenceCreate();
    LUA->CreateTable();
    RightEyeTableRef = LUA->ReferenceCreate();
    return 2;
}

LUA_FUNCTION(Shutdown) {
    SRanipalLipInitialized = false;
    SRanipalEyeInitialized = false;
    ViveSR::anipal::Release(ViveSR::anipal::Lip::ANIPAL_TYPE_LIP_V2);
    ViveSR::anipal::Release(ViveSR::anipal::Eye::ANIPAL_TYPE_EYE_V2);
    if(SRanipalThreadHandle != NULL){
        CloseHandle(SRanipalThreadHandle);
        SRanipalThreadHandle = NULL;
        LUA->ReferenceFree(LipTableRef);
        LUA->ReferenceFree(LeftEyeTableRef);
        LUA->ReferenceFree(RightEyeTableRef);
    }
    return 0;
}

LUA_FUNCTION(GetLipData) {
    LUA->ReferencePush(LipTableRef);
    LUA->PushNumber(lip_data_v2.timestamp);
    LUA->SetField(-2, "timestamp");
    for (int i = 0; i < 37; i++) {
        LUA->PushNumber(lip_data_v2.prediction_data.blend_shape_weight[i]);
        LUA->SetField(-2, facial_blend_names[i]);
    }
    return 1;
}

LUA_FUNCTION(GetEyeData) {
    LUA->ReferencePush(LeftEyeTableRef);
    LUA->PushNumber(eye_data_v2.timestamp);
    LUA->SetField(-2, "timestamp");
    LUA->PushVector(*(Vector*)&eye_data_v2.verbose_data.left.gaze_direction_normalized);
    LUA->SetField(-2, "gaze_direction");
    LUA->PushVector(*(Vector*)&eye_data_v2.verbose_data.left.gaze_origin_mm);
    LUA->SetField(-2, "gaze_origin_mm");
    LUA->PushNumber(eye_data_v2.verbose_data.left.pupil_diameter_mm);
    LUA->SetField(-2, "pupil_diameter_mm");
    LUA->PushNumber(eye_data_v2.verbose_data.left.eye_openness);
    LUA->SetField(-2, "eye_openness");
    LUA->PushNumber(eye_data_v2.expression_data.left.eye_wide);
    LUA->SetField(-2, "eye_wide");
    LUA->PushNumber(eye_data_v2.expression_data.left.eye_squeeze);
    LUA->SetField(-2, "eye_squeeze");
    LUA->PushNumber(eye_data_v2.expression_data.left.eye_frown);
    LUA->SetField(-2, "eye_frown");
    LUA->ReferencePush(RightEyeTableRef);
    LUA->PushVector(*(Vector*)&eye_data_v2.verbose_data.right.gaze_direction_normalized);
    LUA->SetField(-2, "gaze_direction");
    LUA->PushVector(*(Vector*)&eye_data_v2.verbose_data.right.gaze_origin_mm);
    LUA->SetField(-2, "gaze_origin_mm");
    LUA->PushNumber(eye_data_v2.verbose_data.right.pupil_diameter_mm);
    LUA->SetField(-2, "pupil_diameter_mm");
    LUA->PushNumber(eye_data_v2.verbose_data.right.eye_openness);
    LUA->SetField(-2, "eye_openness");
    LUA->PushNumber(eye_data_v2.expression_data.right.eye_wide);
    LUA->SetField(-2, "eye_wide");
    LUA->PushNumber(eye_data_v2.expression_data.right.eye_squeeze);
    LUA->SetField(-2, "eye_squeeze");
    LUA->PushNumber(eye_data_v2.expression_data.right.eye_frown);
    LUA->SetField(-2, "eye_frown");
    return 2;
}

LUA_FUNCTION(GetVersion) {
    LUA->PushNumber(1);
    return 1;
}

GMOD_MODULE_OPEN(){
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA->GetField(-1, "sranipal");
    if (!LUA->IsType(-1, GarrysMod::Lua::Type::TABLE)) {
        LUA->Pop(1);
        LUA->CreateTable();
    }
    LUA->PushCFunction(GetVersion);
    LUA->SetField(-2, "GetVersion");
    LUA->PushCFunction(Init);
    LUA->SetField(-2, "Init");
    LUA->PushCFunction(Shutdown);
    LUA->SetField(-2, "Shutdown");
    LUA->PushCFunction(GetLipData);
    LUA->SetField(-2, "GetLipData");
    LUA->PushCFunction(GetEyeData);
    LUA->SetField(-2, "GetEyeData");
    LUA->SetField(-2, "sranipal");
    return 0;
}

GMOD_MODULE_CLOSE(){
    return 0;
}