#!/bin/bash

show_help() {
	echo "Usage: source env_setup.sh [OPTIONS]"
	echo ""
	echo "Options (all are optional):"
	echo "  --raw-queue-name NAME       -> sets RAW_LINKS_QUEUE_NAME (default: rawlinks)"
	echo "  --download-queue-name NAME  -> sets DOWNLOAD_LINKS_QUEUE_NAME (default: downloadlinks)"
	echo "  --write-user USERNAME       -> sets WRITE_USERNAME (default: u2)"
	echo "  --read-user USERNAME        -> sets READ_USERNAME (default: u1)"
	echo "  --queue-pass PASSWORD       -> sets QUEUE_PASSWORD (default: 123)"
	echo "  --api-login URL             -> sets API_LOGIN (default: login)"
	echo "  --api-send PATH             -> sets API_SEND (default: /write)"
	echo "  --api-receive PATH          -> sets API_RECEIVE (default: /read)"
	echo "  --mongo-uri URI             -> sets MONGO_URLS_URI (default: empty)"
	echo "  --mongo-db NAME             -> sets MONGO_URLS_DB (default: empty)"
	echo "  --mongo-client NAME         -> sets MONGO_URLS_CLIENT (default: empty)"
	echo "  --help                      -> show this help message"
}

# Default values
RAW_LINKS_QUEUE_NAME="rawlinks"
DOWNLOAD_LINKS_QUEUE_NAME="downloadlinks"
WRITE_USERNAME="u2"
READ_USERNAME="u1"
QUEUE_PASSWORD="123"
API_LOGIN="login"
API_SEND="/write"
API_RECEIVE="/read"
MONGO_URLS_URI=""
MONGO_URLS_DB=""
MONGO_URLS_CLIENT=""

# Parse arguments
while [[ $# -gt 0 ]]; do
	case "$1" in
	--raw-queue-name)
		RAW_LINKS_QUEUE_NAME="$2"
		shift 2
		;;
	--download-queue-name)
		DOWNLOAD_LINKS_QUEUE_NAME="$2"
		shift 2
		;;
	--write-user)
		WRITE_USERNAME="$2"
		shift 2
		;;
	--read-user)
		READ_USERNAME="$2"
		shift 2
		;;
	--queue-pass)
		QUEUE_PASSWORD="$2"
		shift 2
		;;
	--api-login)
		API_LOGIN="$2"
		shift 2
		;;
	--api-send)
		API_SEND="$2"
		shift 2
		;;
	--api-receive)
		API_RECEIVE="$2"
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

# Export environment variables
export RAW_LINKS_QUEUE_NAME
export DOWNLOAD_LINKS_QUEUE_NAME
export WRITE_USERNAME
export READ_USERNAME
export QUEUE_PASSWORD
export API_LOGIN
export API_SEND
export API_RECEIVE
export MONGO_URLS_URI
export MONGO_URLS_DB
export MONGO_URLS_CLIENT

# Show current values
echo "Environment variables set:"
echo "RAW_LINKS_QUEUE_NAME: $RAW_LINKS_QUEUE_NAME"
echo "DOWNLOAD_LINKS_QUEUE_NAME: $DOWNLOAD_LINKS_QUEUE_NAME"
echo "WRITE_USERNAME: $WRITE_USERNAME"
echo "READ_USERNAME: $READ_USERNAME"
echo "QUEUE_PASSWORD: $QUEUE_PASSWORD"
echo "API_LOGIN: $API_LOGIN"
echo "API_SEND: $API_SEND"
echo "API_RECEIVE: $API_RECEIVE"
echo "MONGO_URLS_URI: $MONGO_URLS_URI"
echo "MONGO_URLS_DB: $MONGO_URLS_DB"
echo "MONGO_URLS_CLIENT: $MONGO_URLS_CLIENT"
