#!/bin/bash

DISCORD_WEBHOOK="https://discord.com/api/webhooks/1223572472096620555/03KZivChR8PJIMgXCC_jo2aBTKxXvdfBIs1-0bO3QnFZPYS_Nx_bjMFd2WU4tpLI-5K7"

send_webhook_message() {
    local description="$1"
    local color="$2"

    local payload="{
        \"embeds\": [{
            \"title\": \"Server Status\",
            \"description\": \"$description\",
            \"color\": $color,
            \"thumbnail\": {
                \"url\": \"https://i.ibb.co/5XDLrfN/Site-favicon.png\"
            }
        }]
    }"

    curl -H "Content-Type: application/json" -X POST -d "$payload" "$DISCORD_WEBHOOK"
}

start_servers() {
    description="Started all game servers."
    
    send_webhook_message "$description" 3066993

    echo "Starting master server..."
    cd /var/www/rrolf/game/MasterServer
    node main.js &
    
    echo "Starting game server..."
    cd /var/www/rrolf/game/Server/build
    ./rrolf-server &
}

start_servers
