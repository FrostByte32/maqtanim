#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>
#include "colors.h"
#include "logging.h"
#include "minute-layer.h"

typedef struct {
    struct tm tick_time;
    EventHandle tick_timer_event_handle;
} Data;

static void prv_update_proc(MinuteLayer *this, GContext *ctx) {
    logf();
    GRect frame = layer_get_frame(this);
    Data *data = layer_get_data(this);
    FFont *font = ffont_create_from_resource(RESOURCE_ID_ROBOTO_CONDENSED_FFONT);

    FContext fctx;
    fctx_init_context(&fctx, ctx);

    fctx_set_offset(&fctx, g2fpoint(frame.origin));
    fctx_set_fill_color(&fctx, colors_get_accent_color());
    fctx_set_text_em_height(&fctx, font, 25);

    char s[3];
    strftime(s, sizeof(s), "%M", &data->tick_time);

    fctx_begin_fill(&fctx);
    fctx_draw_string(&fctx, s, font, GTextAlignmentRight, FTextAnchorTop);
    fctx_end_fill(&fctx);

    if (!clock_is_24h_style()) {
        fctx_set_offset(&fctx, FPointI(frame.origin.x, frame.origin.y + 25));
        strftime(s, sizeof(s), "%p", &data->tick_time);
        fctx_begin_fill(&fctx);
        fctx_draw_string(&fctx, s, font, GTextAlignmentRight, FTextAnchorTop);
        fctx_end_fill(&fctx);
    }

    fctx_deinit_context(&fctx);
    ffont_destroy(font);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed, void *this) {
    logf();
    Data *data = layer_get_data(this);
    memcpy(&data->tick_time, tick_time, sizeof(struct tm));
#ifdef DEMO
    data->tick_time.tm_min = 34;
#endif
    layer_mark_dirty(this);
}

MinuteLayer *minute_layer_create(GPoint origin) {
    logf();
    MinuteLayer *this = layer_create_with_data(GRect(origin.x, origin.y, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT), sizeof(Data));
    layer_set_update_proc(this, prv_update_proc);
    Data *data = layer_get_data(this);

    time_t now = time(NULL);
    prv_tick_handler(localtime(&now), MINUTE_UNIT, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(MINUTE_UNIT, prv_tick_handler, this);

    return this;
}

void minute_layer_destroy(MinuteLayer *this) {
    logf();
    Data *data = layer_get_data(this);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    layer_destroy(this);
}
