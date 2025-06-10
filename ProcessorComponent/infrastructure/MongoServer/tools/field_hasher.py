import hashlib
from pymongo import MongoClient, UpdateOne

client = MongoClient("mongodb://0.0.0.0:8099")  # Add auth if needed
collection = client.links.DownloadedContent

BATCH_SIZE = 1000  # Safe for large collections


def compute_hash(data_str):
    # SHA-256 of the base64 string (not decoded)
    return hashlib.sha256(data_str.encode("utf-8")).hexdigest()


cursor = collection.find({}, {"_id": 1, "html_content_base64": 1})

batch = []
for doc in cursor:
    html_base64 = doc.get("html_content_base64", "")
    html_hash = compute_hash(html_base64)
    batch.append(
        {"filter": {"_id": doc["_id"]}, "update": {"$set": {"html_hash": html_hash}}}
    )

    # Bulk write in batches
    if len(batch) >= BATCH_SIZE:
        collection.bulk_write([UpdateOne(**op) for op in batch])
        print(f"Processed batch of {len(batch)}")
        batch.clear()

# Final batch
if batch:
    collection.bulk_write([UpdateOne(**op) for op in batch])
    print(f"Processed final batch of {len(batch)}")

print("✅ Done hashing all html_content_base64 fields.")
