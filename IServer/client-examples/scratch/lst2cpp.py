import sys
import re

def process_line(line):
    # Remove any leading or trailing whitespace
    line = line.strip()

    # Skip empty lines
    if not line:
        return None

    # Split the line into parts based on whitespace
    # parts = re.split(r'\s+', line)

    # Check if there's an address at the beginning
    #print(f"line [{line}]")
    match = re.match(r'^([0-9A-Fa-f]{8})\s+(\S+:)$', line)
    if match:
        address = match.group(1)
        label = match.group(2).strip()
        return f'            // {address} {label}'

    match = re.match(r'^([0-9A-Fa-f]{8})\s*(([0-9A-Fa-f]{2}\s*){0,4})\s*(.*?)$', line)
    if match:
        address = match.group(1)
        code_stripped = match.group(2).strip()
        machine_code = re.split(r'\s+', code_stripped)
        instruction = match.group(4)
        #print(f" ADDR {address} CODE {code_stripped} INSTR {instruction}")
        if len(code_stripped) > 0:
            comment = f'// {address} {instruction}'

            # Ensure machine code parts are properly formatted
            formatted_machine_code = [f'0x{byte}, ' for byte in machine_code]
            formatted_machine_code = "".join(formatted_machine_code)
            return f'            {formatted_machine_code:<24} {comment}'
        else:
            return None

    return None

if __name__ == "__main__":
    for line in sys.stdin:
        line = line.strip()
        if line:
            try:
                result = process_line(line)
                if result:
                    print(result)
            except Exception as e:
                print(f"Error processing line: {line} - {e}")