#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <curl/curl.h>
#include <libwebsockets.h>
#include "global.h"
#include "bug.h"
#include "utils.h"
#include "i3.h"
#define _DISCORD_C
#include "discord.h"

int discord_on_message(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static struct lws_context* websocket_context;
static struct lws* wsi;

static struct lws_protocols discord_protocols[] = {
    {
        "discord_bot",
        discord_on_message,
        0,
        1024
    },
    {
        NULL,
        NULL,
        0,
        0
    }
};

char* escape_json_string(const char* str) {
  const char* ptr = str;
  char* result = (char*)malloc(4 * strlen(str) + 3);
  char* resptr = result;

  //*resptr++ = '"';
  while (*ptr) {
    if (*ptr == '\\') {
      *resptr++ = '\\';
      *resptr++ = '\\';
    } else if (*ptr == '\"') {
      *resptr++ = '\\';
      *resptr++ = '\"';
    } else if ((*ptr & 0x80) == 0) {
      // single-byte character
      *resptr++ = *ptr;
    } else if ((*ptr & 0xe0) == 0xc0) {
      // two-byte character
      uint32_t codepoint = ((*ptr & 0x1f) << 6) | (*(ptr + 1) & 0x3f);
      resptr += sprintf(resptr, "\\u%04x", codepoint);
      ptr++;
    } else if ((*ptr & 0xf0) == 0xe0) {
      // three-byte character
      uint32_t codepoint = ((*ptr & 0x0f) << 12) | ((*(ptr + 1) & 0x3f) << 6) | (*(ptr + 2) & 0x3f);
      resptr += sprintf(resptr, "\\u%04x", codepoint);
      ptr += 2;
    } else {
      // four-byte character
      uint32_t codepoint = ((*ptr & 0x07) << 18) | ((*(ptr + 1) & 0x3f) << 12) | ((*(ptr + 2) & 0x3f) << 6) | (*(ptr + 3) & 0x3f);
      resptr += sprintf(resptr, "\\u%04x\\u%04x", (codepoint >> 16) & 0xffff, codepoint & 0xffff);
      ptr += 3;
    }
    ptr++;
  }
  //*resptr++ = '"';
  *resptr++ = '\0';

  return result;
}

char *escape_discord_markdown(const char *str) {
  const char* ptr = str;
  char* result = (char*)malloc(3 * strlen(str) + 1);
  char* resptr = result;

  while (*ptr) {
    switch (*ptr) {
      case '_':
      case '*':
      case '~':
      case '|':
      case '`':
        *resptr++ = '\\';
        *resptr++ = *ptr;
        break;
      case '\\':
        *resptr++ = '\\';
        *resptr++ = '\\';
        break;
      default:
        *resptr++ = *ptr;
        break;
    }
    ptr++;
  }
  *resptr++ = '\0';

  return result;
}

void discord_startup(void) {
    struct lws_context_creation_info info = { 0 };
    struct lws_client_connect_info connect_info = { 0 };

    // Create libwebsockets context
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = discord_protocols;
    websocket_context = lws_create_context(&info);
    if (!websocket_context) {
        log_error("Could not create websocket context!");
        return;
    }

    // Create WebSocket client
    connect_info.context = websocket_context;
    connect_info.address = GATEWAY_URL;
    connect_info.port = 443;
    connect_info.ssl_connection = 1;
    connect_info.path = "/";
    connect_info.host = "gateway.discord.gg";
    wsi = lws_client_connect_via_info(&connect_info);
    if (!wsi) {
        log_error("Could not connect to discord!");
        return;
    }

    // Authenticate the bot with the server
    char auth_msg[512];
    sprintf(auth_msg, "{\"op\": 2,\"d\": {\"token\": \"%s\",\"intents\": 513,\"properties\": {\"$os\": \"linux\",\"$browser\": \"my_library\",\"$device\": \"my_library\"}}}", BOT_TOKEN);
    lws_write(wsi, (unsigned char *)auth_msg, strlen(auth_msg), LWS_WRITE_TEXT);

    // Listen for incoming messages
    // This needs to be a polling event, so we can call it every N main loops
    //while (lws_service(context, 0) >= 0) {}

    // Disconnect from server
    //lws_context_destroy(context);
}

void discord_poll(void) {
    // Listen for incoming messages
    if(lws_service(websocket_context, 0) >= 0) {
        // Something happened!
    }
}

void discord_shutdown(void) {
    // Disconnect from server
    lws_context_destroy(websocket_context);
}

int discord_on_message(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    // Parse incoming message and deal with it.
    return 0; // OK
}

size_t discord_write_data_callback( void *buffer, size_t size, size_t nmemb, void *userp) {
    // For now, we just ignore any returned data.
    // Ideally, we want to report non-success results via bug logging.
    return size * nmemb;
}

void discord_send_message(const char *bot_token, const char *channel_id, const char *message) {
    CURL *curl;
    CURLcode res;
    static char discord_error[CURL_ERROR_SIZE] = "\0\0\0\0\0\0\0";
    char endpoint[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char payload[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char botauth[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *json_escaped = NULL;

    // Construct the bot auth string
    snprintf(botauth, MAX_INPUT_LENGTH, "Authorization: Bot %s", bot_token);
    // Construct the Discord API endpoint
    snprintf(endpoint, MAX_INPUT_LENGTH,
            "https://discord.com/api/channels/%s/messages", channel_id);

    // Cheese the JSON
    json_escaped = escape_json_string(message);
    if(!json_escaped || !*json_escaped) {
        // Either we failed, or it was an empty string.
        log_error("Could not escape I3 message for JSON");
        return;
    }
    memset(payload, 0, MAX_STRING_LENGTH);
    snprintf(payload, MAX_STRING_LENGTH, "{\"content\":\"%s\"}", json_escaped);
    free(json_escaped);

    // Initialize cURL
    curl = curl_easy_init();

    if (curl) {
        // Set the HTTP request headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, botauth);

        // Set the cURL options
        curl_easy_setopt(curl, CURLOPT_URL, endpoint);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discord_write_data_callback);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, discord_error);

        // Send the HTTP request
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            log_error("Could not send I3 message to Discord: %s", discord_error);
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

const char *discord_find_my_channel(const char *i3_channel) {
    // These are channels on my own test server
    if(!strcmp(i3_channel, "intergossip"))
        return "1078154922832441514";
    else if(!strcmp(i3_channel, "dchat"))
         return "1078154970026758194";
    else if(!strcmp(i3_channel, "ds"))
         return "1078154990276857906";
    else if(!strcmp(i3_channel, "free_speech"))
         return "1078155017107820555";
    else if(!strcmp(i3_channel, "intercre"))
         return "1078155035755683850";
    else if(!strcmp(i3_channel, "dwchat"))
         return "1078155067036815420";
    else if(!strcmp(i3_channel, "bsg"))
         return "1078155081687502999";
    else
        return NULL;
}

const char *discord_find_channel(const char *i3_channel) {
    // Ugly, but efficient and simple.
    // These are channels on Sali's live server
    if(!strcmp(i3_channel, "intergossip"))
        return "1074652451766026250";
    else if(!strcmp(i3_channel, "dchat"))
        return "1079142483969200148";
    else if(!strcmp(i3_channel, "ds"))
        return "1079142576831090718";
    else if(!strcmp(i3_channel, "free_speech"))
        return "1074652631886217226";
    else if(!strcmp(i3_channel, "intercre"))
        return "1079142677737640107";
    else if(!strcmp(i3_channel, "dwchat"))
        return "1079142483969200148";
    else if(!strcmp(i3_channel, "bsg"))
        return "1074652532586070066";
    else
        return NULL;
}

const char *discord_default_channel(int is_live) {
    // Returns the default "catch all" channel for
    // either my server, or Sali's live server
    return is_live ? "1079164231049556108" : "1078155196355588127";
}

void allchan_discord(int is_emote, const char *channel, const char *speaker,
        const char *username, const char *mud, const char *message) {
    char result[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char *markdown_channel = NULL;
    char *markdown_speaker = NULL;
    char *markdown_mud = NULL;
    char *markdown_message = NULL;
    char *stripped = NULL;
    const char *match = NULL;

    markdown_channel = escape_discord_markdown(channel);
    markdown_speaker = escape_discord_markdown(speaker);
    markdown_mud = escape_discord_markdown(mud);
    markdown_message = escape_discord_markdown(message);

    if( !markdown_channel || !*markdown_channel ||
        !markdown_speaker || !*markdown_speaker ||
        !markdown_mud     || !*markdown_mud ||
        !markdown_message || !*markdown_message) {
        // Either we failed, or it was an empty string.
        log_error("Could not escape I3 message for Markdown");
        if(markdown_channel) free(markdown_channel);
        if(markdown_speaker) free(markdown_speaker);
        if(markdown_mud) free(markdown_mud);
        if(markdown_message) free(markdown_message);
        return;
    }

    memset(result, 0, MAX_STRING_LENGTH);
    match = discord_find_channel(channel);
    if(!match || !*match) {
        match = discord_default_channel(1); // 1 is Sali, 0 is mine
        if(!match || !*match) {
            log_error("Could not find Discord channel to match I3 channel");
            free(markdown_channel);
            free(markdown_speaker);
            free(markdown_mud);
            free(markdown_message);
            return;
        }
        snprintf(result, MAX_STRING_LENGTH, "__**%s**__ `%s@%s%s` %s",
                    channel, speaker, mud,
                    (is_emote? "" : ":"), markdown_message);
        //match = CHANNEL_ID;
    } else {
        snprintf(result, MAX_STRING_LENGTH, "`%s@%s%s` %s",
                    speaker, mud,
                    (is_emote? "" : ":"), markdown_message);
    }
    free(markdown_channel);
    free(markdown_speaker);
    free(markdown_mud);
    free(markdown_message);
    stripped = i3_strip_colors(result);

    log_info("Discord result: %s", stripped);
    discord_send_message(BOT_TOKEN, match, stripped);
}

