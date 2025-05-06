#!/bin/bash

show_help() {
	echo "Usage: source script.sh [OPTIONS]"
	echo ""
	echo "Options (all are optional):"
	echo "  --queue-name NAME           -> sets NEW_LINKS_QUEUE_NAME (default: newlinks)"
	echo "  --queue-user USERNAME       -> sets NEW_LINK_QUEUE_READ_USERNAME (default: u)"
	echo "  --queue-pass PASSWORD       -> sets NEW_LINKS_QUEUE_PASSWORD (default: 123)"
	echo "  --api-login URL             -> sets API_LOGIN (default: login)"
	echo "  --mongo-uri URI             -> sets MONGO_URLS_URI (default: empty)"
	echo "  --mongo-db NAME             -> sets MONGO_URLS_DB (default: empty)"
	echo "  --mongo-client NAME         -> sets MONGO_URLS_CLIENT (default: empty)"
	echo "  --help                      -> show this help message"
}

NEW_LINKS_QUEUE_NAME="newlinks"
NEW_LINK_QUEUE_READ_USERNAME="u"
NEW_LINKS_QUEUE_PASSWORD="123"
API_LOGIN="login"
MONGO_URLS_URI=""
MONGO_URLS_DB=""
MONGO_URLS_CLIENT=""

while [[ $# -gt 0 ]]; do
	case "$1" in
	--queue-name)
		NEW_LINKS_QUEUE_NAME="$2"
		shift 2
		;;
	--queue-user)
		NEW_LINK_QUEUE_READ_USERNAME="$2"
		shift 2
		;;
	--queue-pass)
		NEW_LINKS_QUEUE_PASSWORD="$2"
		shift 2
		;;
	--api-login)
		API_LOGIN="$2"
		shift 2
		;;
	--mongo-uri)
		MONGO_URLS_URI="$2"
		shift 2
		;;
	--mongo-db)
		MONGO_URLS_DB="$2"
		shift 2
		;;
	--mongo-client)
		MONGO_URLS_CLIENT="$2"
		shift 2
		;;
	--help)
		show_help
		return 0
		;;
	*)
		echo "Unknown option: $1"
		show_help
		return 1
		;;
	esac
done

export NEW_LINKS_QUEUE_NAME
export NEW_LINK_QUEUE_READ_USERNAME
export NEW_LINKS_QUEUE_PASSWORD
export API_LOGIN
export MONGO_URLS_URI
export MONGO_URLS_DB
export MONGO_URLS_CLIENT

echo "NEW_LINKS_QUEUE_NAME: $NEW_LINKS_QUEUE_NAME"
echo "NEW_LINK_QUEUE_READ_USERNAME: $NEW_LINK_QUEUE_READ_USERNAME"
echo "NEW_LINKS_QUEUE_PASSWORD: $NEW_LINKS_QUEUE_PASSWORD"
echo "API_LOGIN: $API_LOGIN"
echo "MONGO_URLS_URI: $MONGO_URLS_URI"
echo "MONGO_URLS_DB: $MONGO_URLS_DB"
echo "MONGO_URLS_CLIENT: $MONGO_URLS_CLIENT"
