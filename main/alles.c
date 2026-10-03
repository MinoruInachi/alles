#include "alles.h"


extern uint8_t battery_mask;
extern uint8_t ipv4_quartet;
extern char githash[8];
int16_t client_id;
int64_t clocks[255];
int64_t ping_times[255];
uint8_t alive = 1;

// The current message for the parse task, set by the multicast listener
char *message_start_pointer;
int16_t message_length;

int32_t computed_delta = 0 ; // can be negative no prob, but usually host is larger # than client
uint8_t computed_delta_set = 0; // have we set a delta yet?

extern int64_t last_ping_time;

amy_err_t sync_init() {
    client_id = -1; // for now
    for(uint8_t i=0;i<255;i++) { clocks[i] = 0; ping_times[i] = 0; }
    return AMY_OK;
}



void update_map(int16_t client, uint8_t ipv4, int64_t time) {
    // I'm called when I get a sync response or a regular ping packet
    // I update a map of booted devices.

    //printf("[%d %d] Got a sync response client %d ipv4 %d time %lld\n",  ipv4_quartet, client_id, client , ipv4, time);
    clocks[ipv4] = time;
    int64_t my_sysclock = amy_sysclock();
    ping_times[ipv4] = my_sysclock;

    // Now I basically see what index I would be in the list of booted synths (clocks[i] > 0)
    // And I set my client_id to that index
    uint8_t last_alive = alive;
    uint8_t my_new_client_id = 255;
    alive = 0;
    for(uint8_t i=0;i<255;i++) {
        if(clocks[i] > 0) { 
            if(my_sysclock < (ping_times[i] + (PING_TIME_MS * 2))) { // alive
                //printf("[%d %d] Checking my time %lld against ipv4 %d's of %lld, client_id now %d ping_time[%d] = %lld\n", 
                //    ipv4_quartet, client_id, my_sysclock, i, clocks[i], my_new_client_id, i, ping_times[i]);
                alive++;
            } else {
                //printf("[ipv4 %d client %d] clock %d is dead, ping time was %lld time now is %lld.\n", ipv4_quartet, client_id, i, ping_times[i], my_sysclock);
                clocks[i] = 0;
                ping_times[i] = 0;
            }
            // If this is not me....
            if(i != ipv4_quartet) {
                // predicted time is what we think the alive node should be at by now
                int64_t predicted_time = (my_sysclock - ping_times[i]) + clocks[i];
                if(my_sysclock >= predicted_time) my_new_client_id--;
            } else {
                my_new_client_id--;
            }
        } else {
            // if clocks[] is 0, no need to check
            my_new_client_id--;
        }
    }
    if(client_id != my_new_client_id || last_alive != alive) {
        printf("[%d] my client_id is now %d. %d alive\n", ipv4_quartet, my_new_client_id, alive);
        client_id = my_new_client_id;
    }
}

void handle_sync(int64_t time, int8_t index) {
    // I am called when I get an s message, which comes along with host time and index
    int64_t sysclock = amy_sysclock();
    char message[100];
    // Before I send, i want to update the map locally
    update_map(client_id, ipv4_quartet, sysclock);
    // Send back sync message with my time and received sync index and my client id & battery status (if any)
    sprintf(message, "_U%lldi%dg%dr%dy%dZ", sysclock, index, client_id, ipv4_quartet, battery_mask);
    mcast_send(message, strlen(message));
    // Update computed delta (i could average these out, but I don't think that'll help too much)
    //int64_t old_cd = computed_delta;
    //computed_delta = time - sysclock;
    //computed_delta_set = 1;
    //if(old_cd != computed_delta) printf("Changed computed_delta from %lld to %lld on sync\n", old_cd, computed_delta);
}

// It's ok that r & y are used by AMY, this is only to return values
void ping(int64_t sysclock) {
    char message[100];
    //printf("[%d %d] pinging with %lld\n", ipv4_quartet, client_id, sysclock);
    sprintf(message, "_U%lldi-1g%dr%dy%dZ", sysclock, client_id, ipv4_quartet, battery_mask);
    update_map(client_id, ipv4_quartet, sysclock);
    mcast_send(message, strlen(message));
    last_ping_time = sysclock;
}


void alles_increase_volume() {
    amy_global.volume[AMY_DEFAULT_BUS] += 0.5f;
    if(amy_global.volume[AMY_DEFAULT_BUS] > ALLES_MAX_VOLUME) amy_global.volume[AMY_DEFAULT_BUS] = ALLES_MAX_VOLUME;
}

void alles_decrease_volume() {
    amy_global.volume[AMY_DEFAULT_BUS] -= 0.5f;
    if(amy_global.volume[AMY_DEFAULT_BUS] < 0) amy_global.volume[AMY_DEFAULT_BUS] = 0;
}

// For devices with a single button: step up through a few levels, then wrap around to the quietest
void alles_cycle_volume() {
    const float levels[] = { 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    const int num_levels = sizeof(levels) / sizeof(levels[0]);
    float volume = levels[0];
    for(int i=0;i<num_levels;i++) {
        if(levels[i] > amy_global.volume[AMY_DEFAULT_BUS] + 0.01f) { volume = levels[i]; break; }
    }
    amy_global.volume[AMY_DEFAULT_BUS] = volume;
    printf("volume %.1f\n", volume);
}


void alles_parse_message(char *message, uint16_t length) {
    uint8_t mode = 0;
    int16_t client = -1;
    int64_t sync = 0;
    uint8_t has_sync = 0;
    int8_t sync_index = -1;
    int64_t time = 0;
    uint8_t has_time = 0;  // the host clock can be negative, so track presence separately
    uint8_t ipv4 = 0;
    uint16_t start = 0;
    uint16_t c = 0;
    uint8_t prev_is_cmd = 0;

    uint32_t sysclock = amy_sysclock();
    uint8_t sync_response = (message[0] == '_');

    // Pull out the alles-specific params first. AMY ignores g, t, r and U.
    // A letter straight after a command letter is a sub-command (e.g. "it" in AMY's synth layer), not a mode.
    while(c < length+1) {
        uint8_t b = message[c];
        uint8_t is_alpha = ((b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z'));
        if((is_alpha && !prev_is_cmd) || b == 0) {  // new mode or end
            if(mode=='g') client = atoi(message + start);
            if(mode=='i') sync_index = atoi(message + start);
            if(mode=='t') { time = atoll(message + start); has_time = 1; }
            if(sync_response) if(mode=='r') ipv4=atoi(message + start);
            if(mode=='U') { sync = atoll(message + start); has_sync = 1; }
            mode = b;
            start = c + 1;
            prev_is_cmd = 1;
        } else {
            prev_is_cmd = 0;
        }
        c++;
    }
    if(sync_response) {
        // If this is a sync response, let's update our local map of who is booted
        //printf("got sync response client %d ipv4 %d sync %lld\n", client, ipv4, sync);
        update_map(client, ipv4, sync);
        return;
    }
    if(length == 0) return;
    if(has_sync && sync_index >= 0) {
        // Don't add sync messages to the event queue
        handle_sync(sync, sync_index);
        return;
    }

    // Assume it's for me
    uint8_t for_me = 1;
    // But wait, they specified, so don't assume
    if(client >= 0) {
        for_me = 0;
        if(client <= 255) {
            // If they gave an individual client ID check that it exists
            if(alive>0) { // alive may get to 0 in a bad situation
                if(client >= alive) {
                    client = client % alive;
                }
            }
        }
        // It's actually precisely for me
        if(client == client_id) for_me = 1;
        if(client > 255) {
            // It's a group message, see if i'm in the group
            if(client_id % (client-255) == 0) for_me = 1;
        }
    }
    if(!for_me) return;

    // The host sends its own clock as t. We keep a delta between that and our sysclock,
    // and recompute it if it drifts more than max drift. AMY adds latency_ms in amy_add_event.
    uint32_t event_time = 0;
    if(has_time) {
        int32_t delta = (int32_t)(time - sysclock);
        if(!computed_delta_set || abs(delta - computed_delta) > ALLES_MAX_DRIFT_MS) {
            computed_delta = delta;
            fprintf(stderr,"setting computed delta to %"PRIi32 " (time is %lld sysclock %"PRIu32 ") max_drift_ms %"PRIu32 " latency %"PRIi16 "\n",
                    computed_delta, time, sysclock, (uint32_t)ALLES_MAX_DRIFT_MS, amy_global.latency_ms);
            computed_delta_set = 1;
        }
        event_time = (uint32_t)(time - computed_delta);
    }

    if(message[0] == 'H') {
        // Sequencer (ticks) messages are scheduled by AMY's sequencer, not by time
        handle_ticks_message(message);
        return;
    }
    amy_event e;
    size_t pos = 0;
    do {
        amy_clear_event(&e);
        pos = yield_event_from_message(message, &e, pos);
        if(pos > 0) {
            if(has_time) e.time = event_time;
            amy_add_event(&e);
        }
    } while(pos > 0);
}
