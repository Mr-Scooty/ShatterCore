#!/usr/bin/env python3
"""
Hot Rod DBC Fixer for World of Warcraft 4.3.4
Fixes the Hot Rod (Vehicle ID 181) to have all 4 seats properly configured
and adds the missing seat 2082 to VehicleSeat.dbc

This script:
1. Reads Vehicle.dbc and updates entry 181 to have seats 2081, 2082, 2083, 2084
2. Reads VehicleSeat.dbc and adds the missing seat 2082
3. Writes both corrected DBC files
"""

import struct
import sys
import os
from collections import OrderedDict

class DBCReader:
    """Base class for reading DBC files"""
    
    def __init__(self, filename):
        self.filename = filename
        self.header = {}
        self.records = []
        self.string_block = b''
        self.raw_records = []  # Store raw binary data for each record
        
    def read_header(self, file):
        """Read DBC file header"""
        magic = file.read(4)
        if magic != b'WDBC':
            raise ValueError(f"Invalid DBC file format. Expected 'WDBC', got {magic}")
        
        self.header['record_count'] = struct.unpack('I', file.read(4))[0]
        self.header['field_count'] = struct.unpack('I', file.read(4))[0]
        self.header['record_size'] = struct.unpack('I', file.read(4))[0]
        self.header['string_block_size'] = struct.unpack('I', file.read(4))[0]
        
        return self.header
    
    def read_string(self, offset):
        """Read a null-terminated string from the string block"""
        if offset == 0 or offset >= len(self.string_block):
            return ""
        
        end = self.string_block.find(b'\x00', offset)
        if end == -1:
            return ""
        
        try:
            return self.string_block[offset:end].decode('utf-8', errors='ignore')
        except:
            return ""
    
    def parse(self):
        """Parse the DBC file"""
        try:
            with open(self.filename, 'rb') as f:
                self.read_header(f)
                
                # Read all records as raw binary
                for i in range(self.header['record_count']):
                    record_data = f.read(self.header['record_size'])
                    self.raw_records.append(record_data)
                
                # Read string block
                self.string_block = f.read(self.header['string_block_size'])
                
                return self.parse_records()
                
        except FileNotFoundError:
            print(f"Error: File '{self.filename}' not found")
            return None
        except Exception as e:
            print(f"Error parsing file: {e}")
            return None
    
    def write(self, output_filename):
        """Write DBC file with updated records"""
        try:
            with open(output_filename, 'wb') as f:
                # Write header
                f.write(b'WDBC')
                f.write(struct.pack('I', len(self.raw_records)))  # record count
                f.write(struct.pack('I', self.header['field_count']))
                f.write(struct.pack('I', self.header['record_size']))
                f.write(struct.pack('I', self.header['string_block_size']))
                
                # Write records
                for record_data in self.raw_records:
                    f.write(record_data)
                
                # Write string block
                f.write(self.string_block)
                
            return True
        except Exception as e:
            print(f"Error writing file: {e}")
            return False


class VehicleDBCFixer(DBCReader):
    """Fixes Vehicle.dbc for Hot Rod"""
    
    def parse_records(self):
        """Parse Vehicle.dbc records"""
        self.records = []
        
        for i, record_data in enumerate(self.raw_records):
            # Parse just the ID to identify the Hot Rod
            vehicle_id = struct.unpack('I', record_data[0:4])[0]
            
            if vehicle_id == 181:  # Hot Rod
                print(f"Found Hot Rod (ID 181) at record index {i}")
                
                # Parse the current seat configuration
                seat_ids = []
                for j in range(8):
                    seat_offset = 24 + (j * 4)  # Seats start at byte 24
                    seat_id = struct.unpack('I', record_data[seat_offset:seat_offset+4])[0]
                    seat_ids.append(seat_id)
                
                print(f"  Current seats: {seat_ids}")
                
                # Create modified record with correct seats
                modified_record = bytearray(record_data)
                
                # SeatID_1 = 2081 (driver) - already correct
                # SeatID_2 = 2082 (Ace)
                # SeatID_3 = 2083 (Gobber)
                # SeatID_4 = 2084 (Izzy)
                
                struct.pack_into('I', modified_record, 28, 2082)  # Seat 2 (Ace)
                struct.pack_into('I', modified_record, 32, 2083)  # Seat 3 (Gobber)
                struct.pack_into('I', modified_record, 36, 2084)  # Seat 4 (Izzy)
                
                # Verify the changes
                new_seats = []
                for j in range(8):
                    seat_offset = 24 + (j * 4)
                    seat_id = struct.unpack('I', modified_record[seat_offset:seat_offset+4])[0]
                    new_seats.append(seat_id)
                
                print(f"  Updated seats: {new_seats}")
                
                # Replace the record
                self.raw_records[i] = bytes(modified_record)
                
            self.records.append({'ID': vehicle_id})
        
        return self.records


VEHICLE_SEAT_FLAG_CAN_CONTROL = 0x00000800
VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT = 0x02000000


class VehicleSeatDBCFixer(DBCReader):
    """Fixes VehicleSeat.dbc by adding missing seat 2082"""
    
    def parse_records(self):
        """Parse VehicleSeat.dbc records and add missing seat"""
        self.records = []
        seat_2082_exists = False
        seat_2083_data = None
        seat_2083_index = -1
        seat_2081_index = -1
        seat_2084_index = -1
        
        # First pass: check if 2082 exists and find seat 2083 to copy from
        for i, record_data in enumerate(self.raw_records):
            seat_id = struct.unpack('I', record_data[0:4])[0]
            self.records.append({'ID': seat_id})
            
            # Work on a mutable copy when we need to adjust flags
            record_bytes = bytearray(record_data)
            
            if seat_id == 2082:
                seat_2082_exists = True
                print(f"Seat 2082 already exists at record index {i}")
                flags = struct.unpack('I', record_bytes[4:8])[0]
                new_flags = (flags & ~VEHICLE_SEAT_FLAG_CAN_CONTROL) | VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT
                if new_flags != flags:
                    struct.pack_into('I', record_bytes, 4, new_flags)
                    self.raw_records[i] = bytes(record_bytes)
                    print("  Adjusted seat 2082 flags to passenger configuration")
            elif seat_id == 2083:
                # We'll capture updated bytes after adjustments
                seat_2083_data = None
                seat_2083_index = i
                print(f"Found seat 2083 at record index {i} (will use as template)")
                
                # Clear control flag and ensure enter/exit flag
                flags = struct.unpack('I', record_bytes[4:8])[0]
                new_flags = (flags & ~VEHICLE_SEAT_FLAG_CAN_CONTROL) | VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT
                if new_flags != flags:
                    struct.pack_into('I', record_bytes, 4, new_flags)
                    self.raw_records[i] = bytes(record_bytes)
                    print("  Adjusted seat 2083 flags to passenger configuration")
                else:
                    self.raw_records[i] = bytes(record_bytes)
                seat_2083_data = self.raw_records[i]
            elif seat_id == 2081:
                seat_2081_index = i
                flags = struct.unpack('I', record_bytes[4:8])[0]
                new_flags = (flags | VEHICLE_SEAT_FLAG_CAN_CONTROL | VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT)
                if new_flags != flags:
                    struct.pack_into('I', record_bytes, 4, new_flags)
                    self.raw_records[i] = bytes(record_bytes)
                    print("  Ensured driver seat (2081) has control and enter/exit flags")
            elif seat_id == 2084:
                seat_2084_index = i
                flags = struct.unpack('I', record_bytes[4:8])[0]
                new_flags = (flags & ~VEHICLE_SEAT_FLAG_CAN_CONTROL) | VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT
                if new_flags != flags:
                    struct.pack_into('I', record_bytes, 4, new_flags)
                    self.raw_records[i] = bytes(record_bytes)
                    print("  Adjusted seat 2084 flags to passenger configuration")
                else:
                    self.raw_records[i] = bytes(record_bytes)
        
        if not seat_2082_exists and seat_2083_data:
            print("Creating seat 2082 based on seat 2083...")
            
            # Create seat 2082 by copying seat 2083 and changing the ID
            new_seat_2082 = bytearray(seat_2083_data)
            struct.pack_into('I', new_seat_2082, 0, 2082)  # Set ID to 2082
            
            # Ensure passenger flags are applied (no control, can enter/exit)
            passenger_flags = struct.unpack('I', new_seat_2082[4:8])[0]
            passenger_flags = (passenger_flags & ~VEHICLE_SEAT_FLAG_CAN_CONTROL) | VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT
            struct.pack_into('I', new_seat_2082, 4, passenger_flags)
            
            # Insert the new seat after 2081 (or before 2083)
            # Find the right position to insert
            insert_index = -1
            for i, record in enumerate(self.records):
                if record['ID'] == 2081:
                    insert_index = i + 1
                    break
                elif record['ID'] > 2082:
                    insert_index = i
                    break
            
            if insert_index == -1:
                insert_index = len(self.raw_records)
            
            print(f"Inserting new seat 2082 at index {insert_index}")
            self.raw_records.insert(insert_index, bytes(new_seat_2082))
            self.records.insert(insert_index, {'ID': 2082})
            
            # Update header for new record count
            self.header['record_count'] = len(self.raw_records)
            
        elif seat_2082_exists:
            print("Seat 2082 already exists, no changes needed")
        else:
            print("ERROR: Could not find seat 2083 to use as template!")
        
        return self.records


def main():
    """Main function to fix Hot Rod DBC files"""
    
    print("=" * 60)
    print("Hot Rod DBC Fixer for WoW 4.3.4")
    print("Fixes Vehicle ID 181 to have all 4 seats")
    print("=" * 60)
    print()
    
    # Get file paths
    if len(sys.argv) > 1:
        vehicle_file = sys.argv[1]
    else:
        vehicle_file = input("Enter path to Vehicle.dbc: ").strip()
    
    if len(sys.argv) > 2:
        seat_file = sys.argv[2]
    else:
        seat_file = input("Enter path to VehicleSeat.dbc: ").strip()
    
    if len(sys.argv) > 3:
        output_dir = sys.argv[3]
    else:
        output_dir = input("Enter output directory (press Enter for same directory): ").strip()
    
    if not output_dir:
        output_dir = os.path.dirname(vehicle_file) if vehicle_file else "."
    
    # Process Vehicle.dbc
    if vehicle_file and os.path.exists(vehicle_file):
        print(f"\nProcessing Vehicle.dbc: {vehicle_file}")
        print("-" * 40)
        
        vehicle_fixer = VehicleDBCFixer(vehicle_file)
        vehicle_records = vehicle_fixer.parse()
        
        if vehicle_records:
            output_vehicle = os.path.join(output_dir, "Vehicle_fixed.dbc")
            
            # Find and report on the Hot Rod
            hot_rod_found = False
            for record in vehicle_records:
                if record['ID'] == 181:
                    hot_rod_found = True
                    break
            
            if hot_rod_found:
                if vehicle_fixer.write(output_vehicle):
                    print(f"✓ Successfully wrote fixed Vehicle.dbc to: {output_vehicle}")
                else:
                    print(f"✗ Failed to write Vehicle.dbc")
            else:
                print("✗ Hot Rod (ID 181) not found in Vehicle.dbc!")
    else:
        print(f"✗ Vehicle.dbc not found: {vehicle_file}")
    
    # Process VehicleSeat.dbc
    if seat_file and os.path.exists(seat_file):
        print(f"\nProcessing VehicleSeat.dbc: {seat_file}")
        print("-" * 40)
        
        seat_fixer = VehicleSeatDBCFixer(seat_file)
        seat_records = seat_fixer.parse()
        
        if seat_records:
            output_seat = os.path.join(output_dir, "VehicleSeat_fixed.dbc")
            
            if seat_fixer.write(output_seat):
                print(f"✓ Successfully wrote fixed VehicleSeat.dbc to: {output_seat}")
                
                # Report on the seats
                seat_ids = [2081, 2082, 2083, 2084]
                found_seats = []
                for record in seat_records:
                    if record['ID'] in seat_ids:
                        found_seats.append(record['ID'])
                
                print(f"\nSeat Status:")
                for seat_id in seat_ids:
                    if seat_id in found_seats:
                        print(f"  Seat {seat_id}: ✓ Present")
                    else:
                        print(f"  Seat {seat_id}: ✗ Missing")
            else:
                print(f"✗ Failed to write VehicleSeat.dbc")
    else:
        print(f"✗ VehicleSeat.dbc not found: {seat_file}")
    
    print("\n" + "=" * 60)
    print("Fix Complete!")
    print("=" * 60)
    print("\nNext steps:")
    print("1. Back up your original DBC files")
    print("2. Copy the fixed DBC files to your server's dbc directory:")
    print(f"   - {os.path.join(output_dir, 'Vehicle_fixed.dbc')} → Vehicle.dbc")
    print(f"   - {os.path.join(output_dir, 'VehicleSeat_fixed.dbc')} → VehicleSeat.dbc")
    print("3. Copy the same fixed DBC files to your client's Data/dbc directory")
    print("4. Restart the worldserver")
    print("5. Clear your client's cache folder")
    print("\nThe Hot Rod should now have all 4 seats working!")
    print("- Seat 1 (2081): Driver")
    print("- Seat 2 (2082): Ace")
    print("- Seat 3 (2083): Gobber")  
    print("- Seat 4 (2084): Izzy")


if __name__ == "__main__":
    main()
