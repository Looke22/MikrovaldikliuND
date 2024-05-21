import serial
import pandas as pd
import time

SERIAL_PORT = 'COM3' # Replace with your serial port
SERIAL_BAUD = 9600 # Replace with your serial baud rate


def read_data_from_arduino():
    with serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=10) as ser:  
        ser.write(b'read\n')
        time.sleep(2)  # Wait for the Arduino to send the data
        lines = []
        while ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith("Temp"):
                parts = line.split(': ')
                if len(parts) == 2:
                    index, temp = parts[0].split(' ')[1], parts[1]
                    lines.append((int(index), float(temp)))
        return lines

def save_to_excel(data):
    df = pd.DataFrame(data, columns=["Index", "Temperature"])
    df.to_excel('temperature_data.xlsx', index=False)

def main():
    data = read_data_from_arduino()
    if data:
        save_to_excel(data)
        print("Data saved to temperature_data.xlsx")
    else:
        print("No data received from Arduino")

if __name__ == "__main__":
    main()
