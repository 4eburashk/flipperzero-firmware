#include "subghz_protocol_nice_flo.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

struct SubGhzProtocolNiceFlo {
    SubGhzProtocolCommon common;
};

SubGhzProtocolNiceFlo* subghz_protocol_nice_flo_alloc() {
    SubGhzProtocolNiceFlo* instance = furi_alloc(sizeof(SubGhzProtocolNiceFlo));

    instance->common.name = "Nice FLO";
    instance->common.code_min_count_bit_for_found = 12;
    instance->common.te_short = 700;
    instance->common.te_long = 1400;
    instance->common.te_delta = 200;
    instance->common.type_protocol = TYPE_PROTOCOL_STATIC;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_nice_flo_to_str;
    instance->common.to_save_string =
        (SubGhzProtocolCommonGetStrSave)subghz_protocol_nice_flo_to_save_str;
    instance->common.to_load_protocol=
        (SubGhzProtocolCommonLoad)subghz_protocol_nice_flo_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolEncoderCommonGetUpLoad)subghz_protocol_nice_flo_send_key;
    return instance;
}

void subghz_protocol_nice_flo_free(SubGhzProtocolNiceFlo* instance) {
    furi_assert(instance);
    free(instance);
}

bool subghz_protocol_nice_flo_send_key(SubGhzProtocolNiceFlo* instance, SubGhzProtocolEncoderCommon* encoder){
    furi_assert(instance);
    furi_assert(encoder);
    size_t index = 0;
    encoder->size_upload =(instance->common.code_last_count_bit * 2) + 2;
    if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;
    //Send header
    encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short * 36);
    //Send start bit
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
    //Send key data
    for (uint8_t i = instance->common.code_last_count_bit; i > 0; i--) {
        if(bit_read(instance->common.code_last_found, i - 1)){
            //send bit 1
            encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_long);
            encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
        }else{
            //send bit 0
            encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);
            encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_long);
        }
    }
    return true;
}

void subghz_protocol_nice_flo_reset(SubGhzProtocolNiceFlo* instance) {
    instance->common.parser_step = 0;
}

void subghz_protocol_nice_flo_parse(SubGhzProtocolNiceFlo* instance, bool level, uint32_t duration) {
    switch (instance->common.parser_step) {
    case 0:
        if ((!level)
                && (DURATION_DIFF(duration, instance->common.te_short * 36)< instance->common.te_delta * 36)) {
            //Found header Nice Flo
            instance->common.parser_step = 1;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
        if (!level) {
            break;
        } else if (DURATION_DIFF(duration, instance->common.te_short)< instance->common.te_delta) {
            //Found start bit Nice Flo
            instance->common.parser_step = 2;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if (!level) { //save interval
            if (duration >= (instance->common.te_short * 4)) {
                instance->common.parser_step = 1;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {
                    
                    instance->common.serial = 0x0;
                    instance->common.btn = 0x0;

                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;
                    if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);

                }
                break;
            }
            instance->common.te_last = duration;
            instance->common.parser_step = 3;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 3:
        if (level) {
            if ((DURATION_DIFF(instance->common.te_last,instance->common.te_short) < instance->common.te_delta)
                    && (DURATION_DIFF(duration, instance->common.te_long)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long)< instance->common.te_delta)
                    && (DURATION_DIFF(duration, instance->common.te_short)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = 2;
            } else
                instance->common.parser_step = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    }
}

void subghz_protocol_nice_flo_to_str(SubGhzProtocolNiceFlo* instance, string_t output) {
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);

    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %d Bit\r\n"
        " KEY:0x%08lX\r\n"
        " YEK:0x%08lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_lo,
        code_found_reverse_lo
        );
}


void subghz_protocol_nice_flo_to_save_str(SubGhzProtocolNiceFlo* instance, string_t output) {
    string_printf(
        output,
        "Protocol: %s\n"
        "Bit: %d\n"
        "Key: %08lX\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found & 0x00000000ffffffff));
}

bool subghz_protocol_nice_flo_to_load_protocol(FileWorker* file_worker, SubGhzProtocolNiceFlo* instance){
    bool loaded = false;
    string_t temp_str;
    string_init(temp_str);
    int res = 0;
    int data = 0;

    do {
        // Read and parse bit data from 2nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        res = sscanf(string_get_cstr(temp_str), "Bit: %d\n", &data);
        if(res != 1) {
            break;
        }
        instance->common.code_last_count_bit = (uint8_t)data;

        // Read and parse key data from 3nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        uint32_t temp_key = 0;
        res = sscanf(string_get_cstr(temp_str), "Key: %08lX\n", &temp_key);
        if(res != 1) {
            break;
        }
        instance->common.code_last_found = (uint64_t)temp_key;

        loaded = true;
    } while(0);

    string_clear(temp_str);

    return loaded;
}
