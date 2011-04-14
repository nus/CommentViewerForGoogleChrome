// プラグイン情報の記述。NP_GetValue関数をコールされることでプラグイン情報が取得される。
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
            // プラグイン名を返却
            #ifdef WIN32
            *static_cast<const char **>(value) = "x-winNiconamaClient";
            #endif
            break;
        case NPPVpluginDescriptionString:
            // プラグインの説明文字列を返却
            *static_cast<const char **>(value) = "Get NicoNama Comments";
            break;
        default:
            return NPERR_INVALID_PARAM;
            break;
        }
        return NPERR_NO_ERROR;
    }
}
