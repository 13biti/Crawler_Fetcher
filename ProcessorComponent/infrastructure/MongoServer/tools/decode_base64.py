import base64
from pymongo import MongoClient
from bson import ObjectId

print("please change MongoClient and set your db & collecction properly ")
client = MongoClient("mongodb://0.0.0.0:8099")
collection = client.links.DownloadedContent


def print_decoded_html(document_id):
    try:
        doc_id = ObjectId(document_id)
        doc = collection.find_one({"_id": doc_id})
        if not doc:
            print(f"No document found with ID: {document_id}")
            return
        if "html_content_base64" not in doc:
            print("Document doesn't contain html_content_base64 field")
            return
        decoded_html = base64.b64decode(doc["html_content_base64"]).decode(
            "utf-8", errors="replace"
        )

        print(f"\n=== Document ID: {doc['_id']} ===")
        print(f"URL: {doc.get('url', 'N/A')}")
        print("\n=== Decoded HTML Content ===\n")
        print(decoded_html[:100000] + ("..." if len(decoded_html) > 100000 else ""))

    except Exception as e:
        print(f"Error processing document: {e}")


if __name__ == "__main__":
    doc_id = input("Enter MongoDB document _id: ").strip()
    print_decoded_html(doc_id)
