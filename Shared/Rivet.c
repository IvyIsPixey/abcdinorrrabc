#include <Shared/Rivet.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef EMSCRIPTEN
#include <curl/curl.h>
#else
#include <emscripten.h>
#endif

#include <Shared/cJSON.h>

#ifdef RIVET_BUILD
#define BASE_API_URL "https://rrolf.io/api/"
#else
#define BASE_API_URL "https://ms.dinorr.fun/api/"
#endif

#define RR_RIVET_CURL_PROLOGUE                                                 \
    struct curl_slist *list = 0;                                               \
    int err = 0;                                                               \
    CURL *curl = curl_easy_init();                                             \
    assert(curl);                                                              \
    char header[500] = "Authorization: Bearer ";                               \
    list = curl_slist_append(list, strcat(header, lobby_token));               \
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

#define RR_RIVET_CURL_EPILOGUE                                                 \
    err = curl_easy_perform(curl);                                             \
    assert(!err);                                                              \
    curl_easy_cleanup(curl);                                                   \
    curl_slist_free_all(list);

void rr_rivet_lobbies_ready(char const *lobby_token)
{
#ifdef RR_SERVER
    RR_RIVET_CURL_PROLOGUE
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://matchmaker.api.rivet.gg/v1/lobbies/ready");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{}");
    RR_RIVET_CURL_EPILOGUE
#endif
}

void rr_rivet_lobbies_set_closed(char const *lobby_token, int closed)
{
#ifdef RR_SERVER
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "is_closed", closed);
    char *post_data = cJSON_Print(root);

    RR_RIVET_CURL_PROLOGUE
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://matchmaker.api.rivet.gg/v1/lobbies/closed");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    RR_RIVET_CURL_EPILOGUE

    free(post_data);
    cJSON_Delete(root);
#endif
}

int rr_rivet_players_connected(char const *lobby_token,
                               char const *player_token)
{
#ifdef RR_SERVER
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "player_token", player_token);
    char *post_data = cJSON_Print(root);

    RR_RIVET_CURL_PROLOGUE
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://matchmaker.api.rivet.gg/v1/players/connected");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    err = curl_easy_perform(curl);
    assert(!err);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    free(post_data);
    cJSON_Delete(root);
    curl_easy_cleanup(curl);
    curl_slist_free_all(list);
    return http_code == 200;
#endif
    return 1;
}

void rr_rivet_players_disconnected(char const *lobby_token,
                                   char const *player_token)
{
#ifdef RR_SERVER
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "player_token", player_token);
    char *post_data = cJSON_Print(root);

    RR_RIVET_CURL_PROLOGUE
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://matchmaker.api.rivet.gg/v1/players/disconnected");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    RR_RIVET_CURL_EPILOGUE

    free(post_data);
    cJSON_Delete(root);
#endif
}

// public token:
// dev_prod.eyJ0eXAiOiJKV1QiLCJhbGciOiJFZERTQSJ9.CMStoJHeMhDE1dvT6DEaEgoQ-Yg2l5RWSyiQiBfNQuWRzyIhQh8KEgoQqqIdtJJtQI-wQbKCRZBOIhoJMTI3LjAuMC4x.JlT3Uo-gf1XMVmVkb-Fo0q7bj8TyhPuGjQ6D6daX2cwlyT_iQq3eg6MECgH2L38fFPMKYM8S1uHp4j0gmEJOCw

void rr_rivet_lobbies_find(void *captures, char const *region)
{
    puts("<rr_rivet::lobby_find>");
#ifdef EMSCRIPTEN
    EM_ASM(
        {
            try
            {
                fetch("https://matchmaker.api.rivet.gg/v1/lobbies/find", {
                    "headers" : {
                        "Authorization" :   
                            // clang-format off
                            "Bearer dev_prod.eyJ0eXAiOiJKV1QiLCJhbGciOiJFZERTQSJ9.CMStoJHeMhDE1dvT6DEaEgoQ-Yg2l5RWSyiQiBfNQuWRzyIhQh8KEgoQqqIdtJJtQI-wQbKCRZBOIhoJMTI3LjAuMC4x.JlT3Uo-gf1XMVmVkb-Fo0q7bj8TyhPuGjQ6D6daX2cwlyT_iQq3eg6MECgH2L38fFPMKYM8S1uHp4j0gmEJOCw"
                        // clang-format on
                    },
                    "method" : "POST",
                    "body" : $1 ? '{"game_modes":["default"],"regions":["' +
                                      UTF8ToString($1) + '"]}'
                                : '{"game_modes":["default"]}'
                })
                    .then(function(r) { return r.json(); })
                    .then(function(json) {
                        const host = json["ports"]["default"]["hostname"];
                        const token = "Bearer " + json["player"]["token"];
                        const $host = _malloc(host.length + 1);
                        const $token = _malloc(1 + token.length);
                        HEAPU8.set(new TextEncoder().encode(host), $host);
                        HEAPU8.set(new TextEncoder().encode(token), $token);
                        HEAPU8[$host + host.length] = 0;
                        HEAPU8[$token + token.length] = 0;
                        _rr_rivet_lobby_on_find(
                            $host, $token, json["ports"]["default"]["port"],
                            $0);
                        _free($host);
                        _free($token);
                    })
                    .catch(function(error) { console.log(error); });
            }
            catch (e)
            {
                console.log(e);
            }
        },
        captures, region);
#endif
}

void rr_rivet_lobbies_join(void *captures, char const *lobby_id)
{
    printf("<rr_rivet::lobby_join::%s>\n", lobby_id);
#ifdef EMSCRIPTEN
    EM_ASM(
        {
            fetch("https://matchmaker.api.rivet.gg/v1/lobbies/join", {
                "headers" : {
                    "Authorization" :
                        // clang-format off
                        "Bearer dev_prod.eyJ0eXAiOiJKV1QiLCJhbGciOiJFZERTQSJ9.CMStoJHeMhDE1dvT6DEaEgoQ-Yg2l5RWSyiQiBfNQuWRzyIhQh8KEgoQqqIdtJJtQI-wQbKCRZBOIhoJMTI3LjAuMC4x.JlT3Uo-gf1XMVmVkb-Fo0q7bj8TyhPuGjQ6D6daX2cwlyT_iQq3eg6MECgH2L38fFPMKYM8S1uHp4j0gmEJOCw"
                    // clang-format on
                },
                "method" : "POST",
                "body" : '{"lobby_id":"' + UTF8ToString($1) + '"}'
            })
                .then(function(r) { return r.json(); })
                .then(function(json) {
                    const host = json["ports"]["default"]["hostname"];
                    const token = "Bearer " + json["player"]["token"];
                    const $host = _malloc(host.length + 1);
                    const $token = _malloc(1 + token.length);
                    HEAPU8.set(new TextEncoder().encode(host), $host);
                    HEAPU8.set(new TextEncoder().encode(token), $token);
                    HEAPU8[$host + host.length] = 0;
                    HEAPU8[$token + token.length] = 0;
                    _rr_rivet_lobby_on_find(
                        $host, $token, json["ports"]["default"]["port"], $0);
                    _free($host);
                    _free($token);
                })
                .catch(
                    function(error) { _rr_rivet_lobby_on_find(0, 0, 0, $0); });
        },
        captures, lobby_id);
#endif
}

void rr_rivet_link_account(char *game_user, char *api_password, void *captures)
{
    puts("<rr_rivet::account_link>");
#ifdef EMSCRIPTEN
    // clang-format off
    EM_ASM(
        {
            let token = UTF8ToString($0);
            let api = UTF8ToString($1);
            let api_pw = UTF8ToString($2);
            let w;
            fetch("https://api.rivet.gg/identity/game-links", {
                "method": "POST",
                "body": "{}",
                "headers": {
                    "Authorization": "Bearer " + token
                },
            }).then(r => r.json())
            .then(r => {
                function handle(h)
                {
                    h.then(h => h.json()).then(newer => {
                        if (newer["status"] === "incomplete")
                        {
                            console.log("<not linked yet, checking again>");
                            let h = fetch("https://api.rivet.gg/identity/game-links?identity_link_token=" + r["identity_link_token"] + "&watch_index=" + newer["watch"]["index"]);
                            handle(h);
                        }
                        else if (newer["status"] === "cancelled")
                            console.log("<cancelled linking>");
                        else if (newer["status"] === "complete")
                        {
                            fetch(api + "user_get_password/" + newer["new_identity"]["identity_token"]).then(x => x.text()).then(new_pw => {
                                fetch(api + "account_link/" +
                                    newer["current_identity"]["identity_id"] + "/" +
                                    api_pw + "/" +
                                    newer["new_identity"]["identity"]["identity_id"] + "/" +
                                    new_pw
                                ).then(x => x.text()).then(x => {
                                    if (x !== "success")
                                    {
                                        alert("Linking failed: please try again\n" + "attempted to link " + 
                                        newer["current_identity"]["identity_id"] + " and " + newer["new_identity"]["identity"]["identity_id"] 
                                        + "\nPlease screenshot this and send to devs");
                                    }
                                    else
                                    {   
                                        w.close();
                                        window.localStorage["old_account_uuid"] = newer["current_identity"]["identity_id"];
                                        window.localStorage["DO_NOT_SHARE_old_rivet_account_token"] = window.localStorage["DO_NOT_SHARE_rivet_account_token"];
                                        window.localStorage["DO_NOT_SHARE_rivet_account_token"] = newer["new_identity"]["identity_token"];
                                        location.reload(false);
                                    }
                                });
                            });
                        }
                    })
                }
                w = window.open(r["identity_link_url"], "", "width=600,height=600");
                let h = fetch("https://api.rivet.gg/identity/game-links?identity_link_token=" + r["identity_link_token"], {
                    });
                handle(h);
            })

        }, game_user, BASE_API_URL, api_password);
    // clang-format on
#endif
}


void rr_rivet_identities_create_guest(void *captures)
{
    puts("<rr_rivet::attempt_login>");
#ifdef EMSCRIPTEN
// clang-format off
EM_ASM({
    function on_account(x)
    {
        // doesn't matter if this memory gets leaked, it's only ever ran once
        const $token = _malloc(x["identity_token"].length + 1);
        const $avatar_url = _malloc(x["identity"]["avatar_url"].length + 1);
        const $name = _malloc(x["identity"]["display_name"].length + 1);
        const $uuid = _malloc(x["identity"]["identity_id"].length + 1);
        const $account_number = _malloc(6);
        HEAPU8[$token + x["identity_token"].length] = 0; // null terminate
        HEAPU8[$avatar_url + x["identity"]["avatar_url"].length] = 0;
        HEAPU8[$name + x["identity"]["display_name"].length] = 0;
        HEAPU8[$uuid + x["identity"]["identity_id"].length] = 0;
        HEAPU8[$account_number + 5] = 0;
        HEAPU8.set(new TextEncoder().encode(x["identity_token"]), $token);
        HEAPU8.set(new TextEncoder().encode(x["identity"]["avatar_url"]), $avatar_url);
        HEAPU8.set(new TextEncoder().encode(x["identity"]["display_name"]), $name);
        HEAPU8.set(new TextEncoder().encode(x["identity"]["identity_id"]), $uuid);
        HEAPU8.set(new TextEncoder().encode("#"+x["identity"]["account_number"].toString().padStart(4, "0")), $account_number);
        _rr_rivet_on_log_in($token, $avatar_url, $name, $account_number, $uuid, +x["identity"]["is_game_linked"], $0);
        _free($uuid);
        _free($token);
        _free($avatar_url);
        _free($name);
        _free($account_number);
    }

    function attempt(x)
    {
        if (!window.localStorage[x])
        {
            fetch("https://api.rivet.gg/identity/identities", {
                "method": "POST",
                "body": "{}",
                "headers": {
                    "Authorization":"Bearer dev_prod.eyJ0eXAiOiJKV1QiLCJhbGciOiJFZERTQSJ9.CMStoJHeMhDE1dvT6DEaEgoQ-Yg2l5RWSyiQiBfNQuWRzyIhQh8KEgoQqqIdtJJtQI-wQbKCRZBOIhoJMTI3LjAuMC4x.JlT3Uo-gf1XMVmVkb-Fo0q7bj8TyhPuGjQ6D6daX2cwlyT_iQq3eg6MECgH2L38fFPMKYM8S1uHp4j0gmEJOCw"
                },
            }).then(r => r.json())
            .then(r => {
                window.localStorage[x] = r["identity_token"];
                window.localStorage["rivet_account_uuid"] = r["identity"]["identity_id"];
                on_account(r);
            });
        }
        else
        {
            fetch("https://api.rivet.gg/identity/identities", {
                "method": "POST",
                "body": JSON.stringify({
                    "existing_identity_token": window.localStorage[x]
                }),
                "headers": {
                    "Authorization": "Bearer dev_prod.eyJ0eXAiOiJKV1QiLCJhbGciOiJFZERTQSJ9.CMStoJHeMhDE1dvT6DEaEgoQ-Yg2l5RWSyiQiBfNQuWRzyIhQh8KEgoQqqIdtJJtQI-wQbKCRZBOIhoJMTI3LjAuMC4x.JlT3Uo-gf1XMVmVkb-Fo0q7bj8TyhPuGjQ6D6daX2cwlyT_iQq3eg6MECgH2L38fFPMKYM8S1uHp4j0gmEJOCw"
                },
            }).then(r => r.json())
            .then(r => {
                if (r.code == "ERROR")
                    throw r;
                window.localStorage["DO_NOT_SHARE_rivet_account_token"] = r["identity_token"];
                if (!window.localStorage["rivet_account_uuid"])
                    window.localStorage["rivet_account_uuid"] = r["identity"]["identity_id"];
                on_account(r);
            }).catch(function(e)
            {
               alert("Login failed: please reload");
            });
        }
    } 
        attempt("DO_NOT_SHARE_rivet_account_token");
    }, captures);
#endif
}
