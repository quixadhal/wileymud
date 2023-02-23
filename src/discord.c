#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <curl/curl.h>
#include "global.h"
#include "bug.h"
#include "utils.h"
#include "i3.h"
#define _DISCORD_C
#include "discord.h"

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

void allchan_discord(int is_emote, const char *channel, const char *speaker,
        const char *username, const char *mud, const char *message) {
    char result[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char *markdown_channel = NULL;
    char *markdown_speaker = NULL;
    char *markdown_mud = NULL;
    char *markdown_message = NULL;
    char *stripped = NULL;

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
    snprintf(result, MAX_STRING_LENGTH, "__**%s**__ `%s@%s%s` %s",
                markdown_channel, markdown_speaker, markdown_mud,
                (is_emote? "" : ":"), markdown_message);
    free(markdown_channel);
    free(markdown_speaker);
    free(markdown_mud);
    free(markdown_message);
    stripped = i3_strip_colors(result);

    log_info("Discord result: %s", stripped);
    discord_send_message(BOT_TOKEN, CHANNEL_ID, stripped);
}

