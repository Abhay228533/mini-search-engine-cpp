import os

def process_raw_files(raw_dir="data/raw", output_dir="data/sample", chunk_size=500):
    os.makedirs(output_dir, exist_ok=True)
    
    for filename in os.listdir(raw_dir):
        if filename.endswith(".txt"):
            raw_path = os.path.join(raw_dir, filename)
            print(f"Processing raw file: {filename}...")
            
            with open(raw_path, "r", encoding="utf-8", errors="ignore") as f:
                text = f.read()
                
            # Split large text into smaller chunks to simulate documents
            words = text.split()
            for i in range(0, len(words), chunk_size):
                chunk = words[i:i + chunk_size]
                doc_id = (i // chunk_size) + 1
                out_path = os.path.join(output_dir, f"{os.path.splitext(filename)[0]}_doc{doc_id}.txt")
                
                with open(out_path, "w", encoding="utf-8") as out_file:
                    out_file.write(f"Chunk {doc_id} from {filename}\n")
                    out_file.write(" ".join(chunk))
                    
    print("Raw data processing complete!")

if __name__ == "__main__":
    process_raw_files()