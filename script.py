def split_and_arrange(source_file, destination_file):
    try:
        with open(source_file, 'r') as src:
            lines = src.readlines()

        # Remove leading/trailing whitespaces and newline characters
        lines = [line.strip() for line in lines]

        # Join the lines with commas
        contents_as_csv = ",".join(lines)

        # Write the CSV formatted content to the destination file
        with open(destination_file, 'w') as dest:
            dest.write(contents_as_csv)

        print("Successfully split and arranged the contents of the file.")
    except FileNotFoundError:
        print("Error: Source file not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    source_file_path = "inputnumbers.csv"
    destination_file_path = "oneline.csv"

    split_and_arrange(source_file_path, destination_file_path)

