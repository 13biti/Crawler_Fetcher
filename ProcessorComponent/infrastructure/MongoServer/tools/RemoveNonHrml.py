import base64
from pymongo import MongoClient
import string
import re

client = MongoClient("mongodb://0.0.0.0:8099")
collection = client.links.DownloadedContent

suspicious_ids = []


def is_media_url(url):
    media_extensions = [
        ".pdf",
        ".mp3",
        ".mp4",
        ".wav",
        ".avi",
        ".mov",
        ".mkv",
        ".flv",
        ".wmv",
        ".zip",
        ".rar",
        ".7z",
        ".png",
        ".jpg",
        ".jpeg",
        ".gif",
        ".webp",
        ".svg",
        ".doc",
        ".docx",
        ".xls",
        ".xlsx",
        ".ppt",
        ".pptx",
    ]
    return any(url.lower().endswith(ext) for ext in media_extensions)


def looks_like_html(text):
    text_lower = text.lower()
    if "<html" in text_lower or "<!doctype html" in text_lower or "<body" in text_lower:
        return True
    if "<?php" in text_lower or "</div>" in text_lower:
        return True
    return False


def mostly_printable(decoded):
    printable = set(string.printable)
    if not decoded:
        return False
    ratio = sum(c in printable for c in decoded) / len(decoded)
    return ratio > 0.85


# First pass: Identify suspicious documents
for doc in collection.find():
    url = doc.get("url", "")
    if is_media_url(url):
        suspicious_ids.append(doc["_id"])
        continue

    try:
        decoded = base64.b64decode(doc["html_content_base64"]).decode(
            "utf-8", errors="ignore"
        )
        if not looks_like_html(decoded) or not mostly_printable(decoded):
            suspicious_ids.append(doc["_id"])
    except Exception as e:
        print(f"Error decoding {doc['_id']}: {e}")
        suspicious_ids.append(doc["_id"])

print(f"Found {len(suspicious_ids)} media-like or binary documents.")

# Update all documents in bulk
# First set looks_like_media=True for suspicious documents
if suspicious_ids:
    collection.update_many(
        {"_id": {"$in": suspicious_ids}}, {"$set": {"looks_like_media": True}}
    )

# Then set looks_like_media=False for all other documents
collection.update_many(
    {"_id": {"$nin": suspicious_ids}}, {"$set": {"looks_like_media": False}}
)

print("✅ Completed updating all documents with looks_like_media status")
