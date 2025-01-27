#include "../subghz_i.h"

#define SCENE_SAVE_SUCCESS_CUSTOM_EVENT (0UL)

void subghz_scene_save_success_popup_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SCENE_SAVE_SUCCESS_CUSTOM_EVENT);
}

const void subghz_scene_save_success_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    Popup* popup = subghz->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, subghz);
    popup_set_callback(popup, subghz_scene_save_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewPopup);
}

const bool subghz_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_SAVE_SUCCESS_CUSTOM_EVENT) {
            return scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
        }
    }
    return false;
}

const void subghz_scene_save_success_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    Popup* popup = subghz->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
    popup_set_callback(popup, NULL);
    popup_set_context(popup, NULL);
    popup_set_timeout(popup, 0);
    popup_disable_timeout(popup);
}
