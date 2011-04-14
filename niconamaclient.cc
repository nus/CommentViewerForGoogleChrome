// �v���O�C�����̋L�q�BNP_GetValue�֐����R�[������邱�ƂŃv���O�C����񂪎擾�����B
#include <npapi.h>

extern "C" {
    const char *NP_GetMIMEDescription(void) {
    	#ifdef WIN32
        return "application/x-winNiconamaClient::Get NicoNama Comments";
        #endif
    }

    NPError NP_GetValue(NPP instance, NPPVariable variable, void *value) {
        switch(variable) {
        case NPPVpluginNameString:
            // �v���O�C������ԋp
            #ifdef WIN32
            *static_cast<const char **>(value) = "x-winNiconamaClient";
            #endif
            break;
        case NPPVpluginDescriptionString:
            // �v���O�C���̐����������ԋp
            *static_cast<const char **>(value) = "Get NicoNama Comments";
            break;
        default:
            return NPERR_INVALID_PARAM;
            break;
        }
        return NPERR_NO_ERROR;
    }
}
